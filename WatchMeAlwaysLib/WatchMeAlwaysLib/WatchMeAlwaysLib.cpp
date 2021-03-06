#include "stdafx.h"

#include "Encode/Encoder.h"
#include "Capture/DesktopCapturer.h"

extern "C" {
	class Rect {
	public:
		int Left;
		int Top;
		int Width;
		int Height;
	};

	class Monitor {
	public:
		int Left;
		int Top;
		int Width;
		int Height;
		bool IsPrimary;
	};

	class Frame {
	public:
		int Width;
		int Height;
		int Data;
	};

	DllExport int SetLogPath(char* filepath);
	DllExport int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality);
	// DllExport int RecordFrame(uint8_t* pixels, int width, int height, float timeStamp);
	DllExport int FinishRecording(char* saveFilePath);
	DllExport int GetMonitorCount();
	DllExport int GetMonitor(int n, Monitor* monitor);
	DllExport int CaptureDesktopFrame(Rect* rect, Frame* frame);
	DllExport int EncodeDesktopFrame(int key, float elapsedSeconds);
}

enum APIResult {
	API_RESULT_OK = 0,
	API_RESULT_NG = 1,
	API_RESULT_FATAL = 2,
};

std::unique_ptr<DesktopCapturer> capture = nullptr;
std::unique_ptr<Encoder> encoder = nullptr;

int SetLogPath(char* filepath)
{
	std::string filename(filepath);
	Debug::SetLogPath(filename);
	return API_RESULT_OK;
}
int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality)
{
	if (encoder == nullptr) {
		encoder.reset(new Encoder());
	}
	auto params = RecordingParameters(width, height, maxSeconds, fps, quality);
	bool succeeded = encoder->StartEncoding(params);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->StartRecording()");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int RecordFrame(uint8_t* pixels, int width, int height, float timeStamp)
{
	if (encoder == nullptr) {
		UnityDebugCpp::Error("recorder is not initialized");
		return API_RESULT_NG;
	}
	bool succeeded = encoder->EncodeFrame(pixels, width, height, timeStamp);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->EncodeFrame()");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int FinishRecording(char* saveFilePath)
{
	if (encoder == nullptr) {
		UnityDebugCpp::Error("recorder is not initialized");
		return API_RESULT_NG;
	}
	std::string filename(saveFilePath);
	bool succeeded = encoder->FinishEncoding(filename);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->FinishRecording()");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int GetMonitorCount() {
	if (capture == nullptr) {
		capture.reset(new DesktopCapturer());
	}
	return capture->GetMonitorCount();
}

int GetMonitor(int n, Monitor* outMonitor) {
	if (capture == nullptr) {
		capture.reset(new DesktopCapturer());
	}
	if (outMonitor == nullptr) {
		UnityDebugCpp::Error("outMonitor is null");
		return API_RESULT_NG;
	}
	auto monitor = capture->GetMonitor(n);
	outMonitor->Left = monitor.GetCaptureRect().Left;
	outMonitor->Top = monitor.GetCaptureRect().Top;
	outMonitor->Width = monitor.GetCaptureRect().Width;
	outMonitor->Height = monitor.GetCaptureRect().Height;
	outMonitor->IsPrimary = monitor.IsPrimary();
	return API_RESULT_OK;
}

int CaptureDesktopFrame(Rect* rect, Frame* frame)
{
	if (capture == nullptr) {
		capture.reset(new DesktopCapturer());
	}
	if (rect == nullptr) {
		UnityDebugCpp::Error("rect is null");
		return API_RESULT_NG;
	}
	int data = capture->CaptureDesktopFrame(
		CaptureRect(
			rect->Left,
			rect->Top,
			rect->Width,
			rect->Height
		)
	);
	if (data == 0) {
		UnityDebugCpp::Error("CaptureDesktopFrame failed");
		return API_RESULT_FATAL;
	}
	frame->Data = data;
	return API_RESULT_OK;
}

int EncodeDesktopFrame(int key, float elapsedSeconds)
{
	if (capture == nullptr) {
		UnityDebugCpp::Error("capture is not initialized");
		return API_RESULT_NG;
	}

	if (encoder == nullptr) {
		UnityDebugCpp::Error("recorder is not initialized");
		return API_RESULT_NG;
	}

	auto capturedFrame = capture->GetCapturedFrame(key);
	if (!capturedFrame) {
		UnityDebugCpp::Error("failed to get capturedFrame");
		return API_RESULT_NG;
	}

	bool succeeded = encoder->EncodeFrame(capturedFrame->GetPixels(), capturedFrame->GetWidth(), capturedFrame->GetHeight(), elapsedSeconds);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->EncodeFrame()");
		return API_RESULT_NG;
	}

	capturedFrame->Unregister();

	return API_RESULT_OK;
}