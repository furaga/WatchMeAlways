#pragma once
#ifndef RECORDER_H

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
class Frame;

#include <vector>
#include <memory>

typedef std::unique_ptr<Frame> FramePtr;

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
	typedef std::unique_ptr<AVCodecContext, void(*)(AVCodecContext*)> AVCodecContextPtr;
	typedef std::unique_ptr<AVFrame, void(*)(AVFrame*)> AVFramePtr;
	typedef std::unique_ptr<AVPacket, void(*)(AVPacket*)> AVPacketPtr;
	AVCodecContextPtr ctx_;
	AVFramePtr workingFrame_;
	AVPacketPtr pkt_;
	std::vector<FramePtr> frames_;
	int currentFrame_;
	int recordCount_;
	RecordingQuality quality_;
	int recordFrameLength_;

public:
	Recorder();
	~Recorder();
	bool StartRecording(const RecordingParameters& parameters);
	// width * height * 3 must be size of pixels.
	bool AddFrame(const uint8_t* const pixels, int width, int height, float timeStamp);
	bool FinishRecording(const std::string& filename);

private:
	void clear();
	bool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt);
};

typedef std::unique_ptr<Recorder> RecorderPtr;

#endif