#pragma once

#include <stdio.h>
#include <memory>

class Debug {
public:
	typedef std::unique_ptr<FILE, void(*)(FILE*)> FilePtr;
	static FilePtr file;

	static void Initialize() {
		Dispose();
		FILE* f;
		auto err = fopen_s(&f, "output.txt", "w");
		file.reset(f);
		if (err) {
			file = nullptr;
		}
	}
	static void Dispose() {
		if (file != nullptr) {
			fclose(file.get());
		}
	}
	
	template <typename ... Args>
	static void Printf(const char* format, Args const & ... args) {
		if (file == nullptr) {
			Initialize();
		}
		fprintf_s(file.get(), format, args ...);
	}

	template <typename ... Args>
	static void Println(const char* format, Args const & ... args) {
		if (file == nullptr) {
			Initialize();
		}
		Printf(format, args ...);
		fprintf_s(file.get(), "\n");
	}
};

