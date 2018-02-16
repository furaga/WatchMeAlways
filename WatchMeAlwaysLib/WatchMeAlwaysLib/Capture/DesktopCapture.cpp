#include "stdafx.h"

#include "DesktopCapture.h"
#include <windows.h>

static int capturedImageMapCounter_ = 1;
std::unordered_map<int, std::unique_ptr<CapturedImage> > capturedImageMap_;

void CapturedImage::Unregister() {
	int key = key_;
	Debug::Println("unregister s: key = %d", key);
	capturedImageMap_.erase(this->key_);
	Debug::Println("unregister e: key = %d", key);
}

int DesktopCapture::registerCapturedImage(std::unique_ptr<CapturedImage>&& capturedImage)
{
	int key = capturedImageMapCounter_;
	Debug::Println("register s: key = %d", key);
	capturedImage->key_ = key;
	capturedImageMap_[key] = std::move(capturedImage);
	capturedImageMapCounter_++;
	Debug::Println("register e: key = %d", key);
	return key;
}

int DesktopCapture::CaptureDesktopImage(int& o_width, int& o_height)
{
	auto hwnd = GetDesktopWindow();
	auto hdc = GetDC(NULL);

	// capture image
	RECT rect;
	GetWindowRect(hwnd, &rect);
	UINT width = rect.right - rect.left;
	UINT height = rect.bottom - rect.top;

	HDC hmdc = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, width, height);

	HBITMAP hbmpOld = (HBITMAP)SelectObject(hmdc, hbmp);
	BitBlt(hmdc, 0, 0, width, height, hdc, 0, 0, SRCCOPY);
	SelectObject(hmdc, hbmpOld);

	UINT sizeOfLine = width * 3;
	sizeOfLine += (sizeOfLine % 4 != 0 ? 4 - sizeOfLine % 4 : 0);
	sizeOfLine *= height;

	auto capturedImage = std::unique_ptr<CapturedImage>(new CapturedImage(new uint8_t[sizeOfLine], width, height));

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof bi);
	bi.bmiHeader.biSize = sizeof bi.bmiHeader;
	bi.bmiHeader.biWidth = width;
	bi.bmiHeader.biHeight = height;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = sizeOfLine * height;

	uint8_t* pixels = const_cast<uint8_t*>(capturedImage->GetPixels());
	int res = GetDIBits(hmdc, hbmp, 0, height, pixels, &bi, DIB_RGB_COLORS);

	DeleteObject(hbmp);
	DeleteDC(hmdc);

	int key = registerCapturedImage(std::move(capturedImage));

	o_width = width;
	o_height = height;
	return key;
}

CapturedImage* DesktopCapture::GetCapturedImage(int key) {
	auto iter = capturedImageMap_.find(key);
	if (iter == capturedImageMap_.end()) {
		return nullptr;
	}
	return (*iter).second.get();
}