#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../../Recorder/Recorder.h"  
#include "../../Unity/UnityDebugCpp.h"  

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RecorderTest
{
	std::string latestMessage;
	void mockUnityPrintLog(const char* message, int color, int size) {
		latestMessage = message;
	}

	TEST_CLASS(RecorderTest)
	{
	public:
		TEST_CLASS_INITIALIZE(RecorderTest_Init) {
			RegisterUnityPrintLogFn(mockUnityPrintLog);
		}

		TEST_METHOD(RecorderTest_0)
		{
			std::unique_ptr<Recorder> recorder(new Recorder());

			// start recording
			auto params = RecordingParameters(4, 4, 10, 9, RECORDING_QUALITY_SUPERFAST);
			bool succeeded = recorder->StartRecording(params);
			Assert::IsTrue(succeeded);

			// create frame RGB data
			uint8_t pixels[4 * 4 * 3];
			for (int i = 0; i < sizeof(pixels) / sizeof(uint8_t); i++) {
				pixels[i] = 100;
			}

			// add frames with invalid parameters
			succeeded = recorder->AddFrame(nullptr, 4, 4, 0.1f);
			Assert::IsFalse(succeeded);
			succeeded = recorder->AddFrame(pixels, 0, 4, 0.1f);
			Assert::IsFalse(succeeded);
			succeeded = recorder->AddFrame(pixels, 4, 0, 0.1f);
			Assert::IsFalse(succeeded);

			// add frames with valid parameters
			for (int i = 0; i < 100; i++) {
				succeeded = recorder->AddFrame(pixels, 4, 4, 0.1f);
				Assert::IsTrue(succeeded);
			}

			// finish recording 
			succeeded = recorder->FinishRecording("test_output.h264");
			Assert::IsTrue(succeeded);

			// start recording again
			succeeded = recorder->StartRecording(params);
			Assert::IsTrue(succeeded);

			// finish recording with 0 frame
			succeeded = recorder->FinishRecording("test_output.h264");
			Assert::IsTrue(succeeded);

			std::remove("test_output.h264");
		}
	};
}
#endif