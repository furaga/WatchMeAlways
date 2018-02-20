#include "stdafx.h"

#include "Frame.h"

Frame::Frame() : dataSize_(0), timestamp_(0) {
}

Frame::Frame(void* src, int byteSize, float timestamp) : dataSize_(byteSize), timestamp_(timestamp) {
	SetData(src, byteSize, timestamp);
}

void Frame::SetData(void* src, int byteSize, float timestamp) {
	timestamp_ = timestamp;
	dataSize_ = byteSize;
	data_.resize(byteSize);
	std::memcpy(&data_[0], src, byteSize);
}
