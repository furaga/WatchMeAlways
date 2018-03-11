#include "stdafx.h"
#include "Debug.h"
#include <mutex>

std::mutex Debug::mutexDebugLog_;
std::string Debug::logFilePath_ = "watchmealways_server.log";
