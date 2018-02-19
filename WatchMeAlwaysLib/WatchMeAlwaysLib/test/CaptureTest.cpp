#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../Capture/DesktopCapturer.h"  
#include "../Unity/UnityDebugCpp.h"  

#include <memory>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace CaptureTest
{
	std::string latestMessage;
	void mockUnityPrintLog(const char* message, int color, int size) {
		latestMessage = message;
	}

	TEST_CLASS(CaptureTest)
	{
	public:
		TEST_CLASS_INITIALIZE(CaptureTest_Init) {
			RegisterUnityPrintLogFn(mockUnityPrintLog);
		}

		TEST_METHOD(CaptureTest_0)
		{
			std::unique_ptr<DesktopCapturer> capture(new DesktopCapturer());

			int monitorCount = capture->GetMonitorCount();
			Assert::IsTrue(monitorCount >= 1);

			auto monitor = capture->GetMonitor(0);
			int key = capture->CaptureDesktopFrame(monitor.GetCaptureRect());
			Assert::IsTrue(monitor.GetCaptureRect().Left >= 0);
			Assert::IsTrue(monitor.GetCaptureRect().Top >= 0);
			Assert::IsTrue(monitor.GetCaptureRect().Width > 0);
			Assert::IsTrue(monitor.GetCaptureRect().Height > 0);

			auto capturedFrame = capture->GetCapturedFrame(key);
			Assert::IsTrue(capturedFrame != nullptr);
			Assert::IsTrue(capturedFrame->GetWidth() > 0);
			Assert::IsTrue(capturedFrame->GetHeight() > 0);
			Assert::IsTrue(capturedFrame->GetPixels() != nullptr);

			capturedFrame->Unregister();
		}
		TEST_METHOD(CaptureTest_1) {
			std::unique_ptr<DesktopCapturer> capture(new DesktopCapturer());

			// OK cases
			int monitorCount = capture->GetMonitorCount();
			for (int i = 0; i < monitorCount; i++) {
				auto monitor = capture->GetMonitor(i);
				Assert::IsTrue(monitor.GetCaptureRect().Left >= 0);
				Assert::IsTrue(monitor.GetCaptureRect().Top >= 0);
				Assert::IsTrue(monitor.GetCaptureRect().Width > 0);
				Assert::IsTrue(monitor.GetCaptureRect().Height > 0);
			}

			// NG case
			auto monitor = capture->GetMonitor(-1);
			Assert::IsTrue(monitor.GetCaptureRect().Width <= 0);
			Assert::IsTrue(monitor.GetCaptureRect().Height <= 0);
			monitor = capture->GetMonitor(monitorCount);
			Assert::IsTrue(monitor.GetCaptureRect().Width <= 0);
			Assert::IsTrue(monitor.GetCaptureRect().Height <= 0);
		}
	};
}
#endif