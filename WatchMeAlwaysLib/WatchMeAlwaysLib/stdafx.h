#pragma once

#include "targetver.h"

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#include <windows.h>

#include <cassert>

#define DllExport   __declspec( dllexport )  
#define SAFE_DELETE(x) if (x != nullptr) { delete x; x = nullptr; }

typedef unsigned int uint;

#include <stdio.h>
#include <sstream>
#include <stdint.h>
#include <string>
#include <vector>
#include <memory>
#include "Unity/UnityDebugCpp.h"