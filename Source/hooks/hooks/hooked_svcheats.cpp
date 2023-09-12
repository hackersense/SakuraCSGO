#include "..\hooks.hpp"

bool __fastcall Hooks::hooked_svcheatsgetbool(void* ecx, void* edx) noexcept
{
	if (std::uintptr_t(_ReturnAddress()) == memory->cameraThink && cheat.globals.in_thirdperson)
		return true;

	return svcheats_hook.callOriginal<bool, void*, 13>(ecx);
}
