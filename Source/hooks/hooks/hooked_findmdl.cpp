#include "..\hooks.hpp"

void __fastcall Hooks::hooked_findmdl(void* ecx, void* edx, char* filePath) noexcept
{
	localPlayer->m_iAddonBits() |= 0x10000 | 0x00800;
	if (strstr(filePath, "facemask_battlemask.mdl"))
		sprintf(filePath, "models/player/holiday/facemasks/facemask_dallas.mdl");
	return modelcache_hook.callOriginal<void, void*, 10>(ecx, filePath);
}
