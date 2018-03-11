#pragma once

#include <stdio.h>
#include <memory>
#include <mutex>

class Debug {
public:
	static std::mutex mutexDebugLog_;
	static std::string logFilePath_;

	static void SetLogPath(const std::string& filepath) {
		logFilePath_ = filepath;
	}

	template <typename ... Args>
	static void Printf(const char* format, Args const & ... args) {
		std::lock_guard<std::mutex> lock(mutexDebugLog_);
		FILE* f;
		auto err = fopen_s(&f, logFilePath_.c_str(), "a");
		if (err) {
			f = nullptr;
			return;
		}

		time_t t = time(NULL);
		fprintf_s(f, "[%lld]", t);
		fprintf_s(f, format, args ...);
		fclose(f);
	}

	template <typename ... Args>
	static void Println(const char* format, Args const & ... args)
	{
		std::lock_guard<std::mutex> lock(mutexDebugLog_);
		FILE* f;
		auto err = fopen_s(&f, logFilePath_.c_str(), "a");
		if (err) {
			f = nullptr;
			return;
		}
		time_t t = time(NULL);
		fprintf_s(f, "[%lld]", t);
		fprintf_s(f, format, args ...);
		fprintf_s(f, "\n");
		fclose(f);
	}
};

