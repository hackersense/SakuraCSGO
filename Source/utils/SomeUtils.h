#pragma once

#include <Windows.h>
#include <ctime>

namespace SomeUtils
{
	bool isFoolsDay()
	{
		auto curtime = std::time(0);
		auto time = std::localtime(&curtime);
		return time->tm_mon == 4 && time->tm_mday == 1;
	}

	bool isHalloween()
	{
		auto curtime = std::time(0);
		auto time = std::localtime(&curtime);
		return time->tm_mon == 11 && time->tm_mday == 1;
	}
}