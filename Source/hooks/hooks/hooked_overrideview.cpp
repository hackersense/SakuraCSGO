#include "..\hooks.hpp"
#include "..\..\features\visuals\world_esp.h"

void __stdcall Hooks::hooked_overrideview(CViewSetup* viewsetup) noexcept
{
	if (viewsetup && localPlayer)
	{
		static auto fakeducking = false;

		if (!fakeducking && cheat.globals.fakeducking)
			fakeducking = true;
		else if (fakeducking && !cheat.globals.fakeducking && (!localPlayer->get_animation_state()->m_flDuckAmount || localPlayer->get_animation_state()->m_flDuckAmount == 1.0f))
			fakeducking = false;

		if (!localPlayer->is_alive())
			fakeducking = false;

		auto weapon = localPlayer->getActiveWeapon();

		if (configs.esp.enabled)
		{
			auto currentWeapon = localPlayer->is_alive() ? weapon : ((localPlayer->m_iObserverMode() == OBS_MODE_IN_EYE || localPlayer->m_iObserverMode() == OBS_MODE_CHASE) ? localPlayer->m_hObserverTarget().Get()->getActiveWeapon() : nullptr);
			if (currentWeapon && configs.visuals.removals[REMOVALS_ZOOM] && (!localPlayer->is_alive() && currentWeapon ? localPlayer->m_hObserverTarget().Get()->m_bIsScoped() : localPlayer->m_bIsScoped()) && currentWeapon->m_zoomLevel() > 0)
				viewsetup->fov = (90.0f + (float)configs.visuals.fov);
			else
				viewsetup->fov += configs.visuals.fov;
		}

		if (weapon)
		{
			auto viewmodel = static_cast<entity_t*>(m_entitylist()->GetClientEntityFromHandle(localPlayer->m_hViewModel()));

			if (viewmodel)
			{
				auto eyeAng = viewsetup->angles;
				eyeAng.z -= (float)configs.visuals.viewmodel_roll;
				viewmodel->set_abs_angles(eyeAng);
			}
		}

		util::thirdperson(fakeducking);

		clientmode_hook.callOriginal<void, 18>(viewsetup);

		if (fakeducking)
		{
			viewsetup->origin = localPlayer->GetAbsOrigin() + Vector(0.0f, 0.0f, m_gamemovement()->GetPlayerViewOffset(false).z + 0.064f);

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
		return clientmode_hook.callOriginal<void, 18>(viewsetup);
}
