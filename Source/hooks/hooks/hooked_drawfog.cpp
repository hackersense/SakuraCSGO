#include "..\hooks.hpp"

bool __stdcall Hooks::hooked_drawfog() noexcept
{
	if (std::uintptr_t(_ReturnAddress()) != memory->shouldDrawFogReturnAddress)
		return clientmode_hook.callOriginal<bool, 17>();

	return !configs.visuals.removals[REMOVALS_FOGS] || configs.visuals.fog;
}