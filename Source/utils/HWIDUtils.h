#pragma once

#include <Windows.h>
#include <winioctl.h>  
#include <tchar.h>
#include <string>

namespace HWIDUtils
{
	static const char* getCPUID() noexcept;
	bool GetPhyDriveSerial(LPTSTR pModelNo, LPTSTR pSerialNo) noexcept;
	void ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, LPTSTR pBuf) noexcept;
	void TrimStart(LPTSTR pBuf) noexcept;
	std::string getHWID() noexcept;
	void getcpuid(unsigned int* CPUInfo, unsigned int InfoType);
	void getcpuidex(unsigned int* CPUInfo, unsigned int InfoType, unsigned int ECXValue);
}
