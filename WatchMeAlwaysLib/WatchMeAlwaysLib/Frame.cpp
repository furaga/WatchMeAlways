#include "stdafx.h"

#include <memory>
#include "Frame.h"

Frame::Frame(void* src, int byteSize) : dataSize(byteSize) {
	data = new uint8_t[byteSize];
	std::memcpy(data, src, byteSize);
}
