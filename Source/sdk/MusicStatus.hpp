#pragma once

#include "..\includes.hpp"
#include <TlHelp32.h>
#include "..\utils\StringUtils.h"

enum class MusicPlayerType
{
	None,
	Spotify,
	NeteaseCloudMusic,
	YesPlayMusic,
	HyPlayer
};

static const char* MusicPlayerTypeStr[] =
{
	"None",
	"Spotify",
	"NeteaseCloudMusic",
	"YesPlayMusic",
	"HyPlayer"
};

struct MStatus
{
	std::wstring name = L"";
	std::wstring artist = L"";
	bool playing = false;
	bool find = false;
	MusicPlayerType musicPlayerType = MusicPlayerType::None;
};

typedef struct EnumHWndsArg
{
	std::vector<HWND>* vecHWnds;
	DWORD dwProcessId;
} EnumHWndsArg, * LPEnumHWndsArg;

namespace MSC {
	std::wstring ttl = L"";

	//get window enumeration
	BOOL CALLBACK enumWindowsProc(HWND hwnd, LPARAM lParam) {
		const auto& pids = *reinterpret_cast<std::vector<DWORD>*>(lParam);

		DWORD winId;
		GetWindowThreadProcessId(hwnd, &winId);

		if (pids.empty())
			return FALSE;

		for (DWORD pid : pids) {
			if (winId == pid) {
				std::wstring title(GetWindowTextLengthW(hwnd) + 1, L'\0');
				GetWindowTextW(hwnd, &title[0], title.size());
				if (wcsstr(title.c_str(), L"GDI+ Window (Spotify.exe)") || wcsstr(title.c_str(), L"Sogou_TSF_UI") || wcsstr(title.c_str(), L"Default IME") || title.empty() || wcsstr(title.c_str(), L"MSCTFIME UI")) {
					continue;
				}
				if (!wcsstr(title.c_str(), L" - ") && !wcsstr(title.c_str(), L" \u00b7 ") && !wcsstr(title.c_str(), L"Spotify") && !wcsstr(title.c_str(), L"YesPlayMusic"))
					continue;
				//if (title.find(L" - ") == title.npos && title.find(L" \u00b7 ") == title.npos && title.find(L"Spotify") == title.npos && title.find(L"YesPlayMusic") == title.npos)
				//	continue;
				if (lstrlenW(title.c_str()) > 1) {
					//procID = pid;
					ttl = title;
				}
			}
		}

		return TRUE;
	}

	bool getWindowsTitle(const wchar_t* processName)
	{
		std::vector<DWORD> pids; //process ID's

		//get handle of our proc
		HANDLE snap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

		//get entry point
		PROCESSENTRY32W entry;
		entry.dwSize = sizeof(entry);

		//see if our snapshot is valid
		if (!Process32FirstW(snap, &entry)) {
			return false;
		}

		//check if the handle is to spotify or not
		do {
			if (std::wstring(entry.szExeFile) == processName) {
				pids.emplace_back(entry.th32ProcessID);
			}
		} while (Process32NextW(snap, &entry));


		//get enumeration of processes
		return EnumWindows(enumWindowsProc, reinterpret_cast<LPARAM>(&pids));
	}
}

class MusicStatus
{
public:
	MStatus getSpotify()
	{
		if (MSC::getWindowsTitle(L"Spotify.exe"))
		{
			if (MSC::ttl.find(L"Spotify") != MSC::ttl.npos)
			{
				return MStatus{ L"", L"", false, true, MusicPlayerType::Spotify };
			}
			else if (MSC::ttl.find(L" - ") != MSC::ttl.npos)
			{
				std::wstring name = StringUtils::trim(StringUtils::cutNext(MSC::ttl, L" - "));
				std::wstring artist = StringUtils::trim(StringUtils::cutPre(MSC::ttl, L" - "));
				return MStatus{ name, artist, true, true, MusicPlayerType::Spotify };

			}
			else
				return MStatus{ L"", L"", false, true, MusicPlayerType::Spotify };
		}

		return MStatus{ L"", L"", false, false, MusicPlayerType::Spotify };
	}

	MStatus getYesPlayMusic()
	{
		if (MSC::getWindowsTitle(L"YesPlayMusic.exe"))
		{
			if (!lstrcmpW(MSC::ttl.c_str(), L"YesPlayMusic"))
			{
				return MStatus{ L"", L"", false, true, MusicPlayerType::YesPlayMusic };
			}
			else if (MSC::ttl.find(L" - YesPlayMusic") != MSC::ttl.npos)
			{
				std::wstring name = StringUtils::trim(StringUtils::cutPre(MSC::ttl, L" \u00b7 "));
				std::wstring artist = StringUtils::trim(StringUtils::replaceAll(StringUtils::cutNext(MSC::ttl, L" \u00b7 "), L" - YesPlayMusic", L""));
				return MStatus{ name, artist, true, true, MusicPlayerType::YesPlayMusic };
			}
			else
				return MStatus{ L"", L"", false, true, MusicPlayerType::YesPlayMusic };
		}

		return MStatus{ L"", L"", false, false, MusicPlayerType::YesPlayMusic };
	}
private:

};

inline MusicStatus musicStatus;