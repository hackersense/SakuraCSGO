#pragma once

#include <vector>
#include <iostream>
#include <string>
#include <codecvt>
#include <algorithm>

namespace StringUtils
{
	inline std::string getSubString(std::string origin, std::string pre, std::string max) noexcept
	{
		std::string::size_type Pos_From = origin.find(pre, 0);
		std::string::size_type Pos_Where = origin.find(max, 0);
		std::string strDis = origin.substr(Pos_From, Pos_Where);
		return strDis;
	}

	inline int GetAbsDays(int yea, int mont, int da) noexcept
	{
		int i;
		int month_day[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
		int year = yea - 1;
		int days = year * 365 + year / 4 - year / 100 + year / 400;
		if (yea % 4 == 0 && yea % 100 != 0 || yea % 400 == 0) month_day[1]++;
		for (i = 0; i < mont - 1; i++)
			days += month_day[i];
		days += da - 1;
		return days;
	}

	inline std::string wstringToString(std::wstring ws) noexcept
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().to_bytes(ws);
	}

	inline std::wstring stringToWString(std::string ws) noexcept
	{
		return std::wstring_convert<std::codecvt_utf8<wchar_t>>().from_bytes(ws);
	}

	inline std::vector<std::string> split(const std::string& str, std::string sp_string) noexcept
	{
		std::vector<std::string> vecString;
		int sp_stringLen = sp_string.size();
		int lastPosition = 0;
		int index = -1;
		while (-1 != (index = str.find(sp_string, lastPosition)))
		{
			vecString.push_back(str.substr(lastPosition, index - lastPosition));
			lastPosition = index + sp_stringLen;
		}
		std::string lastStr = str.substr(lastPosition);
		if (!lastStr.empty())
			vecString.push_back(lastStr);
		return vecString;
	}

	inline std::vector<std::wstring> split(const std::wstring& str, std::wstring sp_string) noexcept
	{
		std::vector<std::wstring> vecString;
		int sp_stringLen = sp_string.size();
		int lastPosition = 0;
		int index = -1;
		while (-1 != (index = str.find(sp_string, lastPosition)))
		{
			vecString.push_back(str.substr(lastPosition, index - lastPosition));
			lastPosition = index + sp_stringLen;
		}
		std::wstring lastStr = str.substr(lastPosition);
		if (!lastStr.empty())
			vecString.push_back(lastStr);
		return vecString;
	}

	inline std::string replaceAll(const std::string& str, std::string org_str, std::string rep_str) noexcept
	{
		std::vector<std::string> delimVec = split(str, org_str);
		if (delimVec.size() <= 0)
			return str;
		std::string target("");
		std::vector<std::string>::iterator it = delimVec.begin();
		for (; it != delimVec.end(); ++it)
			target = target + (*it) + rep_str;
		return target;
	}

	inline std::wstring replaceAll(const std::wstring& str, std::wstring org_str, std::wstring rep_str) noexcept
	{
		std::vector<std::wstring> delimVec = split(str, org_str);
		if (delimVec.size() <= 0)
			return str;
		std::wstring target(L"");
		std::vector<std::wstring>::iterator it = delimVec.begin();
		for (; it != delimVec.end(); ++it)
			target = target + (*it) + rep_str;
		return target;
	}

	inline std::string clearAllSpace(std::string str) noexcept
	{
		int index = 0;
		if (!str.empty())
			while ((index = str.find(' ', index)) != std::string::npos)
				str.erase(index, 1);

		return str;
	}

	inline std::string cutNext(std::string stream, std::string str) noexcept
	{
		int nPos = stream.find(str);

		if (nPos != -1)
			stream = stream.substr(nPos + str.size(), stream.size());
		return stream;
	}

	inline std::string cutPre(std::string stream, std::string str) noexcept
	{
		int nPos = stream.find(str);
		if (nPos != -1)
			stream = stream.substr(0, nPos);
		return stream;
	}

	inline std::wstring cutNext(std::wstring stream, std::wstring str) noexcept
	{
		int nPos = stream.find(str);

		if (nPos != -1)
			stream = stream.substr(nPos + str.size(), stream.size());
		return stream;
	}

	inline std::wstring cutPre(std::wstring stream, std::wstring str) noexcept
	{
		int nPos = stream.find(str);
		if (nPos != -1)
			stream = stream.substr(0, nPos);
		return stream;
	}

	inline std::string trim(std::string s) noexcept
	{
		if (s.empty())
			return s;

		std::string::iterator c;
		// Erase whitespace before the string
		for (c = s.begin(); c != s.end() && iswspace(*c++););
		s.erase(s.begin(), --c);

		// Erase whitespace after the string
		for (c = s.end(); c != s.begin() && iswspace(*--c););
		s.erase(++c, s.end());

		return s;
	}

	inline std::wstring trim(std::wstring s) noexcept
	{
		if (s.empty())
			return s;

		std::wstring::iterator c;
		// Erase whitespace before the string
		for (c = s.begin(); c != s.end() && iswspace(*c++););
		s.erase(s.begin(), --c);

		// Erase whitespace after the string
		for (c = s.end(); c != s.begin() && iswspace(*--c););
		s.erase(++c, s.end());

		return s;
	}

	inline std::string toUpper(std::string s)
	{
		transform(s.begin(), s.end(), s.begin(),
			[](char c) { return toupper(c); });
		return s;
	}

	inline std::wstring toUpper(std::wstring s)
	{
		transform(s.begin(), s.end(), s.begin(),
			[](wchar_t c) { return toupper(c); });
		return s;
	}

	inline std::string toLower(std::string s)
	{
		transform(s.begin(), s.end(), s.begin(),
			[](char c) { return tolower(c); });
		return s;
	}

	inline std::wstring toLower(std::wstring s)
	{
		transform(s.begin(), s.end(), s.begin(),
			[](wchar_t c) { return tolower(c); });
		return s;
	}

	inline char* creat(std::string txt)
	{
		char* text = (char*)txt.c_str();
		for (int i = 0; i < strlen(text); i++)
			text[i] += 114514;
		return text;
	}

	inline char* trans(std::string txt)
	{
		char* text = (char*)txt.c_str();
		for (int i = 0; i < strlen(text); i++)
			text[i] -= 114514;
		return text;
	}
}