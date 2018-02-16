#include "stdafx.h"

#include "Recorder.h"
#include "Frame.h"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
}

const char* getPresetString(RecordingQuality quality) {
	switch (quality) {
	case RECORDING_QUALITY_ULTRAFAST:
		return "ultrafast";
	case RECORDING_QUALITY_SUPERFAST:
		return "superfast";
	case RECORDING_QUALITY_VERYFAST:
		return "veryfast";
	case RECORDING_QUALITY_FASTER:
		return "faster";
	case RECORDING_QUALITY_FAST:
		return "fast";
	case RECORDING_QUALITY_MEDIUM:
		return "medium";
	case RECORDING_QUALITY_SLOW:
		return "slow";
	case RECORDING_QUALITY_SLOWER:
		return "slower";
	case RECORDING_QUALITY_VERYSLOW:
		return "veryslow";
	default:
		return "medium";
	}
}

void deleteAVCodecContext(AVCodecContext* ptr) { avcodec_free_context(&ptr); }
void deleteAVFrameFree(AVFrame* ptr) { av_frame_free(&ptr); }
void deleteAVPacket(AVPacket* ptr) { av_packet_free(&ptr); }

Recorder::Recorder() :
	ctx_(nullptr, deleteAVCodecContext),
	workingFrame_(nullptr, deleteAVFrameFree),
	pkt_(nullptr, deleteAVPacket),
	currentFrame_(0),
	recordCount_(0),
	quality_(RECORDING_QUALITY_MEDIUM),
	recordFrameLength_(MaxFrameNum)
{
}

Recorder::~Recorder()
{
	clear();
}

bool Recorder::StartRecording(const RecordingParameters& params) {
	clear();

	// Find H264 codec (libx264)
	avcodec_register_all();
	auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		UnityDebugCpp::Error("Codec 'AV_CODEC_ID_H264' not found\n");
		return false;
	}

	// Create codec context
	ctx_.reset(avcodec_alloc_context3(codec));
	if (!ctx_) {
		UnityDebugCpp::Error("Could not allocate video codec context\n");
		return false;
	}

	// Create avpacket
	pkt_.reset(av_packet_alloc());
	if (!pkt_) {
		return false;
	}

	// Setting of encoding
	ctx_->bit_rate = 400000;
	ctx_->width = params.Width;
	ctx_->height = params.Height;
	ctx_->time_base = { 100, (int)(100 * params.Fps) };
	ctx_->framerate = { (int)(100 * params.Fps) , 100 };
	ctx_->gop_size = 10;
	ctx_->max_b_frames = 1;
	ctx_->pix_fmt = AV_PIX_FMT_YUV420P; // if h264, this must be yuv420p
	if (codec->id == AV_CODEC_ID_H264) {
		// High quality encoding
		av_opt_set(ctx_->priv_data, "preset", getPresetString(params.Quality), 0);
	}

	// Initialize codec context
	int ret = avcodec_open2(ctx_.get(), codec, nullptr);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not open codec: H264\n");
		return false;
	}

	// Create avframe
	workingFrame_.reset(av_frame_alloc());
	if (!workingFrame_) {
		UnityDebugCpp::Error("Could not allocate video frame\n");
		return false;
	}
	workingFrame_->format = ctx_->pix_fmt;
	workingFrame_->width = ctx_->width;
	workingFrame_->height = ctx_->height;
	ret = av_frame_get_buffer(workingFrame_.get(), 32);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not allocate the video frame data\n");
		return false;
	}

	// Set encoded frames
	if (params.ReplayLength <= 0 || params.ReplayLength >= 1000) {
		UnityDebugCpp::Error("Replay length (%f) is invalid. It must be > 0.0 and < 1000", params.ReplayLength);
		return false;
	}

	if (params.Fps <= 0 || 120 <= params.Fps) {
		UnityDebugCpp::Error("FPS (%f) is invalid. It must be > 0 and < 120", params.Fps);
		return false;
	}

	int newFrameSize = (int)(params.ReplayLength * params.Fps);
	frames_.resize(newFrameSize);
	for (int i = 0; i < frames_.size(); i++) {
		frames_[i].reset(new Frame());
	}

	UnityDebugCpp::Info("newFrameSize = %f * %f = %d", params.ReplayLength, params.Fps, newFrameSize);

	// Reset frame counter
	currentFrame_ = 0;
	recordCount_ = 0;

	return true;
}

void FlipFrameJ420(AVFrame* pFrame) {
	for (int i = 0; i < 4; i++) {
		if (i) {
			pFrame->data[i] += pFrame->linesize[i] * ((pFrame->height >> 1) - 1);
		}
		else {
			pFrame->data[i] += pFrame->linesize[i] * (pFrame->height - 1);
		}
		pFrame->linesize[i] = -pFrame->linesize[i];
	}
}

bool Recorder::AddFrame(const uint8_t* const pixels, int width, int height, float timeStamp)
{
	if (pixels == nullptr || width <= 0 || height <= 0) {
		UnityDebugCpp::Error("AddFrame: Frame is empty (pixels=%p, width=%d, height=%d)", pixels, width, height);
		return false;
	}

	// check width, height


	int ret = av_frame_make_writable(workingFrame_.get());
	if (ret < 0) {
		printf("AddFrame: av_frame_make_writable failed");
		return false;
	}

	// cf. https://stackoverflow.com/questions/16667687/how-to-convert-rgb-from-yuv420p-for-ffmpeg-encoder
	SwsContext * c = sws_getContext(
		width / 2 * 2, height / 2 * 2, AV_PIX_FMT_BGR24,
		ctx_->width /2 * 2, ctx_->height / 2 * 2, AV_PIX_FMT_YUV420P,
		0, 0, 0, 0
	);

	int inLinesize[1] = { 3 * width };
	sws_scale(c, &pixels, inLinesize, 0, height, workingFrame_->data, workingFrame_->linesize);
	FlipFrameJ420(workingFrame_.get());

	workingFrame_->pts = (int)timeStamp; // todo


	bool succeeded = encode(ctx_.get(), workingFrame_.get(), pkt_.get());
	if (!succeeded) {
		printf("AddFrame: encode failed");
		return false;
	}

	return true;
}

bool Recorder::FinishRecording(const std::string& filename)
{
	if (ctx_ == nullptr || pkt_ == nullptr) {
		UnityDebugCpp::Error("FinishRecording: Recorder is not started. Please call StartRecording().");
		return false;
	}

	// flush
	bool succeeded = encode(ctx_.get(), nullptr, pkt_.get());
	if (!succeeded) {
		return false;
	}

	FILE *f;
	errno_t err = fopen_s(&f, filename.c_str(), "wb");
	if (err) {
		UnityDebugCpp::Error("FinishRecording: Could not open " + filename + "\n");
		return false;
	}

	for (int i = 0; i < recordCount_; i++) {
		int index = (currentFrame_ - recordCount_ + i + (int)frames_.size()) % frames_.size();
		fwrite(frames_[index]->GetData(), 1, frames_[index]->GetDataSize(), f);
	}

	// add sequence end code to have a real MPEG file
	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	clear();

	return true;
}

void Recorder::clear() {
	// clear ffmpeg internal memories
	ctx_.reset(nullptr);
	workingFrame_.reset(nullptr);
	pkt_.reset(nullptr);

	// clear frames
	frames_.clear();

	// clear counters
	currentFrame_ = 0;
	recordCount_ = 0;
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

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);


		// TODO: 超過分の動画データを捨てる
		//int timestamp = 0;
		//while (timestamp - frames_.peek()->GetTimestamp() >= recordFrameLength_) {
		//	frames_.pop();
		//}

		// Should I reuse instance instead of creating new instance?
		// TODO: Fix if this line would be bottle neck.
		frames_[currentFrame_]->SetData(pkt->data, pkt->size, timestamp);

		currentFrame_ = (currentFrame_ + 1) % frames_.size();
		if (recordCount_ < frames_.size()) {
			recordCount_++;
		}

		av_packet_unref(pkt);
	}

	// ここが切れ目？

	return true;
}
