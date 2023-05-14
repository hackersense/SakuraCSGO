#include "world_esp.h"

void worldesp::paint_traverse()
{
	skybox_changer();

	const auto HighestEntityIndex = m_entitylist()->GetHighestEntityIndex();
	for (int i = 1; i <= HighestEntityIndex; i++)
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_player())
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		switch (client_class->m_ClassID)
		{
		case CEnvTonemapController:
			world_modulation(e);
			break;
		case CInferno:
			molotov_timer(e);
			break;
		case CSmokeGrenadeProjectile:
			smoke_timer(e);
			break;
		case CPlantedC4:
			bomb_timer(e);
			break;
		case CC4:
			if (configs.player.type[ENEMY].flags[FLAGS_C4] || configs.player.type[TEAM].flags[FLAGS_C4] || configs.player.type[LOCAL].flags[FLAGS_C4] || configs.esp.bomb_timer)
			{
				auto owner = (player_t*)m_entitylist()->GetClientEntityFromHandle(e->m_hOwnerEntity());

				if ((configs.player.type[ENEMY].flags[FLAGS_C4] || configs.player.type[TEAM].flags[FLAGS_C4] || configs.player.type[LOCAL].flags[FLAGS_C4]) && owner->valid(false, false))
					cheat.globals.bomb_carrier = owner->EntIndex();
				else if (configs.esp.bomb_timer && !owner->is_player())
				{
					auto screen = ZERO;

					if (math::world_to_screen(e->GetAbsOrigin(), screen))
						render::get().text(fonts[ESP], screen.x, screen.y, Color(215, 20, 20), HFONT_CENTERED_X | HFONT_CENTERED_Y, "BOMB");
				}
			}

			break;
		default:
			grenade_projectiles(e);

			if (client_class->m_ClassID == CAK47 || client_class->m_ClassID == CDEagle || client_class->m_ClassID >= CWeaponAug && client_class->m_ClassID <= CWeaponZoneRepulsor)
				dropped_weapons(e);

			break;
		}
	}
}

void worldesp::skybox_changer()
{
	auto& skybox_name = backup_skybox;

	switch (configs.esp.skybox_changer)
	{
	case 1:
		skybox_name = "cs_tibet";
		break;
	case 2:
		skybox_name = "cs_baggage_skybox_";
		break;
	case 3:
		skybox_name = "italy";
		break;
	case 4:
		skybox_name = "jungle";
		break;
	case 5:
		skybox_name = "office";
		break;
	case 6:
		skybox_name = "sky_cs15_daylight01_hdr";
		break;
	case 7:
		skybox_name = "sky_cs15_daylight02_hdr";
		break;
	case 8:
		skybox_name = "vertigoblue_hdr";
		break;
	case 9:
		skybox_name = "vertigo";
		break;
	case 10:
		skybox_name = "sky_day02_05_hdr";
		break;
	case 11:
		skybox_name = "nukeblank";
		break;
	case 12:
		skybox_name = "sky_venice";
		break;
	case 13:
		skybox_name = "sky_cs15_daylight03_hdr";
		break;
	case 14:
		skybox_name = "sky_cs15_daylight04_hdr";
		break;
	case 15:
		skybox_name = "sky_csgo_cloudy01";
		break;
	case 16:
		skybox_name = "sky_csgo_night02";
		break;
	case 17:
		skybox_name = "sky_csgo_night02b";
		break;
	case 18:
		skybox_name = "sky_csgo_night_flat";
		break;
	case 19:
		skybox_name = "sky_dust";
		break;
	case 20:
		skybox_name = "vietnam";
		break;
	case 21:
		skybox_name = configs.esp.custom_skybox;
		break;
	}

	static auto skybox_number = 0;
	static auto& old_skybox_name = skybox_name;

	static auto color_r = 255;
	static auto color_g = 255;
	static auto color_b = 255;

	if (skybox_number != configs.esp.skybox_changer)
	{
		changed = true;
		skybox_number = configs.esp.skybox_changer;
	}
	else if (old_skybox_name != skybox_name)
	{
		changed = true;
		old_skybox_name = skybox_name;
	}
	else if (color_r != configs.esp.skybox_color[0])
	{
		changed = true;
		color_r = configs.esp.skybox_color[0];
	}
	else if (color_g != configs.esp.skybox_color[1])
	{
		changed = true;
		color_g = configs.esp.skybox_color[1];
	}
	else if (color_b != configs.esp.skybox_color[2])
	{
		changed = true;
		color_b = configs.esp.skybox_color[2];
	}

	if (changed)
	{
		changed = false;
		memory->loadSky(skybox_name.c_str());

		auto materialsystem = m_materialsystem();

		for (auto i = materialsystem->FirstMaterial(); i != materialsystem->InvalidMaterial(); i = materialsystem->NextMaterial(i))
		{
			auto material = materialsystem->GetMaterial(i);

			if (!material)
				continue;

			if (strstr(material->GetTextureGroupName(), crypt_str("SkyBox")))
				material->ColorModulate(configs.esp.skybox_color[0] / 255.0f, configs.esp.skybox_color[1] / 255.0f, configs.esp.skybox_color[2] / 255.0f);
		}
	}
}

void worldesp::fog_changer()
{
	static auto fog_override = m_cvar()->FindVar(crypt_str("fog_override"));

	if (!configs.esp.fog)
	{
		if (fog_override->GetBool())
			fog_override->SetValue(false);

		return;
	}

	if (!fog_override->GetBool())
		fog_override->SetValue(true);

	static auto fog_start = m_cvar()->FindVar(crypt_str("fog_start"));

	if (fog_start->GetInt())
		fog_start->SetValue(0);

	static auto fog_end = m_cvar()->FindVar(crypt_str("fog_end"));

	if (fog_end->GetInt() != configs.esp.fog_distance)
		fog_end->SetValue(configs.esp.fog_distance);

	static auto fog_maxdensity = m_cvar()->FindVar(crypt_str("fog_maxdensity"));

	if (fog_maxdensity->GetFloat() != (float)configs.esp.fog_density * 0.01f)
		fog_maxdensity->SetValue((float)configs.esp.fog_density * 0.01f);

	char buffer_color[12];
	sprintf_s(buffer_color, 12, "%i %i %i", configs.esp.fog_color.r(), configs.esp.fog_color.g(), configs.esp.fog_color.b());

	static auto fog_color = m_cvar()->FindVar(crypt_str("fog_color"));

	if (strcmp(fog_color->GetString(), buffer_color))
		fog_color->SetValue(buffer_color);
}

void worldesp::world_modulation(entity_t* entity)
{
	if (!configs.esp.world_modulation)
		return;

	entity->setUseCustomBloomScale(true);
	entity->setCustomBloomScale(configs.esp.bloom * 0.01f);

	entity->setUseCustomAutoExposureMin(true);
	entity->setCustomAutoExposureMin(configs.esp.exposure * 0.001f);

	entity->setUseCustomAutoExposureMax(true);
	entity->setCustomAutoExposureMax(configs.esp.exposure * 0.001f);
}

struct MotionBlurHistory
{
	MotionBlurHistory() noexcept
	{
		lastTimeUpdate = 0.0f;
		previousPitch = 0.0f;
		previousYaw = 0.0f;
		previousPositon = Vector{ 0.0f, 0.0f, 0.0f };
		noRotationalMotionBlurUntil = 0.0f;
	}

	float lastTimeUpdate;
	float previousPitch;
	float previousYaw;
	Vector previousPositon;
	float noRotationalMotionBlurUntil;
};

void worldesp::motion_blur(CViewSetup* setup)
{
	if (!configs.esp.motionblur)
		return;

	if (!cheat.available())
		return;

	static MotionBlurHistory history;
	static float motionBlurValues[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	if (setup)
	{
		const float timeElapsed = m_globals()->m_realtime - history.lastTimeUpdate;

		const auto viewangles = setup->angles;

		const float currentPitch = math::normalize_pitch(viewangles.x);
		const float currentYaw = math::normalize_yaw(viewangles.y);

		Vector currentSideVector;
		Vector currentForwardVector;
		Vector currentUpVector;
		math::angle_vectors(setup->angles, &currentForwardVector, &currentSideVector, &currentUpVector);

		Vector currentPosition = setup->origin;
		Vector positionChange = history.previousPositon - currentPosition;

		if ((positionChange.Length() > 30.0f) && (timeElapsed >= 0.5f))
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[2] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else if (timeElapsed > (1.0f / 15.0f))
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[2] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else if (positionChange.Length() > 50.0f)
		{
			history.noRotationalMotionBlurUntil = m_globals()->m_realtime + 1.0f;
		}
		else
		{
			const float horizontalFov = setup->fov;
			const float verticalFov = (setup->aspect_ratio <= 0.0f) ? (setup->fov) : (setup->fov / setup->aspect_ratio);
			const float viewdotMotion = currentForwardVector.Dot(positionChange);

			if (configs.esp.motionblur_forward)
				motionBlurValues[2] = viewdotMotion;

			const float sidedotMotion = currentSideVector.Dot(positionChange);
			float yawdiffOriginal = history.previousYaw - currentYaw;
			if (((history.previousYaw - currentYaw > 180.0f) || (history.previousYaw - currentYaw < -180.0f)) &&
				((history.previousYaw + currentYaw > -180.0f) && (history.previousYaw + currentYaw < 180.0f)))
				yawdiffOriginal = history.previousYaw + currentYaw;

			float yawdiffAdjusted = yawdiffOriginal + (sidedotMotion / 3.0f);

			if (yawdiffOriginal < 0.0f)
				yawdiffAdjusted = std::clamp(yawdiffAdjusted, yawdiffOriginal, 0.0f);
			else
				yawdiffAdjusted = std::clamp(yawdiffAdjusted, 0.0f, yawdiffOriginal);

			const float undampenedYaw = yawdiffAdjusted / horizontalFov;
			motionBlurValues[0] = undampenedYaw * (1.0f - (fabsf(currentPitch) / 90.0f));

			const float pitchCompensateMask = 1.0f - ((1.0f - fabsf(currentForwardVector[2])) * (1.0f - fabsf(currentForwardVector[2])));
			const float pitchdiffOriginal = history.previousPitch - currentPitch;
			float pitchdiffAdjusted = pitchdiffOriginal;

			if (currentPitch > 0.0f)
				pitchdiffAdjusted = pitchdiffOriginal - ((viewdotMotion / 2.0f) * pitchCompensateMask);
			else
				pitchdiffAdjusted = pitchdiffOriginal + ((viewdotMotion / 2.0f) * pitchCompensateMask);


			if (pitchdiffOriginal < 0.0f)
				pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, pitchdiffOriginal, 0.0f);
			else
				pitchdiffAdjusted = std::clamp(pitchdiffAdjusted, 0.0f, pitchdiffOriginal);

			motionBlurValues[1] = pitchdiffAdjusted / verticalFov;
			motionBlurValues[3] = undampenedYaw;
			motionBlurValues[3] *= (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f) * (fabs(currentPitch) / 90.0f);

			if (timeElapsed > 0.0f)
				motionBlurValues[2] /= timeElapsed * 30.0f;
			else
				motionBlurValues[2] = 0.0f;

			motionBlurValues[2] = std::clamp((fabsf(motionBlurValues[2]) - configs.esp.motionblur_falling_min) / (configs.esp.motionblur_falling_max - configs.esp.motionblur_falling_min), 0.0f, 1.0f) * (motionBlurValues[2] >= 0.0f ? 1.0f : -1.0f);
			motionBlurValues[2] /= 30.0f;
			motionBlurValues[0] *= configs.esp.motionblur_rotation_intensity * .15f * configs.esp.motionblur_strength;
			motionBlurValues[1] *= configs.esp.motionblur_rotation_intensity * .15f * configs.esp.motionblur_strength;
			motionBlurValues[2] *= configs.esp.motionblur_rotation_intensity * .15f * configs.esp.motionblur_strength;
			motionBlurValues[3] *= configs.esp.motionblur_falling_intensity * .15f * configs.esp.motionblur_strength;

		}

		if (m_globals()->m_realtime < history.noRotationalMotionBlurUntil)
		{
			motionBlurValues[0] = 0.0f;
			motionBlurValues[1] = 0.0f;
			motionBlurValues[3] = 0.0f;
		}
		else
		{
			history.noRotationalMotionBlurUntil = 0.0f;
		}
		history.previousPositon = currentPosition;

		history.previousPitch = currentPitch;
		history.previousYaw = currentYaw;
		history.lastTimeUpdate = m_globals()->m_realtime;
		return;
	}

	const auto material = m_materialsystem()->FindMaterial("dev/motion_blur", "RenderTargets", false);
	if (!material)
		return;

	const auto MotionBlurInternal = material->FindVar("$MotionBlurInternal", nullptr, false);

	MotionBlurInternal->set_vec_component_value(motionBlurValues[0], 0);
	MotionBlurInternal->set_vec_component_value(motionBlurValues[1], 1);
	MotionBlurInternal->set_vec_component_value(motionBlurValues[2], 2);
	MotionBlurInternal->set_vec_component_value(motionBlurValues[3], 3);

	const auto MotionBlurViewPortInternal = material->FindVar("$MotionBlurViewportInternal", nullptr, false);

	MotionBlurViewPortInternal->set_vec_component_value(0.0f, 0);
	MotionBlurViewPortInternal->set_vec_component_value(0.0f, 1);
	MotionBlurViewPortInternal->set_vec_component_value(1.0f, 2);
	MotionBlurViewPortInternal->set_vec_component_value(1.0f, 3);

	const auto drawFunction = memory->drawScreenEffectMaterial;
	int w, h;
	m_engine()->GetScreenSize(w, h);
	__asm {
		__asm push h
		__asm push w
		__asm push 0
		__asm xor edx, edx
		__asm mov ecx, material
		__asm call drawFunction
		__asm add esp, 12
	}
}

void worldesp::molotov_timer(entity_t* entity)
{
	if (!configs.esp.molotov_timer)
		return;

	auto inferno = reinterpret_cast<inferno_t*>(entity);
	auto& origin = inferno->GetAbsOrigin();

	Vector screen_origin;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	auto spawn_time = inferno->get_spawn_time();
	auto factor = (spawn_time + inferno_t::get_expiry_time() - m_globals()->m_curtime) / inferno_t::get_expiry_time();

	static auto size = Vector2D(35.0f, 5.0f);
	render::get().circle_filled(screen_origin.x, screen_origin.y - size.y * 0.5f, 60, 20, Color(15, 15, 15, 187));

	render::get().rect_filled(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f - 1.0f, size.x, size.y, Color(37, 37, 37, configs.esp.molotov_timer_color.a()));
	render::get().rect_filled(screen_origin.x - size.x * 0.5f + 2.0f, screen_origin.y - size.y * 0.5f, (size.x - 4.0f) * factor, size.y - 2.0f, configs.esp.molotov_timer_color);

	render::get().rect(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f, size.x, size.y, Color(7, 7, 7, configs.esp.molotov_timer_color.a()));
	render::get().text(fonts[ESP], screen_origin.x, screen_origin.y - size.y * 0.5f + 12.0f, configs.esp.molotov_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "FIRE");
	render::get().text(fonts[GRENADES], screen_origin.x + 1.0f, screen_origin.y - size.y * 0.5f - 9.0f, configs.esp.molotov_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "l");
}

void worldesp::smoke_timer(entity_t* entity)
{
	if (!configs.esp.smoke_timer)
		return;

	auto smoke = reinterpret_cast<smoke_t*>(entity);

	if (!smoke->m_nSmokeEffectTickBegin() || !smoke->m_bDidSmokeEffect())
		return;

	auto& origin = smoke->GetAbsOrigin();

	Vector screen_origin;

	if (!math::world_to_screen(origin, screen_origin))
		return;

	auto spawn_time = TICKS_TO_TIME(smoke->m_nSmokeEffectTickBegin());
	auto factor = (spawn_time + smoke_t::get_expiry_time() - m_globals()->m_curtime) / smoke_t::get_expiry_time();

	static auto size = Vector2D(35.0f, 5.0f);
	render::get().circle_filled(screen_origin.x, screen_origin.y - size.y * 0.5f, 60, 20, Color(15, 15, 15, 187));

	render::get().rect_filled(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f - 1.0f, size.x, size.y, Color(37, 37, 37, configs.esp.smoke_timer_color.a()));
	render::get().rect_filled(screen_origin.x - size.x * 0.5f + 2.0f, screen_origin.y - size.y * 0.5f, (size.x - 4.0f) * factor, size.y - 2.0f, configs.esp.smoke_timer_color);

	render::get().rect(screen_origin.x - size.x * 0.5f, screen_origin.y - size.y * 0.5f, size.x, size.y, Color(7, 7, 7, configs.esp.smoke_timer_color.a()));
	render::get().text(fonts[ESP], screen_origin.x, screen_origin.y - size.y * 0.5f + 12.0f, configs.esp.smoke_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "SMOKE");
	render::get().text(fonts[GRENADES], screen_origin.x + 1.0f, screen_origin.y - size.y * 0.5f - 9.0f, configs.esp.smoke_timer_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "k");
}

void worldesp::grenade_projectiles(entity_t* entity)
{
	if (!configs.esp.projectiles)
		return;

	auto client_class = entity->GetClientClass();

	if (!client_class)
		return;

	auto model = entity->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto name = std::string(studio_model->szName);

	if (name.find("thrown") != std::string::npos ||
		client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
	{
		auto& grenade_origin = entity->GetAbsOrigin();
		auto grenade_position = ZERO;

		if (!math::world_to_screen(grenade_origin, grenade_position))
			return;

		std::string grenade_name, grenade_icon;

		if (name.find("flashbang") != std::string::npos)
		{
			grenade_name = "FLASHBANG";
			grenade_icon = "i";
		}
		else if (name.find("smokegrenade") != std::string::npos)
		{
			grenade_name = "SMOKE";
			grenade_icon = "k";
		}
		else if (name.find("incendiarygrenade") != std::string::npos)
		{
			grenade_name = "INCENDIARY";
			grenade_icon = "n";
		}
		else if (name.find("molotov") != std::string::npos)
		{
			grenade_name = "MOLOTOV";
			grenade_icon = "l";
		}
		else if (name.find("fraggrenade") != std::string::npos)
		{
			grenade_name = "HE GRENADE";
			grenade_icon = "j";
		}
		else if (name.find("decoy") != std::string::npos)
		{
			grenade_name = "DECOY";
			grenade_icon = "m";
		}
		else
			return;

		Box box;

		if (util::get_bbox(entity, box))
		{
			if (configs.esp.grenade_esp[GRENADE_BOX])
			{
				render::get().rect(box.x, box.y, box.w, box.h, configs.esp.grenade_box_color);

				if (configs.esp.grenade_esp[GRENADE_ICON])
					render::get().text(fonts[GRENADES], box.x + box.w / 2, box.y - 21, configs.esp.projectiles_color, HFONT_CENTERED_X, grenade_icon.c_str());

				if (configs.esp.grenade_esp[GRENADE_TEXT])
					render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, configs.esp.projectiles_color, HFONT_CENTERED_X, grenade_name.c_str());
			}
			else
			{
				if (configs.esp.grenade_esp[GRENADE_ICON] && configs.esp.grenade_esp[GRENADE_TEXT])
				{
					render::get().text(fonts[GRENADES], box.x + box.w / 2, box.y + box.h / 2 - 10, configs.esp.projectiles_color, HFONT_CENTERED_X, grenade_icon.c_str());
					render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 7, configs.esp.projectiles_color, HFONT_CENTERED_X, grenade_name.c_str());
				}
				else
				{
					if (configs.esp.grenade_esp[GRENADE_ICON])
						render::get().text(fonts[GRENADES], box.x + box.w / 2, box.y + box.h / 2, configs.esp.projectiles_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, grenade_icon.c_str());

					if (configs.esp.grenade_esp[GRENADE_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2, configs.esp.projectiles_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, grenade_name.c_str());
				}
			}
		}
	}
	else
	{
		auto model = entity->GetModel();

		if (!model)
			return;

		auto studio_model = m_modelinfo()->GetStudioModel(model);

		if (!studio_model)
			return;

		auto name = std::string(studio_model->szName);

		if (name.find("dropped") != std::string::npos)
		{
			auto weapon = reinterpret_cast<weapon_t*>(entity);
			Box box;

			if (util::get_bbox(weapon, box))
			{
				auto offset = 0;

				if (configs.esp.weapon[WEAPON_BOX])
				{
					render::get().rect(box.x, box.y, box.w, box.h, configs.esp.box_color);

					if (configs.esp.weapon[WEAPON_ICON])
					{
						render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y - 14, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());
						offset = 14;
					}

					if (configs.esp.weapon[WEAPON_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

					if (configs.esp.weapon[WEAPON_DISTANCE])
					{
						auto distance = cheat.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y - 13 - offset, configs.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
					}
				}
				else
				{
					if (configs.esp.weapon[WEAPON_ICON])
						render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y + box.h / 2 - 7, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());

					if (configs.esp.weapon[WEAPON_TEXT])
						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 6, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

					if (configs.esp.weapon[WEAPON_DISTANCE])
					{
						auto distance = cheat.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

						if (configs.esp.weapon[WEAPON_ICON] && configs.esp.weapon[WEAPON_TEXT])
							offset = 21;
						else if (configs.esp.weapon[WEAPON_ICON])
							offset = 21;
						else if (configs.esp.weapon[WEAPON_TEXT])
							offset = 8;

						render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 - offset, configs.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
					}
				}
			}
		}
	}
}

void worldesp::bomb_timer(entity_t* entity)
{
	if (!configs.esp.bomb_timer)
		return;

	if (!cheat.globals.bomb_timer_enable)
		return;

	static auto mp_c4timer = m_cvar()->FindVar(crypt_str("mp_c4timer"));
	auto bomb = reinterpret_cast<CCSBomb*>(entity);

	auto c4timer = mp_c4timer->GetFloat();
	auto bomb_timer = bomb->m_flC4Blow() - m_globals()->m_curtime;

	if (bomb_timer < 0.0f)
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto factor = bomb_timer / c4timer * height;

	auto red_factor = (int)(255.0f - bomb_timer / c4timer * 255.0f);
	auto green_factor = (int)(bomb_timer / c4timer * 255.0f);

	render::get().rect_filled(0, height - factor, 26, factor, Color(red_factor, green_factor, 0, 100));

	auto text_position = height - factor + 11;

	if (text_position > height - 9)
		text_position = height - 9;

	render::get().text(fonts[ESP], 13, text_position, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, "%0.1f", bomb_timer);

	Vector screen;

	if (math::world_to_screen(entity->GetAbsOrigin(), screen))
		render::get().text(fonts[ESP], screen.x, screen.y, Color(red_factor, green_factor, 0), HFONT_CENTERED_X | HFONT_CENTERED_Y, "BOMB");
}

void worldesp::dropped_weapons(entity_t* entity)
{
	auto weapon = reinterpret_cast<weapon_t*>(entity);
	auto owner = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(weapon->m_hOwnerEntity()));

	if (!owner)
		return;

	if (owner->is_player())
		return;

	Box box;

	if (util::get_bbox(weapon, box))
	{
		auto offset = 0;

		if (configs.esp.weapon[WEAPON_BOX])
		{
			render::get().rect(box.x, box.y, box.w, box.h, configs.esp.box_color);

			if (configs.esp.weapon[WEAPON_ICON])
			{
				render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y - 14, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());
				offset = 14;
			}

			if (configs.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h + 2, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (weapon->get_weapon_info() && configs.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h + 14, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h + 15, weapon->m_iClip() * box.w / weapon->get_weapon_info()->iMaxClip1, 2, configs.esp.weapon_ammo_color);
			}

			if (configs.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = cheat.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y - 13 - offset, configs.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
			}
		}
		else
		{
			if (configs.esp.weapon[WEAPON_ICON])
				render::get().text(fonts[SUBTABWEAPONS], box.x + box.w / 2, box.y + box.h / 2 - 7, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_icon());

			if (configs.esp.weapon[WEAPON_TEXT])
				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 + 6, configs.esp.weapon_color, HFONT_CENTERED_X, weapon->get_name().c_str());

			if (configs.esp.weapon[WEAPON_AMMO] && entity->GetClientClass()->m_ClassID != CBaseCSGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSmokeGrenadeProjectile && entity->GetClientClass()->m_ClassID != CSensorGrenadeProjectile && entity->GetClientClass()->m_ClassID != CMolotovProjectile && entity->GetClientClass()->m_ClassID != CDecoyProjectile)
			{
				static auto pos = 0;

				if (configs.esp.weapon[WEAPON_ICON] && configs.esp.weapon[WEAPON_TEXT])
					pos = 19;
				else if (configs.esp.weapon[WEAPON_ICON])
					pos = 8;
				else if (configs.esp.weapon[WEAPON_TEXT])
					pos = 19;

				auto inner_back_color = Color::Black;
				inner_back_color.SetAlpha(153);

				render::get().rect_filled(box.x - 1, box.y + box.h / 2 + pos - 1, box.w + 2, 4, inner_back_color);
				render::get().rect_filled(box.x, box.y + box.h / 2 + pos, weapon->m_iClip() * box.w / weapon->get_weapon_info()->iMaxClip1, 2, configs.esp.weapon_ammo_color);
			}

			if (configs.esp.weapon[WEAPON_DISTANCE])
			{
				auto distance = cheat.local()->GetAbsOrigin().DistTo(weapon->GetAbsOrigin()) / 12.0f;

				if (configs.esp.weapon[WEAPON_ICON] && configs.esp.weapon[WEAPON_TEXT])
					offset = 21;
				else if (configs.esp.weapon[WEAPON_ICON])
					offset = 21;
				else if (configs.esp.weapon[WEAPON_TEXT])
					offset = 8;

				render::get().text(fonts[ESP], box.x + box.w / 2, box.y + box.h / 2 - offset, configs.esp.weapon_color, HFONT_CENTERED_X, "%i FT", (int)distance);
			}
		}
	}
}