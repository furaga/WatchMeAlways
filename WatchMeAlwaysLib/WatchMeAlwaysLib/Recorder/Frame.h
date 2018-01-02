#pragma once
#ifndef FRAME_H

#include <stdint.h>

class Frame {
	uint8_t* data;
	int dataSize;
public:
	Frame(void* src, int byteSize);
	const uint8_t* const GetData() const { return data; }
	const int GetDataSize() const { return dataSize; }
};

#endif