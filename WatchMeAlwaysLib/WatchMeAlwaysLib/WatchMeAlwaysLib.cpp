// WatchMeAlwaysLib.cpp : DLL アプリケーション用にエクスポートされる関数を定義します。
//

#include "stdafx.h"


extern "C" {
	DllExport float FooPluginFunction();
}

float counter = 0.0f;

float FooPluginFunction() {
	return counter++;
}