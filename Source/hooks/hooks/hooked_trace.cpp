#include "..\hooks.hpp"

void __fastcall Hooks::hooked_clip_ray_collideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace) noexcept
{
	auto backup_maxs = pCollide->OBBMaxs().z;
	pCollide->OBBMaxs().z += 5.0f;

	trace_hook.callOriginal<void, void*, 4>(ecx, std::cref(ray), fMask, pCollide, pTrace);

	pCollide->OBBMaxs().z = backup_maxs;
}

void __fastcall Hooks::hooked_trace_ray(void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace) noexcept
{
	if (cheat.globals.autowalling)
		pTrace->surface.flags |= SURF_SKY;

	trace_hook.callOriginal<void, void*, 5>(ecx, std::cref(ray), fMask, pTraceFilter, pTrace);
}