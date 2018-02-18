#pragma once
#ifndef FRAME_H

#include <vector>

class Frame {
	std::vector<uint8_t> data_;
	int dataSize_;
	int timestamp_;
public:
	Frame();
	void SetData(void* src, int byteSize, int timestamp);
	const uint8_t* const GetData() const { return &data_[0]; }
	const int GetDataSize() const { return dataSize_; }
	const int GetTimestamp() const { return timestamp_; }
};

#endif