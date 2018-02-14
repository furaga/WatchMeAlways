#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../../Capture/DesktopCapture.h"  
#include "../../Unity/UnityDebugCpp.h"  

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
			int w, h;

			int key = capture->CaptureDesktopImage(w, h);
			Assert::IsTrue(w > 0);
			Assert::IsTrue(h > 0);

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