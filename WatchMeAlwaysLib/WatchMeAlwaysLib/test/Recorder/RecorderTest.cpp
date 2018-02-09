#include "stdafx.h"  
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
		}
	};


}
