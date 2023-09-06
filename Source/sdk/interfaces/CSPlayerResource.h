#pragma once

#include <windows.h>
#include "..\utils\netmanager.hpp"
#include "..\utils\crypt_str.h"
#include "..\VirtualMethod.h"
#include "..\math\Vector.hpp"

class ICSPlayerResource {
public:
	VIRTUAL_METHOD(bool, is_alive, 5, (int index), (this, index));
	VIRTUAL_METHOD(const char*, get_player_name, 8, (int index), (this, index));
	VIRTUAL_METHOD(int, get_player_health, 14, (int index), (this, index));
};

class CSPlayerResource 
{
public:
	auto getICSPlayerResource() noexcept
	{
		return reinterpret_cast<ICSPlayerResource*>(uintptr_t(this) + 0x9D8);
	}

	Vector GetBombsiteCenterA(int index)
	{
		static auto m_bombsiteCenterA = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_bombsiteCenterA"));
		return *reinterpret_cast<std::add_pointer_t<Vector>>(uintptr_t(this) + m_bombsiteCenterA + index * sizeof(int));
	}

	Vector GetBombsiteCenterB(int index)
	{
		static auto m_bombsiteCenterB = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_bombsiteCenterB"));
		return *reinterpret_cast<std::add_pointer_t<Vector>>(uintptr_t(this) + m_bombsiteCenterB + index * sizeof(int));
	}

	int GetArmor(int index)
	{
		static auto m_iArmor = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iArmor"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iArmor + index * sizeof(int));
	}

	int GetCompetitiveRanking(int index)
	{
		static auto m_iCompetitiveRanking = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iCompetitiveRanking"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iCompetitiveRanking + index * sizeof(int));
	}

	int GetCompetitiveWins(int index)
	{
		static auto m_iCompetitiveWins = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iCompetitiveWins"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iCompetitiveWins + index * sizeof(int));
	}

	int GetPing(int index)
	{
		static auto m_iPing = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iPing"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iPing + index * sizeof(int));
	}

	int GetKills(int index)
	{
		static auto m_iKills = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iKills"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iKills + index * sizeof(int));
	}

	int GetAssists(int index)
	{
		static auto m_iAssists = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iAssists"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iAssists + index * sizeof(int));
	}

	int GetDeaths(int index)
	{
		static auto m_iDeaths = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_iDeaths"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_iDeaths + index * sizeof(int));
	}

	int& m_nMusicID(int index)
	{
		static auto m_nMusicID = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_nMusicID"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_nMusicID + index * sizeof(int));
	}

	int& m_nActiveCoinRank(int index)
	{
		static auto m_nActiveCoinRank = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_nActiveCoinRank"));
		return *reinterpret_cast<std::add_pointer_t<int>>(uintptr_t(this) + m_nActiveCoinRank + index * sizeof(int));
	}

	bool GetConnected(int index)
	{
		static auto m_bConnected = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_bConnected"));
		return *reinterpret_cast<std::add_pointer_t<bool>>(uintptr_t(this) + m_bConnected + index * sizeof(bool));
	}

	std::string GetClanTag(int index)
	{
		static auto m_szClan = netvars::get().get_offset(crypt_str("CCSPlayerResource"), crypt_str("m_szClan"));
		return *reinterpret_cast<std::add_pointer_t<std::string>>(uintptr_t(this) + m_szClan + index * sizeof(bool));
	}
};

class CSGameRulesProxy
{
public:
	NETVAR(bool, m_bMapHasBombTarget, crypt_str("CCSGameRulesProxy"), crypt_str("m_bMapHasBombTarget"));
	NETVAR(bool, m_bFreezePeriod, crypt_str("CCSGameRulesProxy"), crypt_str("m_bFreezePeriod"));
	NETVAR(bool, m_bIsValveDS, crypt_str("CCSGameRulesProxy"), crypt_str("m_bIsValveDS"));
};