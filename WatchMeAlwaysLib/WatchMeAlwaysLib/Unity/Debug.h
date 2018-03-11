#pragma once

#include <stdio.h>
#include <memory>
#include <mutex>

class Debug {
public:
	static std::mutex mutexDebugLog_;

	template <typename ... Args>
	static void Printf(const char* format, Args const & ... args) {
		std::lock_guard<std::mutex> lock(mutexDebugLog_);
		FILE* f;
		auto err = fopen_s(&f, "log_server.txt", "a");
		if (err) {
			f = nullptr;
			return;
		}
		fprintf_s(f, format, args ...);
		fclose(f);
	}

	template <typename ... Args>
	static void Println(const char* format, Args const & ... args)
	{
		std::lock_guard<std::mutex> lock(mutexDebugLog_);
		FILE* f;
		auto err = fopen_s(&f, "log_server.txt", "a");
		if (err) {
			f = nullptr;
			return;
		}
		fprintf_s(f, format, args ...);
		fprintf_s(f, "\n");
		fclose(f);
	}
};

