#include "..\hooks.hpp"
#include "..\..\features\menu.h"

void __stdcall Hooks::hooked_setdrawcolor(int r, int g, int b, int a)
{
	if (configs.esp.enabled && configs.visuals.removals[REMOVALS_SCOPE] && (std::uintptr_t(_ReturnAddress()) == memory->scopeDust || std::uintptr_t(_ReturnAddress()) == memory->scopeArc))
		a = 0;

	surface_hook.callOriginal<void, 15>(r, g, b, a);
}

void __stdcall Hooks::hooked_lockcursor()
{
	if (Hooks::menu_open || !c_menu::get().load_success)
		return m_surface()->UnlockCursor();
	return surface_hook.callOriginal<void, 67>();
}
