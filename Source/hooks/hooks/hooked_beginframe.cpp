#include "..\hooks.hpp"

const char* __fastcall Hooks::hooked_getforeignfallbackfontname(void* ecx, void* edx) noexcept
{
	if (cheat.last_font_name.empty())
		return getforeignfallbackfontname_hook.callOriginal<const char*, void*>(ecx);

	return cheat.last_font_name.c_str();
}