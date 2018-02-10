#include "stdafx.h"

#include <string>
#include <memory>

#include "Recorder/Recorder.h"

extern "C" {
	DllExport int StartRecording(int width, int height);
	DllExport int SetReplayLength(int seconds);
	DllExport int SetBitRate(int mbps);
	DllExport int AddFrame(uint8_t* pixels, int width, int height, float timeStamp);
	DllExport int FinishRecording(char* saveFilePath);
}

enum APIResult {
	API_RESULT_OK = 0,
	API_RESULT_NG = 1,
};

std::unique_ptr<Recorder> recorder = nullptr;

int StartRecording(int width, int height)
{
	recorder.reset(new Recorder());
	bool succeeded = recorder->StartRecording(
		RecordingParameters(width, height, 120, 30, RECORDING_QUALITY_SUPERFAST) // TODO
	);
	if (!succeeded) {
		UnityDebugCpp::Error("failed: recorder->StartRecording()\n");
		return API_RESULT_NG;
	}
	return API_RESULT_OK;
}

int SetReplayLength(int seconds) {
	// TODO
	return 0;
}

int SetBitRate(int mbps) {
	// TODO
	return 0;
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
