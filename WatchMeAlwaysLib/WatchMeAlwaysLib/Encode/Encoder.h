#pragma once
#ifndef RECORDER_H

struct AVCodec;
struct AVCodecContext;
struct AVFrame;
struct AVPacket;
struct SwsContext;
class Frame;

#include <queue>
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


class Encoder {
	typedef std::unique_ptr<AVCodecContext, void(*)(AVCodecContext*)> AVCodecContextPtr;
	typedef std::unique_ptr<AVFrame, void(*)(AVFrame*)> AVFramePtr;
	typedef std::unique_ptr<AVPacket, void(*)(AVPacket*)> AVPacketPtr;
	typedef std::unique_ptr<SwsContext, void(*)(SwsContext*)> SwsContextPtr;
	AVCodecContextPtr codecCtx_;
	AVFramePtr workingFrame_;
	AVPacketPtr packet_;
	SwsContextPtr swsCtx_;
	std::deque<FramePtr> frameQueue_;
	// int currentFrame_;
	// int recordCount_;
	RecordingQuality quality_;
	float replayLength_;

public:
	Encoder();
	~Encoder();
	bool StartEncoding(const RecordingParameters& parameters);
	// width * height * 3 must be size of pixels.
	bool EncodeFrame(const uint8_t* const pixels, int width, int height, float elapsedSeconds);
	bool FinishEncoding(const std::string& filename);

private:
	void clear();
	bool encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, float elapsedSeconds);
};

typedef std::unique_ptr<Encoder> RecorderPtr;

#endif