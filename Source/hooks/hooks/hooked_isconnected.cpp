#include "..\hooks.hpp"

bool __stdcall Hooks::hooked_isconnected()
{
	if (configs.misc.inventory_access && m_engine()->IsInGame() && std::uintptr_t(_ReturnAddress()) == memory->inventoryAccess)
		return false;

	return engine_hook.callOriginal<bool, 27>();
}

float __stdcall Hooks::hooked_getscreenaspectratio(int width, int height)
{
	if (configs.misc.aspect_ratio)
		return configs.misc.aspect_ratio;

	return engine_hook.callOriginal<float, 101>(width, height);
}
