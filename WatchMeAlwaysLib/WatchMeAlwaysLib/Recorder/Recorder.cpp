#include "stdafx.h"

#include "Recorder.h"
#include "Frame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
}

Recorder::Recorder() :
	codec(nullptr),
	c(nullptr),
	frame(nullptr),
	pkt(nullptr),
	currentFrame(0),
	recordCount(0)
{
	for (int i = 0; i < MaxFrameNum; i++) {
		frames[i] = nullptr;
	}
}

bool Recorder::StartRecording() {

	// Find H264 codec (libx264)
	avcodec_register_all();
	codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		UnityDebugCpp::Error("Codec 'AV_CODEC_ID_H264' not found\n");
		return false;
	}

	// Create codec context
	c = avcodec_alloc_context3(codec);
	if (!c) {
		UnityDebugCpp::Error("Could not allocate video codec context\n");
		return false;
	}

	// Create avpacket
	pkt = av_packet_alloc();
	if (!pkt) {
		return false;
	}

	// Setting of encoding
	c->bit_rate = 400000;
	c->width = 352; // not free resolution (what restriction?)
	c->height = 288;// not free resolution (what restriction?)
	c->time_base = { 1, FPS };
	c->framerate = { FPS, 1 };
	c->gop_size = 10;
	c->max_b_frames = 1;
	c->pix_fmt = AV_PIX_FMT_YUV420P; // if h264, this must be yuv420p
	if (codec->id == AV_CODEC_ID_H264) {
		// High quality encoding
		av_opt_set(c->priv_data, "preset", "slow", 0);
	}

	// Initialize codec context
	int ret = avcodec_open2(c, codec, NULL);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not open codec: H264\n");
		return false;
	}

	// Create avframe
	frame = av_frame_alloc();
	if (!frame) {
		UnityDebugCpp::Error("Could not allocate video frame\n");
		return false;
	}
	frame->format = c->pix_fmt;
	frame->width = c->width;
	frame->height = c->height;
	ret = av_frame_get_buffer(frame, 32);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not allocate the video frame data\n");
		return false;
	}

	// Reset Encoded frames
	for (int i = 0; i < MaxFrameNum; i++) {
		SAFE_DELETE(frames[i])
	}

	// Reset frame counter
	currentFrame = 0;
	recordCount = 0;

	return true;
}

bool Recorder::AddFrame(uint8_t* pixels, float timeStamp, int linesize)
{
	int ret = av_frame_make_writable(frame);
	if (ret < 0) {
		return false;
	}

	// TODO: convert rgb -> yuv;

	for (int y = 0; y < c->height; y++) {
		for (int x = 0; x < c->width; x++) {
			frame->data[0][y * frame->linesize[0] + x] = pixels[3 * x + y * linesize + 0];
		}
	}

	for (int y = 0; y < c->height / 2; y++) {
		for (int x = 0; x < c->width / 2; x++) {
			frame->data[1][y * frame->linesize[1] + x] = pixels[3 * x * 2 + y * 2 * linesize + 1];
			frame->data[2][y * frame->linesize[2] + x] = pixels[3 * x * 2 + y * 2 * linesize + 2];
		}
	}

	frame->pts = (int)timeStamp; // todo

	bool succeeded = encode(c, frame, pkt);
	if (!succeeded) {
		return false;
	}

	return true;
}

bool Recorder::FinishRecording(const std::string& filename)
{
	// second arg is NULL => flush
	bool succeeded = encode(c, NULL, pkt);
	if (!succeeded) {
		return false;
	}

	FILE *f;
	errno_t err = fopen_s(&f, filename.c_str(), "wb");
	if (err) {
		UnityDebugCpp::Error("Could not open " + filename + "\n");
		return false;
	}

	for (int i = 0; i < recordCount; i++) {
		int index = (currentFrame - recordCount + i + MaxFrameNum) % MaxFrameNum;
		fwrite(frames[index]->GetData(), 1, frames[index]->GetDataSize(), f);
	}

	/* add sequence end code to have a real MPEG file */
	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&c);
	av_frame_free(&frame);
	av_packet_free(&pkt);

	return true;
}


bool Recorder::encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt)
{
	int ret;

	ret = avcodec_send_frame(enc_ctx, frame);
	if (ret < 0) {
		UnityDebugCpp::Error("Error sending a frame for encoding\n");
		return false;
	}

	while (ret >= 0) {
		ret = avcodec_receive_packet(enc_ctx, pkt);
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) {
			return true;
		}
		else if (ret < 0) {
			UnityDebugCpp::Error("Error during encoding\n");
			return false;
		}

		char str[128] = { 0 };
		sprintf_s(str, 128, "Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		UnityDebugCpp::Info(str);

		assert(frames[currentFrame] == nullptr);
		frames[currentFrame] = new Frame(pkt->data, pkt->size);

		currentFrame = (currentFrame + 1) % MaxFrameNum;
		if (recordCount < MaxFrameNum) {
			recordCount++;
		}

		av_packet_unref(pkt);
	}

	return true;
}
