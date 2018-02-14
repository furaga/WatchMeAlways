#pragma once
#ifndef DESKTOP_CAPTURE_H

class DesktopCapture {
public:
	DesktopCapture () {}
	~DesktopCapture() {}
	uint8_t* CaptureDesktopImage(int& o_width, int& o_height);
};

#endif