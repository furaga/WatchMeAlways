#include "stdafx.h"

#include "DesktopCapture.h"
#include <windows.h>
#include <mutex>

static std::mutex mutexRegistering_;
static int capturedImageMapCounter_ = 1;
static std::unordered_map<int, std::unique_ptr<CapturedImage> > capturedImageMap_;

BOOL CALLBACK onEnumMonitor(HMONITOR hMonitor, HDC, LPRECT monitorRect, LPARAM param) {

	CaptureRect rect(
		monitorRect->left,
		monitorRect->top,
		monitorRect->right - monitorRect->left,
		monitorRect->bottom - monitorRect->top
	);

	// https://msdn.microsoft.com/ja-jp/library/cc428707.aspx
	// https://msdn.microsoft.com/ja-jp/library/windows/desktop/dd145065(v=vs.85).aspx
	MONITORINFOEX monitorInfoEx;
	monitorInfoEx.cbSize = sizeof(MONITORINFOEX);
	BOOL result = GetMonitorInfo(hMonitor, &monitorInfoEx);
	bool isPrimary = result && monitorInfoEx.dwFlags == MONITORINFOF_PRIMARY;

	std::vector<DesktopCapture::Monitor>* monitors = (std::vector<DesktopCapture::Monitor>*)param;
	monitors->push_back(DesktopCapture::Monitor(rect, isPrimary));

	return TRUE;
}

DesktopCapture::DesktopCapture()
{
	// http://yamatyuu.net/computer/program/sdk/base/enumdisplay/index.html
	EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)onEnumMonitor, (LPARAM)&monitors_);
}

void CapturedImage::Unregister() {
	std::lock_guard<std::mutex> lock(mutexRegistering_);
	int key = key_;
	capturedImageMap_.erase(this->key_);
}

int DesktopCapture::registerCapturedImage(std::unique_ptr<CapturedImage>&& capturedImage)
{
	std::lock_guard<std::mutex> lock(mutexRegistering_);
	int key = capturedImageMapCounter_;
	capturedImage->key_ = key;
	capturedImageMap_[key] = std::move(capturedImage);
	capturedImageMapCounter_++;
	return key;
}

int DesktopCapture::CaptureDesktopImage(const CaptureRect& capRect)
{
	// capture image
	int w = capRect.Width;
	int h = capRect.Height;
	RECT rect;
	rect.left = capRect.Left;
	rect.top = capRect.Top;
	rect.right = capRect.Left + capRect.Width;
	rect.bottom = capRect.Top + capRect.Height;

	auto hdc = GetDC(NULL);
	HDC hmdc = CreateCompatibleDC(hdc);
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, capRect.Left, capRect.Top);

	HBITMAP hbmpOld = (HBITMAP)SelectObject(hmdc, hbmp);
	BitBlt(hmdc, 0, 0, w, h, hdc, 0, 0, SRCCOPY);
	SelectObject(hmdc, hbmpOld);

	UINT sizeOfLine = w * 3;
	sizeOfLine += (sizeOfLine % 4 != 0 ? 4 - sizeOfLine % 4 : 0);
	sizeOfLine *= h;

	auto capturedImage = std::unique_ptr<CapturedImage>(new CapturedImage(new uint8_t[sizeOfLine], w, h));

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof bi);
	bi.bmiHeader.biSize = sizeof bi.bmiHeader;
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = sizeOfLine * h;

	uint8_t* pixels = const_cast<uint8_t*>(capturedImage->GetPixels());
	int res = GetDIBits(hmdc, hbmp, 0, h, pixels, &bi, DIB_RGB_COLORS);

	DeleteObject(hbmp);
	DeleteDC(hmdc);

	int key = registerCapturedImage(std::move(capturedImage));
	return key;
}

CapturedImage* DesktopCapture::GetCapturedImage(int key) const {
	auto iter = capturedImageMap_.find(key);
	if (iter == capturedImageMap_.end()) {
		return nullptr;
	}
	return (*iter).second.get();
}