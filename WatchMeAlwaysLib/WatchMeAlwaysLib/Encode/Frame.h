#pragma once
#ifndef FRAME_H

#include <vector>

class Frame {
	std::vector<uint8_t> data_;
	int dataSize_;
	float timestamp_;
public:
	Frame();
	Frame(void* src, int byteSize, float timestamp);
	void SetData(void* src, int byteSize, float timestamp);
	const uint8_t* const GetData() const { return &data_[0]; }
	const int GetDataSize() const { return dataSize_; }
	const float GetTimestamp() const { return timestamp_; }
};

#endif