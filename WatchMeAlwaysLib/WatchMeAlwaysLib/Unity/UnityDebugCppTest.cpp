#include "stdafx.h"  
#include "CppUnitTest.h"  
#include "UnityDebugCpp.h"  
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
namespace MyTest
{
	TEST_CLASS(MyTests)
	{
	public:
		TEST_METHOD(MyTestMethod)
		{
			Assert::AreEqual(6, 6);
		}
	};
}
