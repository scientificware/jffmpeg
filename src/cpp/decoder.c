/*
 * Authors:  Francisco J. Cabello. fjcabello@visual-tools.com
 *           Guilhem Tardy. gtardy@salyens.com
 *
 * Version control
 * ===============
 * $Id: decoder.c,v 1.9 2004/11/01 01:52:12 davidstuart Exp $ 
 * 
 * Revision 1.2  2004/02/19 Guilhem Tardy (gravsten@yahoo.com)
 * Major rewrite, now fully supports H.263/RTP (ffmpeg 0.4.7) and conversion to RGB24.
 *
 * Description
 * ============
 * JMF wrapper for FFMPEG 
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
#include <jni.h>
#include <libavcodec/avcodec.h>
#include "net_sourceforge_jffmpeg_ffmpegnative_NativeDecoder.h"
#include "jffmpeg.h"
#include "yuv2rgb.h"

#ifndef CODEC_FLAG_RFC2190
#define CODEC_FLAG_RFC2190 0
#endif

//#define DEBUG

#ifdef DEBUG
// Prints the first 32 bytes of a buffer, up to length
static void jffmpeg_printBuf(unsigned char *buf, int length) {
	int i=0;
	for(i=0; (i<48 && i<length); i++) {
		printf("%.2x ", buf[i]);
		if (((i + 1) % 8 ) == 0)
			printf("\n");
	}
	printf("\n\n");
}
#endif

/*
 * Class:     net_sourceforge_jffmpeg_NativeDecoder
 * Method:    open_codec
 * Signature: (ILjava/lang/String;)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeDecoder_open_1decoder
  (JNIEnv *env, jobject jffmpeg, jstring codec_name, jboolean rtp, jboolean setTruncated, jboolean yuv2rgb, jint depth, jint rMask, jint gMask, jint bMask, jint width, jint height)
{
    FFMPEGWrapper *wrapper;    
    enum CodecID codec_required;
    static int FFMPEG_init = 0;
    jclass clazz;
    jfieldID fidPeer;
    jint peerVal;
    char * str;
    AVCodecContext * ctx;

    if(!FFMPEG_init) {
        /* must be called before using avcodec lib */
        avcodec_init();

        /* register only the codec you need to have smaller code */
        register_avcodec(&h263_decoder);
#if FFMPEG_VERSION_INT >= 0x409
        register_avcodec(&mpeg1video_decoder);
        register_avcodec(&mpeg2video_decoder);
#else
        register_avcodec(&mpeg_decoder);
#endif
        register_avcodec(&msmpeg4v1_decoder);
        register_avcodec(&msmpeg4v2_decoder);
        register_avcodec(&msmpeg4v3_decoder);
        register_avcodec(&mpeg4_decoder);
        register_avcodec(&wmv1_decoder);
        register_avcodec(&wmv2_decoder);
        register_avcodec(&mjpeg_decoder);

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
    if(!wrapper)
        return (jboolean) 0;

    // Make sure the wrapper is zeroed
    memset(wrapper,0,sizeof(FFMPEGWrapper));

    // Set the "peer" variable in java class
    (*env)->SetIntField(env, jffmpeg, fidPeer, (jint)wrapper);

    // Find matching ffmpeg codec using the codec_name passed in
    str = (*env)->GetStringUTFChars(env, codec_name, 0);
    wrapper->rtp_mode = rtp;
    wrapper->codec = avcodec_find_decoder_by_name(str);

    if (!wrapper->codec ) {
        fprintf(stderr, "codec not found %s\n", str);
        (*env)->ReleaseStringUTFChars(env, codec_name, str);
        return (jboolean) 0;
    }
    (*env)->ReleaseStringUTFChars(env, codec_name, str);

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

    ctx = wrapper->codec_context;

    /* Set all parameters to the encoder before opening */
    ctx->flags |= CODEC_FLAG_INPUT_PRESERVED;
    ctx->flags |= CODEC_FLAG_EMU_EDGE;
    if ( setTruncated ) {
        ctx->flags |= CODEC_FLAG_TRUNCATED;
    }

    /* For some codecs, such as msmpeg4 and mpeg4, width and height
     * MUST be initialized there because these info are not available
     * in the bitstream.
     * For H.263, the decoder will automatically reset these values
     * with correct values.
     */
    ctx->width  = width;
    ctx->height = height;

    ctx->workaround_bugs = 0; // no workaround for buggy H.263 implementations
    ctx->error_concealment = FF_EC_GUESS_MVS | FF_EC_DEBLOCK;
    ctx->error_resilience = FF_ER_CAREFULL;

    if (wrapper->rtp_mode)
        ctx->flags |= CODEC_FLAG_RFC2190;
    else
        ctx->flags &= ~CODEC_FLAG_RFC2190;

    /* open it */
    if (avcodec_open(wrapper->codec_context, wrapper->codec) < 0) {
        fprintf(stderr, "could not open codec\n");
        return (jboolean) 0;
    }    

    if (yuv2rgb) {
        wrapper->cnv = yuv2rgb_get_converter(depth, rMask, gMask, bMask);
        if (wrapper->cnv == NULL) {
            fprintf(stderr, "could not get yuv2rgb converter\n");
            return (jboolean) 0;
        }    
    }

#ifdef DEBUG
    printf("jffmpeg decoder opened\n");
#endif

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeDecoder
 * Method:    close_codec
 * Signature: (I)Z
 */
JNIEXPORT jboolean JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeDecoder_close_1decoder
  (JNIEnv *env, jobject jffmpeg, jint peer)
{
    FFMPEGWrapper *wrapper;
    jclass clazz;
    jfieldID fidPeer;

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;

    if (wrapper->codec_context->codec != NULL)
        avcodec_close(wrapper->codec_context);

    if (wrapper->cnv != NULL)
       release_converter(wrapper->cnv);

    free(wrapper->copy_buf);
    av_free(wrapper->picture);
    av_free(wrapper->codec_context);
    free(wrapper);

    // Unset the "peer" variable in java class
    clazz = (*env)->GetObjectClass(env, jffmpeg);
    fidPeer = (*env)->GetFieldID(env, clazz, "peer", "I");
    (*env)->SetIntField(env, jffmpeg, fidPeer, (jint)0);

#ifdef DEBUG
    printf("jffmpeg decoder closed\n");
#endif

    return (jboolean) 1;
}

/*
 * Class:     net_sourceforge_jffmpeg_NativeDecoder
 * Method:    convert
 * Signature: (ILjava/lang/Object;JLjava/lang/Object;JJ)Z
 */
JNIEXPORT jint JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeDecoder_convert
  (JNIEnv *env, jobject jffmpeg, jint peer,
   jobject jinBuffer, jint inBufSize, jint inOffset, jint inLength,
   jobject joutBuffer, jint outLength, jint eof)
{
    unsigned char *inBuf;
    unsigned char *outBuf;
    FFMPEGWrapper *wrapper;
    AVCodecContext *ctx;
    AVFrame *pict;
    unsigned char *encBuf; // decode from this buffer
    int encBufSize = inLength + INPUT_BUFFER_PADDING_SIZE;
    int got_picture, retval = 1;
    int numberOfBytesUsed;

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;
    ctx = wrapper->codec_context;
    pict = wrapper->picture;

    inBuf = (unsigned char *) (*env)->GetByteArrayElements(env, (jbyteArray) jinBuffer, NULL);

    outBuf = (unsigned char *) (*env)->GetByteArrayElements(env, (jbyteArray) joutBuffer, NULL);

#ifdef DEBUG
	jffmpeg_printBuf(inBuf, inBufSize);
#endif

    if (inLength > 0) {
        if (inOffset + encBufSize > inBufSize) {
            // Use a temporary buffer to ensure correct padding for input.
            if (encBufSize > wrapper->copy_buf_size) {
printf("reallocate copy_buf (%d bytes)\n", encBufSize);
                wrapper->copy_buf_size = encBufSize;
                wrapper->copy_buf = realloc(wrapper->copy_buf, encBufSize);
            }
            encBuf = (unsigned char *) wrapper->copy_buf;
//printf("copy input buffer (%d bytes)\n", (int)inLength);
            memcpy(encBuf, inBuf + inOffset, inLength);
        } else {
            encBuf = inBuf + inOffset;
        }

        // Some decoders might overread/segfault if the first 23 bits of padding are not 0.
        encBuf[inLength] = 0;
        encBuf[inLength+1] = 0;
        encBuf[inLength+2] = 0;
    }

    if (wrapper->rtp_mode) {
        if (inLength > 0) {
            numberOfBytesUsed = avcodec_decode_video(ctx, pict, &got_picture, encBuf, inLength);

            if (!eof)
                goto done;
        }

        // full frame received, now process it...
        numberOfBytesUsed = avcodec_decode_video(ctx, pict, &got_picture, NULL, -1);
    }
    else {
        numberOfBytesUsed = avcodec_decode_video(ctx, pict, &got_picture, encBuf, inLength);
    }

    if (got_picture) {
        int width  = ctx->width;
        int height = ctx->height;
        int size = width * height;

        if (width == 0 || height == 0) {
            fprintf(stderr, "Image dimension is 0\n");
            retval = 0;
            goto done;
        }

        if (wrapper->cnv != NULL) {

#ifdef DEBUG
printf("yuv2rgb(%ix%i @ Y0x%.8x U0x%.8x V0x%.8x)\n", width, height, (int)pict->data[0], (int)pict->data[1], (int)pict->data[2]);
#endif

            yuv2rgb(wrapper->cnv, pict->data[0], pict->data[1], pict->data[2], outBuf, width, height);
        } else {
            // NOTE: JMF expects contiguous planes, and stride = width (width/2, resp.)

#ifdef DEBUG
printf("yuv420 for %ix%i @ Y0x%.8x U0x%.8x V0x%.8x\n", width, height, (int)pict->data[0], (int)pict->data[1], (int)pict->data[2]);
#endif

            if (pict->data[1] == pict->data[0] + size
                && pict->data[2] == pict->data[1] + (size >> 2))
                memcpy(outBuf, pict->data[0], size + (size >> 1));
            else {
                unsigned char *dst = outBuf;
                int i;

                for (i=0; i<3; i++) {
                    unsigned char *src = pict->data[i];
                    int dst_stride = i ? width >> 1 : width;
                    int src_stride = pict->linesize[i];
                    int h = i ? height >> 1 : height;

#ifdef DEBUG
printf("copy plane%i, height %i, stride %i ->%i, 0x%.8x ->0x%.8x\n", i, h, src_stride, dst_stride, (int)src, (int)dst);
#endif

                    if (src_stride==dst_stride) {
                        memcpy(dst, src, dst_stride*h);
                        dst += dst_stride*h;
                    }
                    else {
                        while (h--) {
                            memcpy(dst, src, dst_stride);
                            dst += dst_stride;
                            src += src_stride;
                        }
                    }
                }
            }
        }
    }
    else
        numberOfBytesUsed = 0;

done:
    (*env)->ReleaseByteArrayElements(env, (jbyteArray) joutBuffer, (jbyte *) outBuf, 0);

    (*env)->ReleaseByteArrayElements(env, (jbyteArray) jinBuffer, (jbyte *) inBuf, JNI_ABORT);

    return (jint) numberOfBytesUsed;
}

JNIEXPORT jfloat JNICALL Java_net_sourceforge_jffmpeg_ffmpegnative_NativeDecoder_extractFrameRate
  (JNIEnv * env, jobject ffmpeg, jint peer)
{
    FFMPEGWrapper *wrapper;
    AVCodecContext *ctx;
    AVFrame *pict;

    if (peer == 0)
        return (jboolean) 0;

    wrapper = (FFMPEGWrapper *) peer;
    ctx = wrapper->codec_context;
    pict = wrapper->picture;
    return (jfloat)((float)ctx->frame_rate/(float)ctx->frame_rate_base);
}
