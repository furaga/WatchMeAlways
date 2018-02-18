#include "stdafx.h"

#include "Frame.h"

Frame::Frame() : dataSize_(0), timestamp_(0) {
}

void Frame::SetData(void* src, int byteSize, int timestamp_) {
	dataSize_ = byteSize;
	data_.resize(byteSize);
	std::memcpy(&data_[0], src, byteSize);
}
