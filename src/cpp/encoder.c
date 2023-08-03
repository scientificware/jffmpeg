/*
 * Authors:  Francisco J. Cabello. fjcabello@visual-tools.com
 *           Guilhem Tardy. gtardy@salyens.com
 *
 * Version control
 * ===============
 * $Id: encoder.c,v 1.12 2004/11/01 01:52:12 davidstuart Exp $ 
 * 
 * Revision 1.2  2004/02/19 Guilhem Tardy (gravsten@yahoo.com)
 * Major rewrite, now fully supports H.263/RTP (ffmpeg 0.4.7).
 *
 * Description
 * ============
 * JMF encoding wrapper for FFMPEG 
 *
 * +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <jni.h>
#include <libavcodec/avcodec.h>
#include "net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder.h"
#include "jffmpeg.h"

#ifndef CODEC_FLAG_RFC2190
#define CODEC_FLAG_RFC2190 0
#define CODEC_FLAG_MODE_A_ONLY 0
#endif

//#define DEBUG
//#define FORCE_SPLIT

/*
 * Creates an RTP chunk and adds it to the tail of the queue. Returns a
 * pointer to the chunk, or NULL if it could not be allocated.
 */
static rtpChunk *jffmpeg_alloc_chunk(FFMPEGWrapper *wrapper, void *data, int data_size, void *hdr, int hdr_size)
{
#ifdef DEBUG
    printf("jffmpeg_alloc_chunk(): creating packet with data_size %d and hdr_size %d\n", data_size, hdr_size);
#endif

    // Create a new chunk and add it to the end of the list.
    rtpChunk *newChunk = (rtpChunk *) malloc(sizeof(rtpChunk));
    if (newChunk == NULL) {
        fprintf(stderr, "couldn't allocate another RTP chunk\n");
        return NULL;
    }

    newChunk->header   = hdr;
    newChunk->hdrSize  = hdr_size;
    newChunk->data     = data;
    newChunk->dataSize = data_size;
    newChunk->next = NULL;

    // If this is the first in the list, set the head and tail
    // to point to the same space.
    if (wrapper->tailChunks == NULL)
        wrapper->headChunks = newChunk;
    else
        wrapper->tailChunks->next = newChunk;

    wrapper->tailChunks = newChunk;

    return newChunk;
}

/* RTP callback invoked from the encoder. This function is called every
 * time the encoder has a packet to send. In other words, whenever a chunk
 * is encoded with size below rtp_payload_size */
static void jffmpeg_rtp_callback(void *data, int data_size,
                                 void *hdr, int hdr_size, void *priv_data) {

    FFMPEGWrapper *wrapper;
#ifdef FORCE_SPLIT
    AVCodecContext *context;
    int data_size_limit;
#endif

#ifdef DEBUG
    printf("jffmpeg_rtp_callback(): data_size=%d hdr_size=%d hdr=", data_size, hdr_size);
    int i = 0;
    for(i=0; i<hdr_size; i++)
        printf("%.2x",*(((unsigned char *)hdr)+i));
    printf("\n");
#endif

    assert(priv_data != NULL);
    wrapper = (FFMPEGWrapper *) priv_data;

#ifdef FORCE_SPLIT
    context = wrapper->codec_context;

    // Check the size of the hdr + the size of the data against the
    // MTU size as specified in the context. If it is larger, then
    // we have to split the packet into even smaller chunks known as
    // follow-up packets. For these we use the same header.
    assert(hdr_size < context->rtp_payload_size);
    data_size_limit = context->rtp_payload_size - hdr_size;
    while (data_size > data_size_limit) {
        jffmpeg_alloc_chunk(wrapper, data, data_size_limit, hdr, hdr_size);
        data += data_size_limit;
        data_size -= data_size_limit;
    }
#endif

    // Allocate the remainder which is within the data_size_limit
    jffmpeg_alloc_chunk(wrapper, data, data_size, hdr, hdr_size);
}

static void jffmpeg_set_inputDone(JNIEnv *env, jobject jffmpeg, int done) {
    jclass clazz;
    jfieldID fid;
    assert(env != NULL);

    // Fetch the "inputDone" variable and set it to the value of "done"
    clazz= (*env)->GetObjectClass(env, jffmpeg);
    fid = (*env)->GetFieldID(env, clazz, "inputDone", "Z");
    (*env)->SetBooleanField(env, jffmpeg, fid, (jboolean)done);
}

static void jffmpeg_set_outputSize(JNIEnv *env, jobject jffmpeg, int size) {
    jclass clazz;
    jfieldID fid;

    assert(env != NULL);

    // Fetch the "outputSize" variable and set it to the value of "size"
    clazz = (*env)->GetObjectClass(env, jffmpeg);
    fid = (*env)->GetFieldID(env, clazz, "outputSize", "I");
    (*env)->SetIntField(env, jffmpeg, fid, (jint)size);
}

static void jffmpeg_processList(JNIEnv *env, jobject jffmpeg, FFMPEGWrapper *wrapper, unsigned char *outBuf, int outLength) {
    int dataSize;
    rtpChunk * temp;

#ifdef DEBUG
    printf ("jffmpeg_processList(): processing the head element in the queue\n");
#endif

    // Check the linked list. If there are chunks, pull them off the head and
    // put them into the output buffer without bothering to do any encoding.
    temp = wrapper->headChunks;

    // Copy the header to outBuf
    memcpy(outBuf, temp->header, temp->hdrSize);

    // Truncate the packet in order to avoid "ArrayOutOfBounds" exceptions
    dataSize = temp->dataSize;
    if (temp->hdrSize + dataSize > outLength)
        dataSize = outLength - temp->hdrSize;

    // Copy the data to outBuf
    memcpy(outBuf + temp->hdrSize, temp->data, dataSize);

    // Advance the head pointer
    wrapper->headChunks = temp->next;
    if (wrapper->headChunks == NULL)
        wrapper->tailChunks = NULL; // also set tail

    // The input frame is consumed only if there are no more chunks to process.
    jffmpeg_set_inputDone(env, jffmpeg, wrapper->headChunks == NULL ? 1 : 0);

    // The output frame is filled, set its size.
    jffmpeg_set_outputSize(env, jffmpeg, temp->hdrSize + dataSize);

    // Free memory from old pointer
    free(temp);
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    open_encoder
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_open_1encoder
  (JNIEnv *env, jobject jffmpeg, jstring codec_name,
   jint width, jint height, jint bitRate, jint frameRate, jint keyFrameInterval,
   jfloat quality,  jboolean dynQuality, jint rtpPayloadSize, jboolean compatibility)
{
    FFMPEGWrapper *wrapper;    
    static int FFMPEG_init = 0;
    enum CodecID codec_required;
    jclass clazz;
    jfieldID fidPeer;
    jint peerVal;
    const char * str;
    AVCodecContext * ctx;

    if(!FFMPEG_init) {
        /* must be called before using avcodec lib */
        avcodec_init();

        /* register only the codec you need to have smaller code */
        register_avcodec(&h263_encoder);
        register_avcodec(&h263p_encoder);

        FFMPEG_init = 1;
    }

    // Only perform the following section if the "peer" variable is zero
    clazz = (*env)->GetObjectClass(env, jffmpeg);
    fidPeer = (*env)->GetFieldID(env, clazz, "peer", "I");   
    peerVal = (*env)->GetIntField(env, jffmpeg, fidPeer);
    if (peerVal != 0)
        return (jboolean) 1;
    
    // Otherwise, initialize the wrapper structure and set the peer
    // variable
    wrapper = (FFMPEGWrapper *) malloc(sizeof(FFMPEGWrapper));
    if(wrapper == NULL)
        return (jboolean) 0;

    // Make sure the wrapper is zeroed
    memset(wrapper,0,sizeof(FFMPEGWrapper));

    // Set the "peer" variable in java class
    (*env)->SetIntField(env, jffmpeg, fidPeer, (jint)wrapper);

    // Find matching ffmpeg codec using the codec_name passed in
    str = (*env)->GetStringUTFChars(env, codec_name, 0);

#ifdef DEBUG
    printf("open_encoder(): trying to open codec %s\n", str);
#endif

    if (strcasecmp(str, J_H263) == 0) {
        codec_required = CODEC_ID_H263;
        wrapper->rtp_mode = 0;
    } else if (strcasecmp(str, J_H263_RTP) == 0) {
        codec_required = CODEC_ID_H263;
        wrapper->rtp_mode = 1;
    } else {
        (*env)->ReleaseStringUTFChars(env, codec_name, str);
        return (jboolean) 0;
    }
    (*env)->ReleaseStringUTFChars(env, codec_name, str);

    /* find the video encoder */
    wrapper->codec = avcodec_find_encoder(codec_required);
    if (!wrapper->codec) {
        fprintf(stderr, "codec not found\n");
        return (jboolean) 0;
    }

    // Allocate the codec context and frame.
    wrapper->codec_context = avcodec_alloc_context();
    if (!wrapper->codec_context) {
        fprintf(stderr, "context not allocated\n");
        return (jboolean) 0;
    }

    wrapper->picture = avcodec_alloc_frame();
    if (!wrapper->picture) {
        fprintf(stderr, "frame not allocated\n");
        return (jboolean) 0;
    }

    wrapper->picture->quality = quality;

    ctx = wrapper->codec_context;

    /* Set all parameters to the encoder before opening */
    ctx->flags |= CODEC_FLAG_INPUT_PRESERVED;
    ctx->flags |= CODEC_FLAG_EMU_EDGE;

    ctx->width  = width;
    ctx->height = height;

    ctx->frame_rate = frameRate;
    ctx->frame_rate_base= 1;

    ctx->gop_size = keyFrameInterval;

    ctx->bit_rate = (3 * bitRate) >> 2;
    ctx->bit_rate_tolerance = bitRate << 3;
    ctx->rc_min_rate = 0; // minimum bitrate
    ctx->rc_max_rate = bitRate; // maximum bitrate
    ctx->mb_qmin = ctx->qmin = 4;
    ctx->mb_qmax = ctx->qmax = 24;
    ctx->max_qdiff = 3; // max q difference between frames
    ctx->rc_qsquish = 0; // limit q by clipping
    ctx->rc_eq= "tex^qComp"; // rate control equation
    ctx->qcompress = 0.5; // qscale factor between easy & hard scenes (0.0-1.0)
    ctx->i_quant_factor = (float)-0.6; // qscale factor between p and i frames
    ctx->i_quant_offset = (float)0.0; // qscale offset between p and i frames

    if (dynQuality)
        ctx->flags |= CODEC_FLAG_PASS1;
    else
        ctx->flags |= CODEC_FLAG_QSCALE;

    ctx->mb_decision = FF_MB_DECISION_SIMPLE; // choose only one MB type at a time
    ctx->me_method = ME_EPZS;
    ctx->me_subpel_quality = 8;

    ctx->max_b_frames = 0;

    if (wrapper->rtp_mode) {
        int buf_size = (width * height * 3) >> 1;

        /* Activate the RTP mode with a callback and target payload size. */
        ctx->rtp_mode = 1;
        ctx->opaque = wrapper;
        ctx->rtp_payload_size = rtpPayloadSize;
        ctx->rtp_callback = jffmpeg_rtp_callback;
        ctx->flags |= CODEC_FLAG_RFC2190; // Enable RFC2190 payload header
        if (compatibility)
          ctx->flags |= CODEC_FLAG_MODE_A_ONLY; // Mode A only
        else
          ctx->flags &= ~CODEC_FLAG_MODE_A_ONLY; // Mode A + B

        /* Allocate a temporary buffer for encoding the frame. Make
         * it the same size as a single frame of YUV. */
        wrapper->encode_buf = malloc(buf_size);
        wrapper->encode_buf_size = buf_size;

#ifdef DEBUG
        printf("open_encoder(): rtp_payload_size is %d\n", ctx->rtp_payload_size);
        printf("open_encoder(): temporary buffer size is %d\n", buf_size);
#endif
    }

    /* open it */
    if (avcodec_open(wrapper->codec_context, wrapper->codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        return (jboolean) 0;
    }    

#ifdef DEBUG
    printf("jffmpeg encoder opened\n");
#endif

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    close_encoder
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_close_1encoder
  (JNIEnv *env, jobject jffmpeg, jint peer)
{
    FFMPEGWrapper *wrapper;    
    rtpChunk * temp;
    rtpChunk * prev;
    jclass clazz;
    jfieldID fidPeer;

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (wrapper->codec_context->codec != NULL)
        avcodec_close(wrapper->codec_context);

    // Clean up the rtp queue
    temp = wrapper->headChunks;
    while(temp != NULL) {
        prev = temp;
        temp = temp->next;
        free(prev);
    }

    free(wrapper->copy_buf);
    free(wrapper->encode_buf);
    av_free(wrapper->picture);
    av_free(wrapper->codec_context);
    free(wrapper);

    // Unset the "peer" variable in java class
    clazz = (*env)->GetObjectClass(env, jffmpeg);
    fidPeer = (*env)->GetFieldID(env, clazz, "peer", "I");
    (*env)->SetIntField(env, jffmpeg, fidPeer, (jint)0);

#ifdef DEBUG
    printf("jffmpeg encoder closed\n");
#endif
    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    set_framerate
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_set_1frameRate
  (JNIEnv *env, jobject jffmpeg, jint peer, jint frameRate)
{
    FFMPEGWrapper *wrapper;    

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (!wrapper->codec_context)
        return (jboolean) 0;

    wrapper->codec_context->frame_rate = frameRate;

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    set_quality
 * Signature: (IF)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_set_1quality
  (JNIEnv *env, jobject jffmpeg, jint peer, jfloat quality)
{
    FFMPEGWrapper *wrapper;    

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (!wrapper->picture)
        return (jboolean) 0;

    wrapper->picture->quality = quality;

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    set_rtpPayloadSize
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_set_1rtpPayloadSize
  (JNIEnv *env, jobject jffmpeg, jint peer, jint rtpPayloadSize)
{
    FFMPEGWrapper *wrapper;    

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (!wrapper->codec_context)
        return (jboolean) 0;

    wrapper->codec_context->rtp_payload_size = rtpPayloadSize;

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    set_
 * Signature: (II)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_set_1compatibility
  (JNIEnv *env, jobject jffmpeg, jint peer, jboolean compatibility)
{
    FFMPEGWrapper *wrapper;    

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (!wrapper->codec_context)
        return (jboolean) 0;

    if (compatibility)
      wrapper->codec_context->flags |= CODEC_FLAG_MODE_A_ONLY; // Mode A only
    else
      wrapper->codec_context->flags &= ~CODEC_FLAG_MODE_A_ONLY; // Mode A + B

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeEncoder
 * Method:    convert
 * Signature: (ILjava/lang/Object;JLjava/lang/Object;JJ)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeEncoder_convert
  (JNIEnv *env, jobject jffmpeg, jint peer,
   jobject jinBuffer, jlong inBytes, jint inBufSize, jint inOffset, jint inLength,
   jobject joutBuffer, jlong outBytes, jint outLength)
{
    unsigned char *inBuf  = (unsigned char *) inBytes;
    unsigned char *outBuf = (unsigned char *) outBytes;
    FFMPEGWrapper *wrapper;    
    AVCodecContext *ctx;
    AVFrame        *pict;
    unsigned char *rawBuf; // encode from this buffer
    int rawBufSize = inLength + INPUT_BUFFER_PADDING_SIZE;
    unsigned char *encBuf; // encode to this buffer
    int encBufSize, encSize, retval = 1;

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;
    ctx = wrapper->codec_context;
    pict = wrapper->picture;

    if (inBytes == 0)
        inBuf = (unsigned char *) (*env)->GetByteArrayElements(env, (jbyteArray) jinBuffer, NULL);

    if (outBytes == 0)
        outBuf = (unsigned char *) (*env)->GetByteArrayElements(env, (jbyteArray) joutBuffer, NULL);

    if (wrapper->rtp_mode) {
        // First, process any "chunks" that are in the list before doing a
        // REAL encode operation. The previous frame might not be finished yet.
        if (wrapper->headChunks != NULL) {
            jffmpeg_processList(env, jffmpeg, wrapper, outBuf, outLength);
            goto done;
        }

        // Otherwise use a temporary buffer for the encoded frame.
        assert(wrapper->encode_buf != NULL);
        encBuf  = (unsigned char *) wrapper->encode_buf;
        encBufSize = wrapper->encode_buf_size;
    } else {
        encBuf = outBuf;
        encBufSize = outLength;
    }

    if (inOffset + rawBufSize > inBufSize) {
        // Use a temporary buffer to ensure correct padding for input.
        if (rawBufSize > wrapper->copy_buf_size) {
#ifdef DEBUG
            printf("reallocate copy_buf (%d bytes)\n", rawBufSize);
#endif
            wrapper->copy_buf = realloc(wrapper->copy_buf, rawBufSize);
            wrapper->copy_buf_size = rawBufSize;
        }
        rawBuf = (unsigned char *) wrapper->copy_buf;
#ifdef DEBUG
        printf("copy input buffer (%d bytes)\n", (int)inLength);
#endif
        memcpy(rawBuf, inBuf + inOffset, inLength);
    } else {
        rawBuf = inBuf + inOffset;
    }

    // Set the picture data to point to the in buffer from JNI.
    // This should tell the encoder to read straight from the JNI buffer.
    pict->data[0] = rawBuf;                                            // Y offset
    pict->data[1] = pict->data[0] + (ctx->width * ctx->height);        // U offset
    pict->data[2] = pict->data[1] + ((ctx->width * ctx->height) >> 2); // V offset
    pict->linesize[0] = ctx->width;
    pict->linesize[1] = pict->linesize[2] = ctx->width >> 1;

    encSize = avcodec_encode_video(ctx, encBuf, encBufSize, pict);

    if (encSize < 0) {
        fprintf(stderr, "Error while encoding frame\n");
        retval = 0;
        goto done;
    }

    if (wrapper->rtp_mode) {
        assert(wrapper->headChunks != NULL);
        jffmpeg_processList(env, jffmpeg, wrapper, outBuf, outLength);
    } else {
        // The input frame is consumed.
        jffmpeg_set_inputDone(env, jffmpeg, 1);

        // The output frame is filled, set its size.
        jffmpeg_set_outputSize(env, jffmpeg, encSize);
    }

done:
    if (outBytes == 0)
        (*env)->ReleaseByteArrayElements(env, (jbyteArray) joutBuffer, (jbyte *) outBuf, 0);

    if (inBytes == 0)
        (*env)->ReleaseByteArrayElements(env, (jbyteArray) jinBuffer, (jbyte *) inBuf, JNI_ABORT);

    return (jboolean) retval;
}
