#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../Capture/DesktopCapture.h"  
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
			std::unique_ptr<DesktopCapture> capture(new DesktopCapture());

			int monitorCount = capture->GetMonitorCount();
			Assert::IsTrue(monitorCount >= 1);
			Assert::IsTrue(monitorCount == 2);

			auto monitor = capture->GetMonitor(0);
			int key = capture->CaptureDesktopImage(monitor.GetCaptureRect());
			Assert::IsTrue(monitor.GetCaptureRect().Left > 0);
			Assert::IsTrue(monitor.GetCaptureRect().Top > 0);
			Assert::IsTrue(monitor.GetCaptureRect().Width > 0);
			Assert::IsTrue(monitor.GetCaptureRect().Height > 0);

			auto capturedImage = capture->GetCapturedImage(key);
			Assert::IsTrue(capturedImage != nullptr);
			Assert::IsTrue(capturedImage->GetWidth() > 0);
			Assert::IsTrue(capturedImage->GetHeight() > 0);
			Assert::IsTrue(capturedImage->GetPixels() != nullptr);

			capturedImage->Unregister();
		}
	};
}
#endif