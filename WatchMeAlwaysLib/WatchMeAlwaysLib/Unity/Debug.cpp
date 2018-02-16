#include "stdafx.h"
#include "Debug.h"


void deleteFile(FILE* ptr) {
	if (ptr) {
		fclose(ptr);
	}
}

Debug::FilePtr Debug::file = Debug::FilePtr(nullptr, deleteFile);

