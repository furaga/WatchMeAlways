#include "stdafx.h"

#include "Encoder.h"
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
void deleteSwsContext(SwsContext* ptr) { sws_freeContext(ptr); }

Encoder::Encoder() :
	codecCtx_(nullptr, deleteAVCodecContext),
	workingFrame_(nullptr, deleteAVFrameFree),
	packet_(nullptr, deleteAVPacket),
	swsCtx_(nullptr, deleteSwsContext),
	//currentFrame_(0),
	//recordCount_(0),
	quality_(RECORDING_QUALITY_MEDIUM),
	replayLength_(MaxFrameNum)
{
}

Encoder::~Encoder()
{
	clear();
}

bool Encoder::StartEncoding(const RecordingParameters& params) {
	clear();

	// Find H264 codec (libx264)
	avcodec_register_all();
	auto codec = avcodec_find_encoder(AV_CODEC_ID_H264);
	if (!codec) {
		UnityDebugCpp::Error("Codec 'AV_CODEC_ID_H264' not found\n");
		return false;
	}

	// Create codec context
	codecCtx_.reset(avcodec_alloc_context3(codec));
	if (!codecCtx_) {
		UnityDebugCpp::Error("Could not allocate video codec context\n");
		return false;
	}

	// Create avpacket
	packet_.reset(av_packet_alloc());
	if (!packet_) {
		return false;
	}

	// Setting of encoding
	codecCtx_->bit_rate = 400000;
	codecCtx_->width = params.Width;
	codecCtx_->height = params.Height;
	codecCtx_->time_base = { 100, (int)(100 * params.Fps) };
	codecCtx_->framerate = { (int)(100 * params.Fps) , 100 };
	codecCtx_->gop_size = 10;
	codecCtx_->max_b_frames = 1;
	codecCtx_->pix_fmt = AV_PIX_FMT_YUV420P; // if h264, this must be yuv420p
	if (codec->id == AV_CODEC_ID_H264) {
		// High quality encoding
		av_opt_set(codecCtx_->priv_data, "preset", getPresetString(params.Quality), 0);
	}

	// Initialize codec context
	int ret = avcodec_open2(codecCtx_.get(), codec, nullptr);
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
	workingFrame_->format = codecCtx_->pix_fmt;
	workingFrame_->width = codecCtx_->width;
	workingFrame_->height = codecCtx_->height;
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
	replayLength_ = params.ReplayLength;

	//if (params.Fps <= 0 || 120 <= params.Fps) {
	//	UnityDebugCpp::Error("FPS (%f) is invalid. It must be > 0 and < 120", params.Fps);
	//	return false;
	//}

	// clear queue
	frameQueue_.clear();

	//// Reset frame counter
	//currentFrame_ = 0;
	//recordCount_ = 0;

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

bool Encoder::EncodeFrame(const uint8_t* const pixels, int width, int height, float elapsedSeconds)
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
	swsCtx_.reset(sws_getContext(
		width / 2 * 2, height / 2 * 2, AV_PIX_FMT_BGR24,
		codecCtx_->width /2 * 2, codecCtx_->height / 2 * 2, AV_PIX_FMT_YUV420P,
		0, 0, 0, 0
	));

	int inLinesize[1] = { 3 * width };
	sws_scale(swsCtx_.get(), &pixels, inLinesize, 0, height, workingFrame_->data, workingFrame_->linesize);
	FlipFrameJ420(workingFrame_.get());

	//TODO: time base?
	workingFrame_->pts = (int)(1000 * elapsedSeconds * codecCtx_->time_base.den / codecCtx_->time_base.num);

	//printf("workingFrame_->pts %d \n", workingFrame_->pts);

	bool succeeded = encode(codecCtx_.get(), workingFrame_.get(), packet_.get(), elapsedSeconds);
	if (!succeeded) {
		printf("AddFrame: encode failed");
		return false;
	}

	return true;
}

bool Encoder::FinishEncoding(const std::string& filename)
{
	if (codecCtx_ == nullptr || packet_ == nullptr) {
		UnityDebugCpp::Error("FinishRecording: Recorder is not started. Please call StartRecording().");
		return false;
	}

	// flush
	float timestamp = 0.0f;
	if (frameQueue_.size() >= 1) {
		timestamp = frameQueue_[frameQueue_.size() - 1]->GetTimestamp();
	}
	bool succeeded = encode(codecCtx_.get(), nullptr, packet_.get(), timestamp);
	if (!succeeded) {
		return false;
	}

	FILE *f;
	errno_t err = fopen_s(&f, filename.c_str(), "wb");
	if (err) {
		UnityDebugCpp::Error("FinishRecording: Could not open " + filename + "\n");
		return false;
	}

	for (auto& frame : frameQueue_) {
		fwrite(frame->GetData(), 1, frame->GetDataSize(), f);
	}

	// add sequence end code to have a real MPEG file
	const uint8_t endcode[] = { 0, 0, 1, 0xb7 };
	fwrite(endcode, 1, sizeof(endcode), f);
	fclose(f);

	clear();

	return true;
}

void Encoder::clear() {
	// clear ffmpeg internal memories
	codecCtx_.reset(nullptr);
	workingFrame_.reset(nullptr);
	packet_.reset(nullptr);

	// clear frames
	frameQueue_.clear();

	//// clear counters
	//currentFrame_ = 0;
	//recordCount_ = 0;
}

bool Encoder::encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *pkt, float elapsedSeconds)
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

		// is here slow?
	//	printf("pkt->pts = %d, t = %f\n", pkt->pts, elapsedSeconds);
		frameQueue_.push_back(FramePtr(new Frame(pkt->data, pkt->size, pkt->pts* 0.001f * codecCtx_->time_base.num / codecCtx_->time_base.den)));

		// sort? use priority_queue?
		for (int i = frameQueue_.size() - 1; i >= 0; i--) {
			if (elapsedSeconds - frameQueue_[i]->GetTimestamp() >= replayLength_) {
				frameQueue_.erase(frameQueue_.begin() + i);
			}
		}

		//while (elapsedSeconds - frameQueue_[0]->GetTimestamp() >= replayLength_) {
		//	frameQueue_.pop_front();
		//}

		av_packet_unref(pkt);
	}

	return true;
}
