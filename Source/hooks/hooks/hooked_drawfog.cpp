#include "..\hooks.hpp"

using DrawFog_t = bool(__thiscall*)(void*);

bool __fastcall Hooks::hooked_drawfog(void* ecx, void* edx)
{
	static auto original_fn = clientmode_hook->getOriginal<DrawFog_t>(17);

	if (std::uintptr_t(_ReturnAddress()) != memory->shouldDrawFogReturnAddress)
		return original_fn(ecx);

	return !configs.visuals.removals[REMOVALS_FOGS] || configs.visuals.fog;
}