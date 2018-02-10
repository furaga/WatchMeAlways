#pragma once
#ifndef FRAME_H

#include <vector>

class Frame {
	std::vector<uint8_t> data;
	int dataSize;
public:
	Frame();
	void SetData(void* src, int byteSize);
	const uint8_t* const GetData() const { return &data[0]; }
	const int GetDataSize() const { return dataSize; }
};

#endif