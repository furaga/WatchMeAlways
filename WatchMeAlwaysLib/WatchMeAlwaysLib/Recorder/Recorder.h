#pragma once
#ifndef RECORDER_H

#include <stdint.h>
#include <string>
#include <vector>

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class Frame;

const int FPS = 25;
const int MaxFrameNum = 25 * 60 * 2; // 2 minutes

enum RecordingQuality {
	RECORDING_QUALITY_ULTRAFAST = 0,
	RECORDING_QUALITY_SUPERFAST,
	RECORDING_QUALITY_VERYFAST,
	RECORDING_QUALITY_FASTER,
	RECORDING_QUALITY_FAST,
	RECORDING_QUALITY_MEDIUM, // default
	RECORDING_QUALITY_SLOW,
	RECORDING_QUALITY_SLOWER,
	RECORDING_QUALITY_VERYSLOW,
};

struct RecordingParameters {
	int Width;
	int Height;
	float ReplayLength; // in seconds
	float Fps;
	RecordingQuality Quality;
	RecordingParameters(int width, int height, float seconds, float fps, RecordingQuality quality)
		: Width(width),
		Height(height),
		ReplayLength(seconds),
		Fps(fps),
		Quality(quality)
	{
	}
};

class Recorder {

	AVCodec *codec_;
	AVCodecContext *ctx_;
	AVFrame *workingFrame_;
	AVPacket *pkt_;
	std::vector<Frame*> frames_;
	int currentFrame;
	int recordCount_;
	RecordingQuality quality_;
	int recordFrameLength_;

public:
	Recorder();
	bool StartRecording(const RecordingParameters& parameters);
	bool AddFrame(uint8_t* pixels, float timeStamp, int imgWidth, int imgHeight);
	bool FinishRecording(const std::string& filename);

private:
	bool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt);
};

#endif