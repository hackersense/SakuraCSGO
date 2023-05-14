#include "ConsoleUtils.h"

std::uintptr_t ConsoleUtils::create_convar(const char* pName, const char* pDefaultValue, int flags,
	const char* pHelpString, bool bMin, float fMin,
	bool bMax, float fMax, FnChangeCallback_t callback) noexcept
{
	auto pVar = reinterpret_cast<ConVar*>(m_memalloc()->Alloc(0x58));
	memset(pVar, 0, 0x58);

	using create_t = uintptr_t(__thiscall*)(void*, const char*, const char*, int, const char*, bool bMin, float fMin,
		bool bMax, float fMax, FnChangeCallback_t callback);

	static auto FnOffset = util::FindSignature("client_panorama.dll", "55 8B EC F3 0F 10 45 ?? 56 6A 00");
	static auto CreateFn = reinterpret_cast<create_t>(util::FindSignature("client_panorama.dll", "55 8B EC 83 E4 F8 8B 45 0C 83 EC 10"));

	//pVar->vftable = *reinterpret_cast<PVOID*>(uintptr_t(FnOffset + 0x58));
	//pVar->convar_ref = *reinterpret_cast<PVOID*>(uintptr_t(FnOffset + 0x5F));

	return CreateFn(pVar, pName, pDefaultValue, flags, pHelpString, bMin, fMin, bMax, fMax, callback);
}
