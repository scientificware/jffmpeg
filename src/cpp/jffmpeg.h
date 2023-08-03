#ifndef __JFFMPEG_H__
#define __JFFMPEG_H__

#include <libavcodec/avcodec.h>
#include "yuv2rgb.h"

#define J_H263 "h263"
#define J_H263_RTP "h263/rtp"
#define J_MPEG "mpeg"

#define INPUT_BUFFER_PADDING_SIZE 8

// Because FFMPEG processes a whole frame at once, we must temporarily cache
// pointers to each RTP packet in order to feed them to JMF upon demand.
typedef struct rtpChunk {
    void *header;
    int hdrSize;
    void *data;
    int dataSize;
    struct rtpChunk *next;
} rtpChunk;

/*
 * Wrapper structure that holds relevant data per codec instance.
 */
typedef struct {
	AVCodec *codec;
	AVCodecContext *codec_context;
	AVFrame *picture;

	// temporary buffer used for encoding "h263/rtp" ONLY
	void *encode_buf;
	int encode_buf_size;

	// temporary buffer used to ensure correct padding for input
	void *copy_buf;
	int copy_buf_size;

	// list of pointers to RTP packets
	rtpChunk *headChunks;
	rtpChunk *tailChunks;

	// variable used to activate RTP mode
	int rtp_mode;

	// variable used for YUV420P to RGB24 conversion
	Converter* cnv;
} FFMPEGWrapper;

#endif
