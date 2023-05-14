#pragma once
#include <Windows.h>

namespace FileUtils
{
	bool write_memory_to_file(HANDLE hFile, LONG offset, DWORD size, LPCVOID dataBuffer)
	{
		DWORD lpNumberOfBytesWritten = 0;
		DWORD retValue = 0;
		DWORD dwError = 0;

		if ((hFile != INVALID_HANDLE_VALUE) && dataBuffer)
		{
			retValue = SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
			dwError = GetLastError();

			if ((retValue == INVALID_SET_FILE_POINTER) && (dwError != NO_ERROR))
			{
				return false;
			}
			else
			{
				if (WriteFile(hFile, dataBuffer, size, &lpNumberOfBytesWritten, 0))
				{
					return true;
				}
				else
				{
					return false;
				}
			}
		}
		else
		{
			return false;
		}
	}

	bool write_memory_to_new_file(const CHAR* file, DWORD size, LPCVOID dataBuffer)
	{
		HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

		if (hFile != INVALID_HANDLE_VALUE)
		{
			bool resultValue = write_memory_to_file(hFile, 0, size, dataBuffer);
			CloseHandle(hFile);
			return resultValue;
		}
		else
		{
			return false;
		}
	}
}
