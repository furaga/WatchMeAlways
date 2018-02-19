#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../Encode/Encoder.h"  
#include "../Capture/DesktopCapturer.h"  
#include "../Unity/UnityDebugCpp.h"  
#include <thread>
#include <queue>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace EncoderTest
{
	std::string latestMessage;
	void mockUnityPrintLog(const char* message, int color, int size) {
		latestMessage = message;
	}

	TEST_CLASS(EncoderTest)
	{

	public:
		TEST_CLASS_INITIALIZE(EncoderTest_Init) {
			RegisterUnityPrintLogFn(mockUnityPrintLog);
		}

		TEST_METHOD(EncoderTest_0)
		{
			//
			// start -> finalize -> start -> finalize
			//

			std::unique_ptr<Encoder> Encoder(new Encoder());

			// start recording
			auto params = RecordingParameters(4, 4, 10, 9, RECORDING_QUALITY_SUPERFAST);
			bool succeeded = Encoder->StartEncoding(params);
			Assert::IsTrue(succeeded);

			// create frame RGB data
			uint8_t pixels[4 * 4 * 3];
			for (int i = 0; i < sizeof(pixels) / sizeof(uint8_t); i++) {
				pixels[i] = 100;
			}

			// add frames with invalid parameters
			succeeded = Encoder->EncodeFrame(nullptr, 4, 4, 0.1f);
			Assert::IsFalse(succeeded);
			succeeded = Encoder->EncodeFrame(pixels, 0, 4, 0.1f);
			Assert::IsFalse(succeeded);
			succeeded = Encoder->EncodeFrame(pixels, 4, 0, 0.1f);
			Assert::IsFalse(succeeded);

			// add frames with valid parameters
			for (int i = 0; i < 100; i++) {
				succeeded = Encoder->EncodeFrame(pixels, 4, 4, 0.1f);
				Assert::IsTrue(succeeded);
			}

			// finish recording 
			succeeded = Encoder->FinishEncoding("test_output.h264");
			Assert::IsTrue(succeeded);

			// start recording again
			succeeded = Encoder->StartEncoding(params);
			Assert::IsTrue(succeeded);

			// finish recording with 0 frame
			succeeded = Encoder->FinishEncoding("test_output.h264");
			Assert::IsTrue(succeeded);

			std::remove("test_output.h264");
		}

		TEST_METHOD(EncoderTest_1)
		{
			//
			// finalize -> start -> finalize
			//

			std::unique_ptr<Encoder> Encoder(new Encoder());

			// finish recording 
			bool succeeded = Encoder->FinishEncoding("test_output.h264");
			Assert::IsFalse(succeeded);

			std::remove("test_output.h264");

			// start recording
			auto params = RecordingParameters(4, 4, 10, 9, RECORDING_QUALITY_SUPERFAST);
			succeeded = Encoder->StartEncoding(params);
			Assert::IsTrue(succeeded);

			// create frame RGB data
			uint8_t pixels[4 * 4 * 3];
			for (int i = 0; i < sizeof(pixels) / sizeof(uint8_t); i++) {
				pixels[i] = 100;
			}

			// add frames with valid parameters
			for (int i = 0; i < 100; i++) {
				succeeded = Encoder->EncodeFrame(pixels, 4, 4, 0.1f);
				Assert::IsTrue(succeeded);
			}

			// finish recording 
			succeeded = Encoder->FinishEncoding("test_output.h264");
			Assert::IsTrue(succeeded);
			std::remove("test_output.h264");
		}


		TEST_METHOD(EncoderTest_2)
		{
			//
			// run DesktopCapture and Encoder in parallel
			//

			std::unique_ptr<Encoder> Encoder(new Encoder());
			std::unique_ptr<DesktopCapturer> capture(new DesktopCapturer());

			auto monitor = capture->GetMonitor(0);
			int key = capture->CaptureDesktopImage(monitor.GetCaptureRect());

			auto params = RecordingParameters(monitor.GetCaptureRect().Width, monitor.GetCaptureRect().Height, 1.0f, 10.0f, RECORDING_QUALITY_ULTRAFAST);
			bool succeeded = Encoder->StartEncoding(params);
			Assert::IsTrue(succeeded);

			std::queue<int> q;
			q.push(key);

			// run capturing thread
			std::thread cap([&] {
				for (int i = 0; i < 10; i++) {
					int key = capture->CaptureDesktopImage(monitor.GetCaptureRect());
					q.push(key);
				}
			});

			// run recording thread
			std::thread rec([&] {
				for (int i = 0; i < 10 + 1; i++) {
					while (true) {
						if (q.empty() == false) {
							int key = q.front();
							q.pop();
							auto capturedImage = capture->GetCapturedFrame(key);
							succeeded = Encoder->EncodeFrame(capturedImage->GetPixels(), monitor.GetCaptureRect().Width, monitor.GetCaptureRect().Height, i * 30.0f);
							capturedImage->Unregister();
							break;
						}
					}
				}
			});

			// wait until each thread finishes
			cap.join();
			rec.join();

			// finish recording 
			succeeded = Encoder->FinishEncoding("test_output.h264");
			Assert::IsTrue(succeeded);
			std::remove("test_output.h264");
		}

	};
}
#endif