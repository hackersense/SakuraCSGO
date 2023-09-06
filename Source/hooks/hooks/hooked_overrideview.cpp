#include "..\hooks.hpp"
#include "..\..\features\visuals\world_esp.h"

using OverrideView_t = void(__stdcall*)(CViewSetup*);

void __stdcall Hooks::hooked_overrideview(CViewSetup* viewsetup)
{
	static auto original_fn = clientmode_hook->getOriginal<OverrideView_t>(18);

	if (!viewsetup)
		return original_fn(viewsetup);

	if (cheat.local())
	{
		static auto fakeducking = false;

		if (!fakeducking && cheat.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !cheat.globals.fakeducking && (!cheat.local()->get_animation_state()->m_flDuckAmount || cheat.local()->get_animation_state()->m_flDuckAmount == 1.0f))
			fakeducking = false;

		if (!cheat.local()->is_alive())
			fakeducking = false;

		auto weapon = cheat.local()->m_hActiveWeapon().Get();

		if (configs.esp.enabled)
		{
			auto currentWeapon = cheat.local()->is_alive() ? weapon : ((cheat.local()->m_iObserverMode() == OBS_MODE_IN_EYE || cheat.local()->m_iObserverMode() == OBS_MODE_CHASE) ? cheat.local()->m_hObserverTarget().Get()->m_hActiveWeapon().Get() : nullptr);
			if (currentWeapon && configs.visuals.removals[REMOVALS_ZOOM] && (!cheat.local()->is_alive() && currentWeapon ? cheat.local()->m_hObserverTarget().Get()->m_bIsScoped() : cheat.local()->m_bIsScoped()) && currentWeapon->m_zoomLevel() > 0)
				viewsetup->fov = (90.0f + (float)configs.visuals.fov);
			else
				viewsetup->fov += configs.visuals.fov;
		}

		if (weapon)
		{
			auto viewmodel = static_cast<entity_t*>(m_entitylist()->GetClientEntityFromHandle(cheat.local()->m_hViewModel()));

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)configs.visuals.viewmodel_roll;
				viewmodel->set_abs_angles(eyeAng);
			}
		}

		util::thirdperson(fakeducking);

		original_fn(viewsetup);

		if (fakeducking)
		{
			viewsetup->origin = cheat.local()->GetAbsOrigin() + Vector(0.0f, 0.0f, m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f);

			if (m_input()->m_bCameraInThirdPerson)
			{
				auto camera_angles = Vector(m_input()->m_vecCameraOffset.x, m_input()->m_vecCameraOffset.y, 0.0f);
				auto camera_forward = ZERO;

				math::angle_vectors(camera_angles, camera_forward);
				math::VectorMA(viewsetup->origin, -m_input()->m_vecCameraOffset.z, camera_forward, viewsetup->origin);
			}
		}

		worldesp::get().motion_blur(viewsetup);
	}
	else
		return original_fn(viewsetup);
}
