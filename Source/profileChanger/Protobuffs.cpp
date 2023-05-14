#include "..\sdk\Memory.h"
#include "..\sdk\SteamAPI.h"
#include "Protobuffs.h"
#include "..\includes.hpp"
#include "Messages.h"
#include "ProtoWriter.h"
#include "valve_parser.h"
#include "pbwrap.hpp"
#include <array>

#define CAST(cast, address, add) reinterpret_cast<cast>((uint32_t)address + (uint32_t)add)

#define _gc2ch MatchmakingGC2ClientHello
#define _pci PlayerCommendationInfo
#define _pri PlayerRankingInfo
using namespace pbwrap;
struct zMatchmakingGC2ClientHello : pbmsg<20> {
	struct PlayerRankingInfo : pbmsg<6> {
		PBMSG_CTOR;
		PBFIELD(1, types::Uint32, account_id);
		PBFIELD(2, types::Uint32, rank_id);
		PBFIELD(3, types::Uint32, wins);
		PBFIELD(6, types::Uint32, rank_type_id);
	};
	struct PlayerCommendationInfo : pbmsg<4> {
		PBMSG_CTOR;
		PBFIELD(1, types::Uint32, cmd_friendly);
		PBFIELD(2, types::Uint32, cmd_teaching);
		PBFIELD(4, types::Uint32, cmd_leader);
	};

	PBMSG_CTOR;
	PBFIELD(7, PlayerRankingInfo, ranking);
	PBFIELD(8, PlayerCommendationInfo, commendation);
	PBFIELD(17, types::Int32, player_level);
};
struct zCMsgGCCStrike15_v2_ClientGCRankUpdate : pbmsg<1> {
	PBMSG_CTOR;
	PBFIELD(1, zMatchmakingGC2ClientHello::PlayerRankingInfo, ranking);
};

static std::string profileChanger(void* pubDest, uint32_t* pcubMsgSize)
{
	ProtoWriter msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8, 19);
	auto _commendation = msg.has(_gc2ch::commendation) ? msg.get(_gc2ch::commendation).String() : std::string("");
	ProtoWriter commendation(_commendation, 4);
	commendation.replace(Field(_pci::cmd_friendly, TYPE_UINT32, (int64_t)configs.profileChanger.friendly));
	commendation.replace(Field(_pci::cmd_teaching, TYPE_UINT32, (int64_t)configs.profileChanger.teacher));
	commendation.replace(Field(_pci::cmd_leader, TYPE_UINT32, (int64_t)configs.profileChanger.leader));
	msg.replace(Field(_gc2ch::commendation, TYPE_STRING, commendation.serialize()));
	auto _ranking = msg.has(_gc2ch::ranking) ? msg.get(_gc2ch::ranking).String() : std::string("");
	ProtoWriter ranking(_ranking, 6);

	ranking.replace(Field(_pri::rank_id, TYPE_UINT32, (int64_t)configs.profileChanger.rank));
	ranking.replace(Field(_pri::wins, TYPE_UINT32, (int64_t)configs.profileChanger.wins));
	msg.replace(Field(_gc2ch::ranking, TYPE_STRING, ranking.serialize()));

	msg.replace(Field(_gc2ch::player_level, TYPE_INT32, (int64_t)configs.profileChanger.level));
	msg.replace(Field(_gc2ch::player_cur_xp, TYPE_INT32, (int64_t)configs.profileChanger.exp));

	msg.replace(Field(_gc2ch::penalty_reason, TYPE_INT32, (int64_t)configs.profileChanger.ban_type));
	msg.replace(Field(_gc2ch::penalty_seconds, TYPE_INT32, (int64_t)configs.profileChanger.ban_time));

	return msg.serialize();
}

void Protobuffs::writePacket(std::string packet, void* thisPtr, void* oldEBP, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	if ((uint32_t)packet.size() <= cubDest - 8)
	{
		memcpy((void*)((DWORD)pubDest + 8), (void*)packet.data(), packet.size());
		*pcubMsgSize = packet.size() + 8;
	}
	else if (m_memalloc())
	{
		auto memPtr = *CAST(void**, thisPtr, 0x14);
		auto memPtrSize = *CAST(uint32_t*, thisPtr, 0x18);
		auto newSize = (memPtrSize - cubDest) + packet.size() + 8;

		auto memory = m_memalloc()->Realloc(memPtr, newSize + 4);

		*CAST(void**, thisPtr, 0x14) = memory;
		*CAST(uint32_t*, thisPtr, 0x18) = newSize;
		*CAST(void**, oldEBP, -0x14) = memory;

		memcpy(CAST(void*, memory, 24), (void*)packet.data(), packet.size());

		*pcubMsgSize = packet.size() + 8;
	}
}

bool Protobuffs::sendClientGcRankUpdateMatchmaking()
{
	ProtoWriter msg(0);
	auto packet = msg.serialize();
	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->sendMessage(k_EMsgGCCStrike15_v2_MatchmakingClient2GCHello | ((DWORD)1 << 31), ptr, packet.size() + 8) == SteamResult::OK;
	free(ptr);

	return result;
}

bool Protobuffs::sendClientGcRankUpdateWingman()
{
	zMatchmakingGC2ClientHello::PlayerRankingInfo rank_wingman;
	rank_wingman.rank_type_id().set(7); // 6 - mm, 7 - wingman, 10 - zone

	zCMsgGCCStrike15_v2_ClientGCRankUpdate msg;
	msg.ranking().set(rank_wingman);

	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->sendMessage(k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31), ptr, packet.size() + 8) == SteamResult::OK;
	free(ptr);

	return result;
}

bool Protobuffs::sendClientGcRankUpdateZone()
{
	zMatchmakingGC2ClientHello::PlayerRankingInfo rank_zone;
	rank_zone.rank_type_id().set(10);

	zCMsgGCCStrike15_v2_ClientGCRankUpdate msg;
	msg.ranking().set(rank_zone);

	auto packet = msg.serialize();

	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->sendMessage(k_EMsgGCCStrike15_v2_ClientGCRankUpdate | ((DWORD)1 << 31), ptr, packet.size() + 8) == SteamResult::OK;
	free(ptr);

	return result;
}

void Protobuffs::receiveMessage(void* thisPtr, void* oldEBP, uint32_t messageType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize)
{
	if (!configs.profileChanger.enabled)
		return;

	if (messageType == k_EMsgGCCStrike15_v2_MatchmakingGC2ClientHello)
	{
		auto packet = profileChanger(pubDest, pcubMsgSize);
		writePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
	}
	else if (messageType == k_EMsgGCCStrike15_v2_ClientGCRankUpdate)
	{
		zCMsgGCCStrike15_v2_ClientGCRankUpdate msg((void*)((DWORD)pubDest + 8), *pcubMsgSize - 8);

		auto ranking = msg.ranking().get();

		if (ranking.rank_type_id().get() == 7)
		{
			ranking.rank_id().set(configs.profileChanger.r_rank);
			ranking.wins().set(configs.profileChanger.r_wins);
			msg.ranking().set(ranking);
			auto packet = msg.serialize();
			writePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
		else if (ranking.rank_type_id().get() == 10)
		{
			ranking.rank_id().set(configs.profileChanger.t_rank);
			ranking.wins().set(configs.profileChanger.t_wins);
			msg.ranking().set(ranking);
			auto packet = msg.serialize();
			writePacket(packet, thisPtr, oldEBP, pubDest, cubDest, pcubMsgSize);
		}
	}
}

bool Protobuffs::sendClientHello()
{
	ProtoWriter msg(0);
	auto packet = msg.serialize();
	void* ptr = malloc(packet.size() + 8);

	if (!ptr)
		return false;

	((uint32_t*)ptr)[0] = k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31);
	((uint32_t*)ptr)[1] = 0;

	memcpy((void*)((DWORD)ptr + 8), (void*)packet.data(), packet.size());
	bool result = memory->steamGameCoordinator->sendMessage(k_EMsgGCAdjustItemEquippedState | ((DWORD)1 << 31), ptr, packet.size() + 8) == SteamResult::OK;
	free(ptr);

	return result;
}
