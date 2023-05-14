#include "..\hooks.hpp"

#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))

bool __stdcall Hooks::hooked_isplayingdemo()
{
	if (configs.misc.reveal_money && std::uintptr_t(_ReturnAddress()) == memory->demoOrHLTV && *reinterpret_cast<std::uintptr_t*>(FRAME_ADDRESS() + 8) == memory->money)
		return true;

	return engine_hook.callOriginal<bool, 82>();
}