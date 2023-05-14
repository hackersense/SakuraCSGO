#include "..\hooks.hpp"

_declspec(noinline)const char* Hooks::getforeignfallbackfontname_detour(void* ecx, uint32_t i)
{
	if (cheat.last_font_name.empty())
		return ((GetForeignFallbackFontNameFn)original_getforeignfallbackfontname)(ecx);

	return cheat.last_font_name.c_str();
}

const char* __fastcall Hooks::hooked_getforeignfallbackfontname(void* ecx, uint32_t i)
{
	return getforeignfallbackfontname_detour(ecx, i);
}