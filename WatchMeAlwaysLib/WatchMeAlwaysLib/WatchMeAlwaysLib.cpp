#include "stdafx.h"

#include <memory>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
	DllExport int StartRecording(int width, int height);
	DllExport int AddFrame(uint8_t* pixels, float timeStamp, int lineSize);
	DllExport int FinishRecording();
}



const AVCodec *codec;
AVCodecContext *c = NULL;
int i, ret, x, y;
AVFrame *frame;
AVPacket *pkt;

const int MaxFrameNum = 25 * 60 * 2;
uint8_t* frames[MaxFrameNum];
int frameDataSizes[MaxFrameNum];
int currentFrame = 0;
int recordCount = 0;


static void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
{
	int ret;

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

		// todo: re-new if size changed.
		if (frames[currentFrame] == nullptr) {
			frames[currentFrame] = new uint8_t[pkt->size];
		}

		std::memcpy(frames[currentFrame], pkt->data, pkt->size);
		frameDataSizes[currentFrame] = pkt->size;
		currentFrame = (currentFrame + 1) % MaxFrameNum;
		if (recordCount < MaxFrameNum) {
			recordCount++;
		}

		av_packet_unref(pkt);
	}
}

int StartRecording(int width, int height)
{
	avcodec_register_all();

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
	if (!pkt) {
		return 1;
	}

	c->bit_rate = 400000;

	// not free resolution (what restriction?)
	c->width = 352;
	c->height = 288;
	c->time_base = { 1, 25 };
	c->framerate = { 25, 1 };
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P;

	if (codec->id == AV_CODEC_ID_H264)
		av_opt_set(c->priv_data, "preset", "slow", 0);

	ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		//		fprintf(stderr, "Could not open codec: %s\n", av_make_error_string({ 0 }, AV_ERROR_MAX_STRING_SIZE, ret));
		return 1;
	}

	frame = av_frame_alloc();
	if (!frame) {
		fprintf(stderr, "Could not allocate video frame\n");
		return 1;
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;

	ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		fprintf(stderr, "Could not allocate the video frame data\n");
		return 1;
	}

	// reset
	for (int i = 0; i < MaxFrameNum; i++) {
		if (frames[i] != nullptr) {
			delete frames[i];
			frames[i] = nullptr;
		}
	}
	currentFrame = 0;
	recordCount = 0;

	return 0;
}

int AddFrame(uint8_t* pixels, float timeStamp, int linesize)
{
	ret = av_frame_make_writable(frame);
	if (ret < 0)
		return 5;

	for (y = 0; y < c->height; y++) {
		for (x = 0; x < c->width; x++) {
			frame->data[0][y * frame->linesize[0] + x] = pixels[3 * x + y * linesize + 0];
		}
	}

	for (y = 0; y < c->height / 2; y++) {
		for (x = 0; x < c->width / 2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = pixels[3 * x * 2 + y * 2 * linesize + 1];
			frame->data[2][y * frame->linesize[2] + x] = pixels[3 * x * 2 + y * 2 * linesize + 2];
		}
	}

	frame->pts = (int)timeStamp; // todo

	encode(c, frame, pkt);
	return 0;
}

int FinishRecording()
{
	// second arg is NULL => flush
	encode(c, NULL, pkt);

	const char *filename;
	FILE *f;
	uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	filename = "C:\\Users\\furaga\\Documents\\tmpdata\\sample-unity.h264";

	errno_t err = fopen_s(&f, filename, "wb");
	if (err) {
		fprintf(stderr, "Could not open %s\n", filename);
		return err + 10000;
	}

	for (i = 0; i < recordCount; i++) {
		int index = (currentFrame - recordCount + i + MaxFrameNum) % MaxFrameNum;
		fwrite(frames[index], 1, frameDataSizes[index], f);
	}

	/* add sequence end code to have a real MPEG file */
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return 0;
}