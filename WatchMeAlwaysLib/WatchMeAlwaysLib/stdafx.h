#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows �w�b�_�[����g�p����Ă��Ȃ����������O���܂��B
#include <windows.h>

#include <cassert>

#define DllExport   __declspec( dllexport )  
#define SAFE_DELETE(x) if (x != nullptr) { delete x; x = nullptr; }
