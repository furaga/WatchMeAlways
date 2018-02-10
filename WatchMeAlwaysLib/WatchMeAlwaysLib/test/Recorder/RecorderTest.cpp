#include "stdafx.h"  
#include <memory>
#include "CppUnitTest.h"  
#include "../../Recorder/Recorder.h"  
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace RecorderTest
{
	std::string latestMessage;
	void mockCallbackInstance(const char* message, int color, int size) {
		latestMessage = message;
	}

	TEST_CLASS(RecorderTest)
	{
	public:
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

			for (int i = 0; i < 100; i++) {
				succeeded = recorder->AddFrame(
					pixels,
					0.1f,
					4, 4
				);
				Assert::IsTrue(succeeded);
			}

			succeeded = recorder->FinishRecording("test_output.h264");
			Assert::IsTrue(succeeded);

			std::remove("test_output.h264");
		}
	};


}
