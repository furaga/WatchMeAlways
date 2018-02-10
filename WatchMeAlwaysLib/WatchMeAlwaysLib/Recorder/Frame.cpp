#include "stdafx.h"

#include "Frame.h"

Frame::Frame() : dataSize(0) {
}

void Frame::SetData(void* src, int byteSize) {
	dataSize = byteSize;
	data.resize(byteSize);
	std::memcpy(&data[0], src, byteSize);
}
