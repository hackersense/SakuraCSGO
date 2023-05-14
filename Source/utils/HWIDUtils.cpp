#include "HWIDUtils.h"
#include "EncodeUtils.h"

static const char* HWIDUtils::getCPUID() noexcept
{
	static char pCpuId[32];
	int dwBuf[4];
	getcpuid((unsigned int*)dwBuf, 1);
	sprintf(pCpuId, "%08X", dwBuf[3]);
	sprintf(pCpuId + 8, "%08X", dwBuf[0]);
	return pCpuId;
}

void HWIDUtils::getcpuid(unsigned int* CPUInfo, unsigned int InfoType)
{
	getcpuidex(CPUInfo, InfoType, 0);
}

void HWIDUtils::getcpuidex(unsigned int* CPUInfo, unsigned int InfoType, unsigned int ECXValue)
{
	if (NULL == CPUInfo)
		return;

	_asm {
		// load.
		mov edi, CPUInfo;
		mov eax, InfoType;
		mov ecx, ECXValue;
		// CPUID  
		cpuid;
		// save.
		mov[edi], eax;
		mov[edi + 4], ebx;
		mov[edi + 8], ecx;
		mov[edi + 12], edx;
	}
}

bool HWIDUtils::GetPhyDriveSerial(LPTSTR pModelNo, LPTSTR pSerialNo) noexcept
{
	BYTE IdentifyResult[sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1];
	DWORD dwBytesReturned;
	GETVERSIONINPARAMS get_version;
	SENDCMDINPARAMS send_cmd = { 0 };

	HANDLE hFile = CreateFile(_T("\\\\.\\PHYSICALDRIVE0"), GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
	if (hFile == INVALID_HANDLE_VALUE)
		return false;

	DeviceIoControl(hFile, SMART_GET_VERSION, nullptr, 0,
		&get_version, sizeof(get_version), &dwBytesReturned, nullptr);

	send_cmd.irDriveRegs.bCommandReg = (get_version.bIDEDeviceMap & 0x10) ? ATAPI_ID_CMD : ID_CMD;
	DeviceIoControl(hFile, SMART_RCV_DRIVE_DATA, &send_cmd, sizeof(SENDCMDINPARAMS) - 1,
		IdentifyResult, sizeof(IdentifyResult), &dwBytesReturned, nullptr);
	CloseHandle(hFile);

	PUSHORT pWords = (USHORT*)(((SENDCMDOUTPARAMS*)IdentifyResult)->bBuffer);
	ToLittleEndian(pWords, 27, 46, pModelNo);
	ToLittleEndian(pWords, 10, 19, pSerialNo);
	return true;
}

void HWIDUtils::ToLittleEndian(PUSHORT pWords, int nFirstIndex, int nLastIndex, LPTSTR pBuf) noexcept
{
	int index;
	LPTSTR pDest = pBuf;
	for (index = nFirstIndex; index <= nLastIndex; ++index)
	{
		pDest[0] = pWords[index] >> 8;
		pDest[1] = pWords[index] & 0xFF;
		pDest += 2;
	}
	*pDest = 0;

	--pDest;
	while (*pDest == 0x20)
	{
		*pDest = 0;
		--pDest;
	}
}

void HWIDUtils::TrimStart(LPTSTR pBuf) noexcept
{
	if (*pBuf != 0x20)
		return;

	LPTSTR pDest = pBuf;
	LPTSTR pSrc = pBuf + 1;
	while (*pSrc == 0x20)
		++pSrc;

	while (*pSrc)
	{
		*pDest = *pSrc;
		++pDest;
		++pSrc;
	}
	*pDest = 0;
}

static std::string TCHARTurnString(TCHAR* STR)
{
	int iLen = WideCharToMultiByte(CP_ACP, 0, LPCWCH(STR), -1, NULL, 0, NULL, NULL);
	char* chRtn = new char[iLen * sizeof(char)];
	WideCharToMultiByte(CP_ACP, 0, LPCWCH(STR), -1, chRtn, iLen, NULL, NULL);
	return std::string(chRtn);
}

std::string HWIDUtils::getHWID() noexcept
{
	std::string cpuid = getCPUID();
	if (!cpuid.empty())
	{
		return cpuid;
	}
	else
	{
		TCHAR szModelNo[48], szSerialNo[24];
		if (GetPhyDriveSerial(szModelNo, szSerialNo))
		{
			TrimStart(szSerialNo);
			return EncodeUtils().MD5("MD" + TCHARTurnString(szSerialNo));
		}
	}

	return "";
}
