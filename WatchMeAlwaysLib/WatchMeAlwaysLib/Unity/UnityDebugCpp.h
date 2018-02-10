// cf) https://stackoverflow.com/questions/43732825/use-debug-log-from-c

#pragma once
#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>

#define DLLExport __declspec(dllexport)

extern "C"
{
	typedef void(*UnityPrintLogFn)(const char* message, int color, int size);
	DLLExport void RegisterUnityPrintLogFn(UnityPrintLogFn fn);
}

//Color Enum
enum class Color { Red, Green, Blue, Black, White, Yellow, Orange };

class UnityDebugCpp
{
public:
	static void Log(const char* message, Color color = Color::Black);
	static void Log(const std::string message, Color color = Color::Black);
	static void Log(const int message, Color color = Color::Black);
	static void Log(const char message, Color color = Color::Black);
	static void Log(const float message, Color color = Color::Black);
	static void Log(const double message, Color color = Color::Black);
	static void Log(const bool message, Color color = Color::Black);

	static void Info(const std::string message) {
		Log(message, Color::Black);
	}
	static void Warn(const std::string message) {
		Log(message, Color::Yellow);
	}
	static void Error(const std::string message) {
		Log(message, Color::Red);
	}

	template <typename ... Args>
	static void Info(const char* format, Args const & ... args) {
		char str[512];
		sprintf_s(str, format, args ...);
		Log(str, Color::Red);
	}

	template <typename ... Args>
	static void Warn(const char* format, Args const & ... args) {
		char str[512];
		sprintf_s(str, format, args ...);
		Log(str, Color::Red);
	}

	template <typename ... Args>
	static void Error(const char* format, Args const & ... args) {
		char str[512];
		sprintf_s(str, format, args ...);
		Log(str, Color::Red);
	}

private:
	static void printLog(const std::stringstream &ss, const Color &color);
};

