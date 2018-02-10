#include "stdafx.h"  
#include <memory>
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
			bool succeeded = recorder->StartRecording(
				RecordingParameters(4, 4, 120, 30, RECORDING_QUALITY_SUPERFAST) // TODO
			);
			Assert::IsTrue(succeeded);

			uint8_t pixels[4 * 4 * 3];
			for (int i = 0; i < sizeof(pixels) / sizeof(uint8_t); i++) {
				pixels[i] = 100;
			}

			succeeded = recorder->AddFrame(nullptr, 4, 4, 0.1f);
			Assert::IsFalse(succeeded);

			succeeded = recorder->AddFrame(pixels, 0, 4, 0.1f);
			Assert::IsFalse(succeeded);

			succeeded = recorder->AddFrame(pixels, 4, 0, 0.1f);
			Assert::IsFalse(succeeded);

			for (int i = 0; i < 100; i++) {
				succeeded = recorder->AddFrame(pixels, 4, 4, 0.1f);
				Assert::IsTrue(succeeded);
			}

			succeeded = recorder->FinishRecording("test_output.h264");
			Assert::IsTrue(succeeded);

			std::remove("test_output.h264");
		}
	};


}
