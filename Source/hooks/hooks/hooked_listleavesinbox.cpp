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

int __stdcall Hooks::hooked_listleavesinbox(const Vector& min, const Vector& max, unsigned short* list, int listMax) noexcept
{
	if (!localPlayer)
		return bspquery_hook.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);

	if (!configs.esp.type[ENEMY].chams[PLAYER_CHAMS_VISIBLE] && !configs.esp.type[TEAM].chams[PLAYER_CHAMS_VISIBLE] && !configs.esp.type[LOCAL].chams[PLAYER_CHAMS_VISIBLE] && !configs.esp.fake_chams_enabled && !configs.esp.backtrack_chams)
		return bspquery_hook.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);

	if (std::uintptr_t(_ReturnAddress()) != memory->insertIntoTree)
		return bspquery_hook.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);

	//auto info = *reinterpret_cast<RenderableInfo_t**>(std::uintptr_t(_AddressOfReturnAddress()) + 0x14);
	auto info = *reinterpret_cast<RenderableInfo_t**>(std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t) + 0x18);

	if (!info || !info->m_pRenderable || !info->m_pRenderable->GetIClientUnknown()->GetBaseEntity()->is_player())
		return bspquery_hook.callOriginal<int, 6>(std::cref(min), std::cref(max), list, listMax);

	info->m_Flags &= ~0x100;
	info->m_Flags2 |= 0xC0;

	constexpr float maxCoord = 16384.0f;
	constexpr float minCoord = -maxCoord;
	const Vector mins{ minCoord, minCoord, minCoord };
	const Vector maxs{ maxCoord, maxCoord, maxCoord };

	return bspquery_hook.callOriginal<int, 6>(std::cref(mins), std::cref(maxs), list, listMax);
}