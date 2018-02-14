#pragma once
#ifndef DESKTOP_CAPTURE_H

#include <functional>
#include <unordered_map>

class CapturedImage {
	int width_;
	int height_;
	std::unique_ptr<uint8_t[]> pixels_;
	std::function<void(void)> unregisterFn;

public:
	const int GetWidth() const {
		return width_;
	}
	const int GetHeight() const {
		return height_;
	}
	const uint8_t* GetPixels() const {
		return pixels_.get();
	}
	CapturedImage(uint8_t* pixels, int width, int height) {
		this->pixels_ = std::unique_ptr<uint8_t[]>(pixels);
		this->width_ = width;
		this->height_ = height;
	}
	void Unregister() {
		if (unregisterFn != nullptr) {
			unregisterFn();
		}
	}
	void SetUnregisterFunction(std::function<void(void)> fn) {
		this->unregisterFn = fn;
	}
};

class DesktopCapture {
private:
	std::unordered_map<int, std::unique_ptr<CapturedImage> > capturedImageMap_;

private:
	void unregisterCapturedImage(int key);
	int registerCapturedImage(std::unique_ptr<CapturedImage>&& capturedImage);

public:
	DesktopCapture() {}
	~DesktopCapture() {}
	int CaptureDesktopImage(int& o_width, int& o_height);
	CapturedImage* GetCapturedImage(int key) {
		auto iter = capturedImageMap_.find(key);
		if (iter == capturedImageMap_.end()) {
			return nullptr;
		}
		return (*iter).second.get();
	}
};


#endif