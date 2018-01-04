#pragma once
#ifndef RECORDER_H

#include <stdint.h>
#include <string>

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class Frame;

const int FPS = 25;
const int MaxFrameNum = 25 * 60 * 2; // 2 minutes

class Recorder {

	AVCodec *codec;
	AVCodecContext *c;
	AVFrame *frame;
	AVPacket *pkt;
	Frame* frames[MaxFrameNum];
	int currentFrame;
	int recordCount;

public:
	Recorder();
	bool StartRecording(int width, int height);
	bool AddFrame(uint8_t* pixels, float timeStamp, int imgWidth, int imgHeight);
	bool FinishRecording(const std::string& filename);

private:
	bool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt);
};

#endif