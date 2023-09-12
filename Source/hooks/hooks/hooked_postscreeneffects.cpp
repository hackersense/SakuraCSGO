#include "..\hooks.hpp"

void __stdcall Hooks::hooked_postscreeneffects(void* param) noexcept
{
	if (!configs.esp.enabled || !localPlayer)
		return clientmode_hook.callOriginal<void, 44>(param);

	for (auto i = 0; i < m_glow()->m_GlowObjectDefinitions.Count(); i++)
	{
		if (m_glow()->m_GlowObjectDefinitions[i].IsUnused())
			continue;

		auto object = &m_glow()->m_GlowObjectDefinitions[i];
		auto entity = object->GetEnt();

		if (!entity)
			continue;

		auto client_class = entity->GetClientClass();

		if (!client_class)
			continue;

		if (entity->is_player())
		{
			auto e = reinterpret_cast<player_t*>(entity);
			auto should_glow = false;

			if (!e->valid(false, false))
				continue;

			auto type = ENEMY;

			if (e == localPlayer.get())
				type = LOCAL;
			else if (e->m_iTeamNum() == localPlayer->m_iTeamNum())
				type = TEAM;

			float color[4];

			if (configs.esp.type[ENEMY].glow && type == ENEMY)
			{
				should_glow = true;

				color[0] = configs.esp.type[ENEMY].glow_color[0] / 255.0f;
				color[1] = configs.esp.type[ENEMY].glow_color[1] / 255.0f;
				color[2] = configs.esp.type[ENEMY].glow_color[2] / 255.0f;
				color[3] = configs.esp.type[ENEMY].glow_color[3] / 255.0f;
			}
			else if (configs.esp.type[TEAM].glow && type == TEAM)
			{
				should_glow = true;

				color[0] = configs.esp.type[TEAM].glow_color[0] / 255.0f;
				color[1] = configs.esp.type[TEAM].glow_color[1] / 255.0f;
				color[2] = configs.esp.type[TEAM].glow_color[2] / 255.0f;
				color[3] = configs.esp.type[TEAM].glow_color[3] / 255.0f;
			}
			else if (configs.esp.type[LOCAL].glow && type == LOCAL)
			{
				should_glow = true;

				color[0] = configs.esp.type[LOCAL].glow_color[0] / 255.0f;
				color[1] = configs.esp.type[LOCAL].glow_color[1] / 255.0f;
				color[2] = configs.esp.type[LOCAL].glow_color[2] / 255.0f;
				color[3] = configs.esp.type[LOCAL].glow_color[3] / 255.0f;
			}

			if (!should_glow)
				continue;

			object->Set
			(
				color[0],
				color[1],
				color[2],
				configs.misc.anti_screenshot && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
				1.0f,
				configs.esp.type[type].glow_type
			);
		}
		else if (configs.visuals.weapon[WEAPON_GLOW] && (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor))
		{
			float color[4] =
			{
				color[0] = configs.visuals.weapon_glow_color[0] / 255.0f,
				color[1] = configs.visuals.weapon_glow_color[1] / 255.0f,
				color[2] = configs.visuals.weapon_glow_color[2] / 255.0f,
				color[3] = configs.visuals.weapon_glow_color[3] / 255.0f
			};

			object->Set
			(
				color[0],
				color[1],
				color[2],
				configs.misc.anti_screenshot && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
				1.0f,
				0
			);
		}
		else if (configs.visuals.grenade_esp[GRENADE_GLOW])
		{
			auto model = entity->GetModel();

			if (!model)
				continue;

			auto studio_model = m_modelinfo()->GetStudioModel(model);

			if (!studio_model)
				continue;

			std::string name = studio_model->name;

			if (name.find("flashbang") == std::string::npos && name.find("smokegrenade") == std::string::npos && name.find("incendiarygrenade") == std::string::npos && name.find("molotov") == std::string::npos && name.find("fraggrenade") == std::string::npos && name.find("decoy") == std::string::npos)
				continue;

			if (name.find("thrown") != std::string::npos ||
				client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
			{
				float color[4] =
				{
					color[0] = configs.visuals.grenade_glow_color[0] / 255.0f,
					color[1] = configs.visuals.grenade_glow_color[1] / 255.0f,
					color[2] = configs.visuals.grenade_glow_color[2] / 255.0f,
					color[3] = configs.visuals.grenade_glow_color[3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					configs.misc.anti_screenshot && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
					1.0f,
					0
				);
			}
			else if (name.find("dropped") != std::string::npos && configs.visuals.weapon[WEAPON_GLOW])
			{
				float color[4] =
				{
					color[0] = configs.visuals.weapon_glow_color[0] / 255.0f,
					color[1] = configs.visuals.weapon_glow_color[1] / 255.0f,
					color[2] = configs.visuals.weapon_glow_color[2] / 255.0f,
					color[3] = configs.visuals.weapon_glow_color[3] / 255.0f
				};

				object->Set
				(
					color[0],
					color[1],
					color[2],
					configs.misc.anti_screenshot && m_engine()->IsTakingScreenshot() ? 0.0f : color[3],
					1.0f,
					0
				);
			}
		}
	}

	clientmode_hook.callOriginal<void, 44>(param);
}
