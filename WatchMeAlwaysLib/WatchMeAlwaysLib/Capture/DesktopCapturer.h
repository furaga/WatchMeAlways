#pragma once
#ifndef DESKTOP_CAPTURE_H

#include <functional>
#include <unordered_map>

class DesktopCapturer;

class CapturedFrame {
	friend class DesktopCapturer;
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
	CapturedFrame(uint8_t* pixels, int width, int height)
		: width_(width),
		height_(height)
	{
		pixels_ = std::unique_ptr<uint8_t[]>(pixels);
	}
};

struct CaptureRect {
	int Left;
	int Top;
	int Width;
	int Height;
	CaptureRect(int left, int top, int width, int height) :
		Left(left),
		Top(top),
		Width(width),
		Height(height) { }
};

class DesktopCapturer {
public:
	class Monitor {
		CaptureRect rect_;
		bool isPrimary_;
	public:
		Monitor(const CaptureRect& rect, bool isPrimary) : rect_(rect), isPrimary_(isPrimary) {}
		const CaptureRect GetCaptureRect() const { return rect_; }
		bool IsPrimary() const { return isPrimary_; }
	};

private:
	int registerCapturedFrame(std::unique_ptr<CapturedFrame>&& capturedFrame);
	std::vector<Monitor> monitors_;

public:
	DesktopCapturer();
	~DesktopCapturer() {}
	int CaptureDesktopFrame(const CaptureRect& rect);
	CapturedFrame* GetCapturedFrame(int key) const;
	int GetMonitorCount() const { return (int)monitors_.size(); }
	const Monitor GetMonitor(int n) const {
		if (0 <= n && n < monitors_.size()) {
			return monitors_[n];
		}
		return Monitor(CaptureRect(0, 0, 0, 0), false);
	}
};


#endif