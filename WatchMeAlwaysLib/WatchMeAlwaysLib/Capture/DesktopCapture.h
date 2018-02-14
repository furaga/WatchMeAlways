#pragma once
#ifndef DESKTOP_CAPTURE_H

#include <functional>
#include <unordered_map>

class DesktopCapture;

class CapturedImage {
	friend class DesktopCapture;
	int width_;
	int height_;
	std::unique_ptr<uint8_t[]> pixels_;
	int key_;
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
	void Unregister();
	CapturedImage(uint8_t* pixels, int width, int height)
		: width_(width),
		height_(height)
	{
		pixels_ = std::unique_ptr<uint8_t[]>(pixels);
	}
};

class DesktopCapture {
	int registerCapturedImage(std::unique_ptr<CapturedImage>&& capturedImage);

public:
	DesktopCapture() {}
	~DesktopCapture() {}
	int CaptureDesktopImage(int& o_width, int& o_height);
	CapturedImage* GetCapturedImage(int key);
};


#endif