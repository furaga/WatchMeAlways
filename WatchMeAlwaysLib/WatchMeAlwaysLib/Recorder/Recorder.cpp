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

Recorder::Recorder() :
	codec_(nullptr),
	ctx_(nullptr),
	workingFrame_(nullptr),
	pkt_(nullptr),
	currentFrame(0),
	recordCount_(0),
	quality_(RECORDING_QUALITY_MEDIUM),
	recordFrameLength_(MaxFrameNum)
{
	//frames_.resize(recordFrameLength_);
	//for (int i = 0; i < frames_.size(); i++) {
	//	frames_[i] = nullptr;
	//}
}

bool Recorder::StartRecording(const RecordingParameters& params) {

	// Find H264 codec (libx264)
	avcodec_register_all();
	codec_ = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec_) {
		UnityDebugCpp::Error("Codec 'AV_CODEC_ID_H264' not found\n");
		return false;
	}

	// Create codec context
	ctx_ = avcodec_alloc_context3(codec_);
	if (!ctx_) {
		UnityDebugCpp::Error("Could not allocate video codec context\n");
		return false;
	}

	// Create avpacket
	pkt_ = av_packet_alloc();
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
	if (codec_->id == AV_CODEC_ID_H264) {
		// High quality encoding
		av_opt_set(ctx_->priv_data, "preset", getPresetString(params.Quality), 0);
	}

	// Initialize codec context
	int ret = avcodec_open2(ctx_, codec_, NULL);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not open codec: H264\n");
		return false;
	}

	// Create avframe
	workingFrame_ = av_frame_alloc();
	if (!workingFrame_) {
		UnityDebugCpp::Error("Could not allocate video frame\n");
		return false;
	}
	workingFrame_->format = ctx_->pix_fmt;
	workingFrame_->width = ctx_->width;
	workingFrame_->height = ctx_->height;
	ret = av_frame_get_buffer(workingFrame_, 32);
	if (ret < 0) {
		UnityDebugCpp::Error("Could not allocate the video frame data\n");
		return false;
	}

	// Reset Encoded frames
	for (int i = 0; i < frames_.size(); i++) {
		SAFE_DELETE(frames_[i]);
	}

	if (params.ReplayLength <= 0 || params.ReplayLength >= 1000) {
		UnityDebugCpp::Error("Replay length (%f) is invalid. It must be > 0.0 and < 1000", params.ReplayLength);
		return false;
	}

	if (params.Fps <= 0 || 120 <= params.Fps) {
		UnityDebugCpp::Error("FPS (%f) is invalid. It must be > 0 and < 120", params.Fps);
		return false;
	}

	int newFrameSize = (int)(params.ReplayLength * params.Quality);
	frames_.resize(newFrameSize);
	for (int i = 0; i < frames_.size(); i++) {
		frames_[i] = nullptr;
	}

	// Reset frame counter
	currentFrame = 0;
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
bool Recorder::AddFrame(uint8_t* pixels, float timeStamp, int imgWidth, int imgHeight)
{
	int ret = av_frame_make_writable(workingFrame_);
	if (ret < 0) {
		return false;
	}

	// convert rgb -> yuv420
	// cf. https://stackoverflow.com/questions/16667687/how-to-convert-rgb-from-yuv420p-for-ffmpeg-encoder
	SwsContext * c = sws_getContext(
		imgWidth, imgHeight, AV_PIX_FMT_RGB24,
		imgWidth, imgHeight, AV_PIX_FMT_YUV420P,
		0, 0, 0, 0);
	int inLinesize[1] = { 3 * imgWidth };
	//pixels[0] += inLinesize[0] * (imgHeight - 1);
	//inLinesize[0] = -inLinesize[0];

	sws_scale(c, &pixels, inLinesize, 0, imgHeight, workingFrame_->data, workingFrame_->linesize);
	FlipFrameJ420(workingFrame_);

	workingFrame_->pts = (int)timeStamp; // todo

	bool succeeded = encode(ctx_, workingFrame_, pkt_);
	if (!succeeded) {
		return false;
	}

	return true;
}

bool Recorder::FinishRecording(const std::string& filename)
{
	// flush
	bool succeeded = encode(ctx_, NULL, pkt_);
	if (!succeeded) {
		return false;
	}

	FILE *f;
	errno_t err = fopen_s(&f, filename.c_str(), "wb");
	if (err) {
		UnityDebugCpp::Error("Could not open " + filename + "\n");
		return false;
	}

	for (int i = 0; i < recordCount_; i++) {
		int index = (currentFrame - recordCount_ + i + (int)frames_.size()) % frames_.size();
		fwrite(frames_[index]->GetData(), 1, frames_[index]->GetDataSize(), f);
	}

	// add sequence end code to have a real MPEG file
	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	avcodec_free_context(&ctx_);
	av_frame_free(&workingFrame_);
	av_packet_free(&pkt_);

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

		printf("Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		//char str[128] = { 0 };
		//sprintf_s(str, 128, "Write packet %3" PRId64 " (size=%5d)\n", pkt->pts, pkt->size);
		//UnityDebugCpp::Info(str);

		assert(frames_[currentFrame] == nullptr);
		frames_[currentFrame] = new Frame(pkt->data, pkt->size);

		currentFrame = (currentFrame + 1) % frames_.size();
		if (recordCount_ < frames_.size()) {
			recordCount_++;
		}

		av_packet_unref(pkt);
	}

	return true;
}
