#include "stdafx.h"  

#ifdef UNIT_TEST
#include "CppUnitTest.h"  
#include "../Unity/UnityDebugCpp.h"  
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace UnityDebugCppTest
{
	std::string latestMessage;
	void mockUnityPrintLog(const char* message, int color, int size) {
		latestMessage = message;
	}

	TEST_CLASS(UnityDebugCppTest)
	{
	public:
		TEST_METHOD(UnityDebugCppTest_0)
		{
			RegisterUnityPrintLogFn(mockUnityPrintLog);

			UnityDebugCpp::Info("INFO");
			Assert::AreEqual(latestMessage, std::string("INFO"));
			UnityDebugCpp::Warn("WARN");
			Assert::AreEqual(latestMessage, std::string("WARN"));
			UnityDebugCpp::Error("ERROR");
			Assert::AreEqual(latestMessage, std::string("ERROR"));

			UnityDebugCpp::Error("INFO %d", 1);
			Assert::AreEqual(latestMessage, std::string("INFO 1"));
			UnityDebugCpp::Error("WARN %d", 2);
			Assert::AreEqual(latestMessage, std::string("WARN 2"));
			UnityDebugCpp::Error("ERROR %d", 3);
			Assert::AreEqual(latestMessage, std::string("ERROR 3"));
		}
	};
}
#endif