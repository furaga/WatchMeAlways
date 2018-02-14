#include "stdafx.h"

#include "Recorder/Recorder.h"
#include "Capture/DesktopCapture.h"

extern "C" {
	DllExport int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality);
	DllExport int AddFrame(uint8_t* pixels, int width, int height, float timeStamp);
	DllExport int FinishRecording(char* saveFilePath);

	class Frame {
	public:
		int Width;
		int Height;
		int Data;
	};
	DllExport int CaptureDesktopImage(Frame* frame);
	DllExport int AddCapturedDesktopFrame(int key, float timeStamp);
}

enum APIResult {
	API_RESULT_OK = 0,
	API_RESULT_NG = 1,
};

std::unique_ptr<DesktopCapture> capture = nullptr;
std::unique_ptr<Recorder> recorder = nullptr;

int StartRecording(int width, int height, float maxSeconds, float fps, RecordingQuality quality)
{
	if (recorder == nullptr) {
		recorder.reset(new Recorder());
	}
	auto params = RecordingParameters(width, height, maxSeconds, fps, quality);
	bool succeeded = recorder->StartRecording(params);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->StartRecording()\n");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int AddFrame(uint8_t* pixels, int width, int height, float timeStamp)
{
	if (recorder == nullptr) {
		return API_RESULT_NG;
	}
	bool succeeded = recorder->AddFrame(pixels, width, height, timeStamp);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->AddFrame()\n");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int FinishRecording(char* saveFilePath)
{
	if (recorder == nullptr) {
		return API_RESULT_NG;
	}
	std::string filename(saveFilePath);
	UnityDebugCpp::Info(filename.c_str());
	bool succeeded = recorder->FinishRecording(filename);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->FinishRecording()\n");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int CaptureDesktopImage(Frame* frame)
{
	if (capture == nullptr) {
		capture.reset(new DesktopCapture());
	}
	frame->Data = capture->CaptureDesktopImage(frame->Width, frame->Height);
	return API_RESULT_OK;
}

int AddCapturedDesktopFrame(int key, float timeStamp)
{
	if (capture == nullptr) {
		return API_RESULT_NG;
	}

	if (recorder == nullptr) {
		return API_RESULT_NG;
	}

	auto capturedImage = capture->GetCapturedImage(key);
	if (!capturedImage) {
		UnityDebugCpp::Error("failed to get capturedImage\n");
		return API_RESULT_NG;
	}

	bool succeeded = recorder->AddFrame(capturedImage->GetPixels(), capturedImage->GetWidth(), capturedImage->GetHeight(), timeStamp);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->AddFrame()\n");
		return API_RESULT_NG;
	}

	capturedImage->Unregister();

	return API_RESULT_OK;
}