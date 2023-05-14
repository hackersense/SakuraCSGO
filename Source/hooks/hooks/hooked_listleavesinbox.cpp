#include "..\hooks.hpp"

struct RenderableInfo_t
{
	IClientRenderable* m_pRenderable;
	void* m_pAlphaProperty;
	int m_EnumCount;
	int m_nRenderFrame;
	unsigned short m_FirstShadow;
	unsigned short m_LeafList;
	short m_Area;
	uint16_t m_Flags;
	uint16_t m_Flags2;
	Vector m_vecBloatedAbsMins;
	Vector m_vecBloatedAbsMaxs;
	Vector m_vecAbsMins;
	Vector m_vecAbsMaxs;
};

using ListLeavesInBox_t = int(__thiscall*)(void*, const Vector&, const Vector&, unsigned short*, int);

int __fastcall Hooks::hooked_listleavesinbox(void* ecx, void* edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max)
{
	static auto original_fn = bspquery_hook->getOriginal<ListLeavesInBox_t>(6);

	if (!cheat.local())
		return original_fn(ecx, mins, maxs, list, list_max);

	if (!configs.player.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE] && !configs.player.type[TEAM].chams[PLAYER_CHAMS_VISIBLE] && !configs.player.type[LOCAL].chams[PLAYER_CHAMS_VISIBLE] && !configs.player.fake_chams_enabled && !configs.player.backtrack_chams)
		return original_fn(ecx, mins, maxs, list, list_max);

	if (std::uintptr_t(_ReturnAddress()) != memory->insertIntoTree)
		return original_fn(ecx, mins, maxs, list, list_max);

	//auto info = *reinterpret_cast<RenderableInfo_t**>(std::uintptr_t(_AddressOfReturnAddress()) + 0x14);
	auto info = *reinterpret_cast<RenderableInfo_t**>(std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t) + 0x18);

	if (!info || !info->m_pRenderable || !info->m_pRenderable->GetIClientUnknown()->GetBaseEntity()->is_player())
		return original_fn(ecx, mins, maxs, list, list_max);

	info->m_Flags &= ~0x100;
	info->m_Flags2 |= 0xC0;

	static const Vector map_min = Vector(-16384.f, -16384.f, -16384.f);
	static const Vector map_max = Vector(16384.f, 16384.f, 16384.f);
	return original_fn(ecx, std::cref(map_min), std::cref(map_max), list, list_max);
}