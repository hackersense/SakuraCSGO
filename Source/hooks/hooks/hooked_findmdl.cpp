#include "..\hooks.hpp"

void __fastcall Hooks::hooked_findmdl(void* ecx, void* edx, char* filePath)
{
	//cheat.local()->m_iAddonBits() |= 0x10000 | 0x00800;
	//if (strstr(FilePath, "facemask_battlemask.mdl"))
	//	sprintf(FilePath, "models/player/holiday/facemasks/facemask_dallas.mdl");
	return modelcache_hook.callOriginal<void, void*, 10>(ecx, filePath);
}
