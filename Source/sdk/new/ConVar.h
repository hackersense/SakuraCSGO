#pragma once

#include <type_traits>

#include "..\misc\UtlVector.hpp"
#include "..\VirtualMethod.h"
#include "CVar.h"

/*
enum CvarFlags
{
	NONE = 0,
	UNREGISTERED = (1 << 0),
	DEVELOPMENTONLY = (1 << 1),
	GAMEDLL = (1 << 2),
	CLIENTDLL = (1 << 3),
	HIDDEN = (1 << 4),
	PROTECTED = (1 << 5),
	SPONLY = (1 << 6),
	ARCHIVE = (1 << 7),
	NOTIFY = (1 << 8),
	USERINFO = (1 << 9),
	CHEAT = (1 << 14),
	PRINTABLEONLY = (1 << 10),
	UNLOGGED = (1 << 11),
	NEVER_AS_STRING = (1 << 12),
	REPLICATED = (1 << 13),
	DEMO = (1 << 16),
	DONTRECORD = (1 << 17),
	NOT_CONNECTED = (1 << 22),
	ARCHIVE_XBOX = (1 << 24),
	SERVER_CAN_EXECUTE = (1 << 28),
	SERVER_CANNOT_QUERY = (1 << 29),
	CLIENTCMD_CAN_EXECUTE = (1 << 30)
};
*/

struct NEWConVar {
    VIRTUAL_METHOD(float, GetFloat, 12, (), (this))
    VIRTUAL_METHOD(int, GetInt, 13, (), (this))
    VIRTUAL_METHOD(void, SetValue, 14, (const char* value), (this, value))
    VIRTUAL_METHOD(void, SetValue, 15, (float value), (this, value))
    VIRTUAL_METHOD(void, SetValue, 16, (int value), (this, value))

		/*
	char* GetString()
	{
		auto iterator = **reinterpret_cast<NEWConCommandBase***>(reinterpret_cast<DWORD>(this) + 0x34);
		if (iterator->flags & FCVAR_NEVER_AS_STRING)
			return "FCVAR_NEVER_AS_STRING";
		char const* str = parent->string;
		return str ? str : "";
	}*/

	bool GetBool() { return !!GetInt(); }

    std::byte pad[24];
    std::add_pointer_t<void __cdecl()> changeCallback;
	NEWConVar* parent;
    const char* defaultValue;
    char* string;
	std::byte pad2[44];
	SafeUtlVector<void(__cdecl*)()> onChangeCallbacks;
};