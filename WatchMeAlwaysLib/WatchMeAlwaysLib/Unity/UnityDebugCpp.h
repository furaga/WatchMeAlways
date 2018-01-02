// cf) https://stackoverflow.com/questions/43732825/use-debug-log-from-c

#pragma once
#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>

#define DLLExport __declspec(dllexport)

extern "C"
{
	//Create a callback delegate
	typedef void(*FuncCallBack)(const char* message, int color, int size);
	static FuncCallBack callbackInstance = nullptr;
	DLLExport void RegisterUnityDebugCppCallback(FuncCallBack cb);
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

private:
	static void send_log(const std::stringstream &ss, const Color &color);
};

