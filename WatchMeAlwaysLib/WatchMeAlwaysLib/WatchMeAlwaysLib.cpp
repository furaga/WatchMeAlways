#include "stdafx.h"

#include <string>

#include "Recorder.h"

extern "C" {
	DllExport int StartRecording(int width, int height);
	DllExport int AddFrame(uint8_t* pixels, float timeStamp, int lineSize);
	DllExport int FinishRecording();
}

Recorder* recorder = nullptr;

int StartRecording(int width, int height)
{
	SAFE_DELETE(recorder);
	recorder = new Recorder();
	bool succeeded = recorder->StartRecording();
	if (!succeeded) {
		printf("failed: recorder->StartRecording()\n");
		return 1;
	}
	return 0;
}

int AddFrame(uint8_t* pixels, float timeStamp, int linesize)
{
	if (recorder == nullptr) {
		return 1;
	}
	bool succeeded = recorder->AddFrame(pixels, timeStamp, linesize);
	if (!succeeded) {
		printf("failed: recorder->AddFrame()\n");
		return 1;
	}
	return 0;
}

int FinishRecording()
{
	if (recorder == nullptr) {
		return 1;
	}
	std::string filename = "C:\\Users\\furaga\\Documents\\tmpdata\\sample-unity.h264";
	bool succeeded = recorder->FinishRecording(filename);
	if (!succeeded) {
		printf("failed: recorder->FinishRecording()\n");
		return 1;
	}
	return 0;
}
