#include "stdafx.h"

extern "C" {
	#include <libavcodec/avcodec.h>
	#include <libavutil/opt.h>
	#include <libavutil/imgutils.h>
	DllExport float FooPluginFunction();
	DllExport int TestFFMPEG(int);
}

float counter = 0.0f;

float FooPluginFunction() {
	return counter++;
}

 
static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt,
	FILE *outfile)
{
	int ret;

	/* send the frame to the encoder */
//	if (frame)
//		printf("Send frame %3"PRId64"\n", frame->pts);
 
	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		fprintf(stderr, "Error sending a frame for encoding\n");
		exit(100);
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return;
		else if (ret < 0) {
			fprintf(stderr, "Error during encoding\n");
			exit(101);
		}

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		fwrite(pkt->data, 1, pkt->size, outfile);
		av_packet_unref(pkt);
	}
}

int TestFFMPEG(int frameCount)
{
	const char *filename;
	const AVCodec *codec;
	AVCodecContext *c = NULL;
	int i, ret, x, y;
	FILE *f;
	AVFrame *frame;
	AVPacket *pkt;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };

	filename = "C:\\Users\\furaga\\Documents\\tmpdata\\sample-unity.h264";
	avcodec_register_all();

	/* find the mpeg1video encoder */
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		fprintf(stderr, "Codec '%s' not found\n", "AV_CODEC_ID_H264");
		return 1;
	}

	c = avcodec_alloc_context3(codec);
	if (!c) {
		fprintf(stderr, "Could not allocate video codec context\n");
		return 1;
	}

	pkt = av_packet_alloc();
	if (!pkt)
		return 1;

	/* put sample parameters */
	c->bit_rate = 400000;
	/* resolution must be a multiple of two */
	c->width = 352;
	c->height = 288;
	/* frames per second */
	c->time_base = { 1, 25 };
	c->framerate = { 25, 1 };

	/* emit one intra frame every ten frames
	* check frame pict_type before passing frame
	* to encoder, if frame->pict_type is AV_PICTURE_TYPE_I
	* then gop_size is ignored and the output of encoder
	* will always be I frame irrespective to gop_size
	*/
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	/* open it */
	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		fprintf(stderr, "Could not open codec: %s\n", av_make_error_string({ 0 }, AV_ERROR_MAX_STRING_SIZE, ret));
		return 1;
	}

	errno_t err = fopen_s(&f, filename, "wb");
	if (err) {
		fprintf(stderr, "Could not open %s\n", filename);
		return err + 10000;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		return 3;
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		return 4;
	}

	/* encode 1 second of video */
	for (i = 0; i < frameCount; i++) {
		fflush(stdout);

		/* make sure the frame data is writable */
		ret = av_frame_make_writable(frame);
		if (ret < 0)
			return 5;

		/* prepare a dummy image */
		/* Y */
		for (y = 0; y < c->height; y++) {
			for (x = 0; x < c->width; x++) {
				frame->data[0][y * frame->linesize[0] + x] = x + y + i * 3;
			}
		}

		/* Cb and Cr */
		for (y = 0; y < c->height / 2; y++) {
			for (x = 0; x < c->width / 2; x++) {
				frame->data[1][y * frame->linesize[1] + x] = 128 + y + i * 2;
				frame->data[2][y * frame->linesize[2] + x] = 64 + x + i * 5;
			}
		}

		frame->pts = i;

		/* encode the image */
		encode(c, frame, pkt, f);
	}

	/* flush the encoder */
	encode(c, NULL, pkt, f);

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return 0;
}
