#include "stdafx.h"

#include "DesktopCapturer.h"
#include <windows.h>
#include <mutex>

static std::mutex mutexRegistering_;
static int capturedFrameMapCounter_ = 1;
static std::unordered_map<int, std::unique_ptr<CapturedFrame> > capturedFrameMap_;

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

	std::vector<DesktopCapturer::Monitor>* monitors = (std::vector<DesktopCapturer::Monitor>*)param;
	monitors->push_back(DesktopCapturer::Monitor(rect, isPrimary));

	return TRUE;
}

DesktopCapturer::DesktopCapturer()
{
	// http://yamatyuu.net/computer/program/sdk/base/enumdisplay/index.html
	EnumDisplayMonitors(NULL, NULL, (MONITORENUMPROC)onEnumMonitor, (LPARAM)&monitors_);
}

void CapturedFrame::Unregister() {
	std::lock_guard<std::mutex> lock(mutexRegistering_);
	int key = key_;
	capturedFrameMap_.erase(this->key_);
}

int DesktopCapturer::registerCapturedFrame(std::unique_ptr<CapturedFrame>&& capturedFrame)
{
	std::lock_guard<std::mutex> lock(mutexRegistering_);
	int key = capturedFrameMapCounter_;
	capturedFrame->key_ = key;
	capturedFrameMap_[key] = std::move(capturedFrame);
	capturedFrameMapCounter_++;

	static int cnt = 0;
	if ((cnt++) % 100 == 0) {
		printf("capturedFrameMap_.size() = %zd\n", capturedFrameMap_.size());
	}

	return key;
}

int DesktopCapturer::CaptureDesktopFrame(const CaptureRect& capRect)
{
	// capture frame
	int x = capRect.Left;
	int y = capRect.Top;
	int w = capRect.Width;
	int h = capRect.Height;
	RECT rect;
	rect.left = x;
	rect.top = y;
	rect.right = x + w;
	rect.bottom = y + h;

	auto hdc = GetDC(NULL);
	if (!hdc) {
		return 0;
	}
	HDC hmdc = CreateCompatibleDC(hdc);
	if (!hmdc) {
		return 0;
	}
	HBITMAP hbmp = CreateCompatibleBitmap(hdc, w, h);
	if (!hbmp) {
		return 0;
	}

	HBITMAP hbmpOld = (HBITMAP)SelectObject(hmdc, hbmp);
	BitBlt(hmdc, 0, 0, w, h, hdc, x, y, SRCCOPY);
	SelectObject(hmdc, hbmpOld);

	UINT sizeOfLine = w * 3;
	sizeOfLine += (sizeOfLine % 4 != 0 ? 4 - sizeOfLine % 4 : 0);
	sizeOfLine *= h;

	auto capturedFrame = std::unique_ptr<CapturedFrame>(new CapturedFrame(new uint8_t[sizeOfLine], w, h));

	BITMAPINFO bi;
	ZeroMemory(&bi, sizeof bi);
	bi.bmiHeader.biSize = sizeof bi.bmiHeader;
	bi.bmiHeader.biWidth = w;
	bi.bmiHeader.biHeight = h;
	bi.bmiHeader.biPlanes = 1;
	bi.bmiHeader.biBitCount = 24;
	bi.bmiHeader.biCompression = BI_RGB;
	bi.bmiHeader.biSizeImage = sizeOfLine * h;

	uint8_t* pixels = const_cast<uint8_t*>(capturedFrame->GetPixels());
	int res = GetDIBits(hmdc, hbmp, 0, h, pixels, &bi, DIB_RGB_COLORS);

	DeleteObject(hbmp);
	DeleteDC(hmdc);
	ReleaseDC(NULL, hdc);

	int key = registerCapturedFrame(std::move(capturedFrame));
	return key;
}

CapturedFrame* DesktopCapturer::GetCapturedFrame(int key) const {
	auto iter = capturedFrameMap_.find(key);
	if (iter == capturedFrameMap_.end()) {
		return nullptr;
	}
	return (*iter).second.get();
}