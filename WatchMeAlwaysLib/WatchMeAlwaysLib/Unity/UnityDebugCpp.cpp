#include "stdafx.h"
#include "UnityDebugCpp.h"

#include<stdio.h>
#include <string>
#include <stdio.h>
#include <sstream>

static UnityPrintLogFn unityPrintLog = nullptr;

//-------------------------------------------------------------------
void  UnityDebugCpp::Log(const char* message, Color color) {
	if (unityPrintLog != nullptr)
		unityPrintLog(message, (int)color, (int)strlen(message));
}

void  UnityDebugCpp::Log(const std::string message, Color color) {
	const char* tmsg = message.c_str();
	if (unityPrintLog != nullptr)
		unityPrintLog(tmsg, (int)color, (int)strlen(tmsg));
}

void  UnityDebugCpp::Log(const int message, Color color) {
	std::stringstream ss;
	ss << message;
	printLog(ss, color);
}

void  UnityDebugCpp::Log(const char message, Color color) {
	std::stringstream ss;
	ss << message;
	printLog(ss, color);
}

void  UnityDebugCpp::Log(const float message, Color color) {
	std::stringstream ss;
	ss << message;
	printLog(ss, color);
}

void  UnityDebugCpp::Log(const double message, Color color) {
	std::stringstream ss;
	ss << message;
	printLog(ss, color);
}

void UnityDebugCpp::Log(const bool message, Color color) {
	std::stringstream ss;
	if (message)
		ss << "true";
	else
		ss << "false";

	printLog(ss, color);
}

void UnityDebugCpp::printLog(const std::stringstream &ss, const Color &color) {
	const std::string tmp = ss.str();
	const char* tmsg = tmp.c_str();
	if (unityPrintLog != nullptr) {
		unityPrintLog(tmsg, (int)color, (int)strlen(tmsg));
	}
}
//-------------------------------------------------------------------

//Create a callback delegate
void RegisterUnityPrintLogFn(UnityPrintLogFn cb) {
	unityPrintLog = cb;
}