#include "..\hooks.hpp"
#include "..\..\features\legitbot\backtrack.h"
#include "..\..\features\serverside\animation_system.h"
#include "..\..\features\serverside\engine_prediction.h"
#include "..\..\features\visuals\night_mode.h"
#include "..\..\features\misc\misc.h"
#include "..\..\features\visuals\world_esp.h"
#include "..\..\features\misc\logs.h"
#include "..\..\features\serverside\local_animations.h"
#include "..\..\sdk\BBData.h"

Vector backup_aim_punch;
Vector backup_view_punch;

Vector* aim_punch;
Vector* view_punch;

void weather()
{
	static ClientClass* client_class = nullptr;

	if (!client_class)
		client_class = m_client()->GetAllClasses();

	while (client_class)
	{
		if (client_class->m_ClassID == CPrecipitation)
			break;

		client_class = client_class->m_pNext;
	}

	if (!client_class)
		return;

	auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
	auto serial = math::random_int(0, 4095);

	cheat.globals.m_networkable = client_class->m_pCreateFn(entry, serial);

	if (!cheat.globals.m_networkable)
		return;

	auto m_precipitation = cheat.globals.m_networkable->GetIClientUnknown()->GetBaseEntity();

	if (!m_precipitation)
		return;

	cheat.globals.m_networkable->PreDataUpdate(0);
	cheat.globals.m_networkable->OnPreDataChanged(0);

	static auto m_nPrecipType = netvars::get().get_offset(crypt_str("CPrecipitation"), crypt_str("m_nPrecipType"));
	static auto m_vecMins = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMins"));
	static auto m_vecMaxs = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_vecMaxs"));

	*(int*)(uintptr_t(m_precipitation) + m_nPrecipType) = 0;
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMaxs) = Vector(32768.0f, 32768.0f, 32768.0f);
	*(Vector*)(uintptr_t(m_precipitation) + m_vecMins) = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->GetCollideable()->OBBMaxs() = Vector(32768.0f, 32768.0f, 32768.0f);
	m_precipitation->GetCollideable()->OBBMins() = Vector(-32768.0f, -32768.0f, -32768.0f);

	m_precipitation->set_abs_origin((m_precipitation->GetCollideable()->OBBMins() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f);
	m_precipitation->m_vecOrigin() = (m_precipitation->GetCollideable()->OBBMaxs() + m_precipitation->GetCollideable()->OBBMins()) * 0.5f;

	m_precipitation->OnDataChanged(0);
	m_precipitation->PostDataUpdate(0);
}

void remove_smoke()
{
	if (configs.esp.enabled && configs.visuals.removals[REMOVALS_SMOKE])
	{
		static auto smoke_count = *reinterpret_cast<uint32_t**>(util::FindSignature(crypt_str("client.dll"), crypt_str("A3 ? ? ? ? 57 8B CB")) + 0x1);
		*(int*)smoke_count = 0;
	}

	if (cheat.globals.should_remove_smoke == configs.esp.enabled && configs.visuals.removals[REMOVALS_SMOKE])
		return;

	cheat.globals.should_remove_smoke = configs.esp.enabled && configs.visuals.removals[REMOVALS_SMOKE];

	static std::vector<const char*> smoke_materials =
	{
		"effects/overlaysmoke",
		"particle/beam_smoke_01",
		"particle/particle_smokegrenade",
		"particle/particle_smokegrenade1",
		"particle/particle_smokegrenade2",
		"particle/particle_smokegrenade3",
		"particle/particle_smokegrenade_sc",
		"particle/smoke1/smoke1",
		"particle/smoke1/smoke1_ash",
		"particle/smoke1/smoke1_nearcull",
		"particle/smoke1/smoke1_nearcull2",
		"particle/smoke1/smoke1_snow",
		"particle/smokesprites_0001",
		"particle/smokestack",
		"particle/vistasmokev1/vistasmokev1",
		"particle/vistasmokev1/vistasmokev1_emods",
		"particle/vistasmokev1/vistasmokev1_emods_impactdust",
		"particle/vistasmokev1/vistasmokev1_fire",
		"particle/vistasmokev1/vistasmokev1_nearcull",
		"particle/vistasmokev1/vistasmokev1_nearcull_fog",
		"particle/vistasmokev1/vistasmokev1_nearcull_nodepth",
		"particle/vistasmokev1/vistasmokev1_smokegrenade",
		"particle/vistasmokev1/vistasmokev4_emods_nocull",
		"particle/vistasmokev1/vistasmokev4_nearcull",
		"particle/vistasmokev1/vistasmokev4_nocull"
	};

	for (auto material_name : smoke_materials)
	{
		auto material = m_materialsystem2()->FindMaterial(material_name, nullptr);

		if (!material)
			continue;

		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, cheat.globals.should_remove_smoke);
	}
}

void remove_molotov()
{
	if (cheat.globals.should_remove_molotov == configs.esp.enabled && configs.visuals.removals[REMOVALS_MOLOTOV])
		return;

	cheat.globals.should_remove_molotov = configs.esp.enabled && configs.visuals.removals[REMOVALS_MOLOTOV];

	static std::vector<const char*> molotov_materials =
	{
		"decals/molotovscorch.vmt",
		"particle/fire_burning_character/fire_env_fire.vmt",
		"particle/fire_burning_character/fire_env_fire_depthblend.vmt",
		"particle/particle_flares/particle_flare_001.vmt",
		"particle/particle_flares/particle_flare_004.vmt",
		"particle/particle_flares/particle_flare_004b_mod_ob.vmt",
		"particle/particle_flares/particle_flare_004b_mod_z.vmt",
		"particle/fire_explosion_1/fire_explosion_1_bright.vmt",
		"particle/fire_explosion_1/fire_explosion_1b.vmt",
		"particle/fire_particle_4/fire_particle_4.vmt",
		"particle/fire_explosion_1/fire_explosion_1_oriented.vmt",
		"particle/vistasmokev1/vistasmokev1_nearcull_nodepth.vmt"
	};

	for (const auto material_name : molotov_materials) {
		const auto material = m_materialsystem2()->FindMaterial(material_name);
		material->SetMaterialVarFlag(MATERIAL_VAR_NO_DRAW, cheat.globals.should_remove_molotov);
	}
}

void __stdcall Hooks::hooked_fsn(ClientFrameStage_t stage) noexcept
{
	// animation_system::get().init();

	if (stage == FRAME_RENDER_START)
	{
		misc::get().fake_prime();
		misc::get().color_console();
	}

	if (stage == FRAME_START)
	{
		if (localPlayer && configs.profileChanger.musickit)
			m_playerresource()->m_nMusicID(localPlayer->EntIndex()) = configs.profileChanger.musickit;

		BBData::update();
		key_binds::get().update_key_binds();
	}

	if (!localPlayer)
	{
		nightmode::get().clear_stored_materials();
		return client_hook.callOriginal<void, 37>(stage);
	}

	misc::get().draw_server_hitboxes();

	aim_punch = nullptr;
	view_punch = nullptr;

	backup_aim_punch.Zero();
	backup_view_punch.Zero();

	if (cheat.globals.updating_skins && m_clientstate()->m_iDeltaTick > 0)
		cheat.globals.updating_skins = false;

	SkinChanger::run(stage);
	local_animations::get().run(stage);

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START && localPlayer->is_alive())
	{
		/*
		static auto old_next_cmd_time = 0.0f;
		static auto old_last_command_ack = 0;

		auto last_command_ack = m_clientstate()->nLastCommandAck;
		auto next_cmd_time = m_clientstate()->flNextCmdTime;

		if (old_next_cmd_time != next_cmd_time || old_last_command_ack != last_command_ack)
		{
			auto velocity_modifier = localPlayer->m_flVelocityModifier();

			if (cheat.globals.last_velocity_modifier != velocity_modifier)
			{
				m_prediction()->m_bPreviousAckHadErrors = true;

				cheat.globals.last_velocity_modifier = velocity_modifier;
				cheat.globals.last_velocity_modifier_tick = m_clientstate()->iCommandAck;
			}

			old_next_cmd_time = next_cmd_time;
			old_last_command_ack = last_command_ack;
		}

		auto viewmodel = localPlayer->m_hViewModel().Get();

		if (viewmodel && engineprediction::get().viewmodel_data.weapon == viewmodel->m_hWeapon().Get() && engineprediction::get().viewmodel_data.sequence == viewmodel->m_nSequence() && engineprediction::get().viewmodel_data.animation_parity == viewmodel->m_nAnimationParity())
		{
			viewmodel->m_flCycle() = engineprediction::get().viewmodel_data.cycle;
			viewmodel->m_flAnimTime() = engineprediction::get().viewmodel_data.animation_time;
		}
				*/
	}

	if (stage == FRAME_RENDER_START)
	{
		if (configs.visuals.client_bullet_impacts)
		{
			static auto last_count = 0;
			auto& client_impact_list = *reinterpret_cast<SafeCUtlVector<client_hit_verify_t>*>(uintptr_t(localPlayer.get()) + 0x11C50);

			for (auto i = client_impact_list.count(); i > last_count; --i)
				m_debugoverlay()->AddBoxOverlay(client_impact_list[i - 1].position, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f), Vector(0.0f, 0.0f, 0.0f), configs.visuals.client_bullet_impacts_color.r(), configs.visuals.client_bullet_impacts_color.g(), configs.visuals.client_bullet_impacts_color.b(), configs.visuals.client_bullet_impacts_color.a(), 4.0f);

			if (client_impact_list.count() != last_count)
				last_count = client_impact_list.count();
		}

		misc::get().fix_animation_lod();
		misc::get().force_relay_cluster();
		backtrack::get().update();
		remove_smoke();
		remove_molotov();
		misc::get().preserve_killfeed();
		misc::get().ragdolls();
		misc::get().anti_crasher();
		misc::get().anti_exploit();
		misc::get().color_console();

		if (configs.visuals.removals[REMOVALS_FLASH] && localPlayer->m_flFlashDuration() && configs.esp.enabled)
			localPlayer->m_flFlashDuration() = 0.0f;

		if (*memory->disablePostProcessing != (configs.esp.enabled && configs.visuals.removals[REMOVALS_POSTPROCESSING] && (!configs.visuals.world_modulation || !configs.visuals.exposure)))
			*memory->disablePostProcessing = configs.esp.enabled && configs.visuals.removals[REMOVALS_POSTPROCESSING] && (!configs.visuals.world_modulation || !configs.visuals.exposure);

		if (configs.visuals.removals[REMOVALS_RECOIL] && configs.esp.enabled)
		{
			aim_punch = &localPlayer->m_aimPunchAngle();
			view_punch = &localPlayer->m_viewPunchAngle();

			backup_aim_punch = *aim_punch;
			backup_view_punch = *view_punch;

			(*aim_punch).Zero();
			(*view_punch).Zero();
		}

		auto get_original_scope = false;

		if (localPlayer->is_alive())
		{
			cheat.globals.in_thirdperson = key_binds::get().get_key_bind_state(key_bind_features::THIRD_PERSON);

			if (configs.esp.enabled && configs.visuals.removals[REMOVALS_SCOPE])
			{
				auto weapon = localPlayer->getActiveWeapon();

				if (weapon)
				{
					get_original_scope = true;

					cheat.globals.scoped = localPlayer->m_bIsScoped() && weapon->m_zoomLevel();
					localPlayer->m_bIsScoped() = weapon->m_zoomLevel();
				}
			}
		}

		if (!get_original_scope)
			cheat.globals.scoped = localPlayer->m_bIsScoped();
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		static auto rain = false;

		if (rain != configs.visuals.rain || cheat.globals.should_update_weather)
		{
			rain = configs.visuals.rain;

			if (cheat.globals.m_networkable)
			{
				cheat.globals.m_networkable->Release();
				cheat.globals.m_networkable = nullptr;
			}

			if (rain)
				weather();

			cheat.globals.should_update_weather = false;
		}

		configs.player_list.refreshing = true;
		configs.player_list.players.clear();

		for (auto i = 1; i < 65; i++)
		{
			auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

			if (!e)
			{
				configs.player_list.white_list[i] = false;
				configs.player_list.high_priority[i] = false;
				configs.player_list.force_body_aim[i] = false;

				continue;
			}

			if (e->m_iTeamNum() == localPlayer->m_iTeamNum())
			{
				configs.player_list.white_list[i] = false;
				configs.player_list.high_priority[i] = false;
				configs.player_list.force_body_aim[i] = false;

				continue;
			}

			PlayerInfo player_info;
			m_engine()->GetPlayerInfo(i, &player_info);

			configs.player_list.players.emplace_back(Player_list_data(i, player_info.name));
		}

		configs.player_list.refreshing = false;
	}

	if (stage == FRAME_RENDER_END)
	{
		//static auto r_drawspecificstaticprop = m_cvar2()->FindVar(crypt_str("r_DrawSpecificStaticProp"));

		//if (r_drawspecificstaticprop->GetInt() != 0)
		//	r_drawspecificstaticprop->SetValue(0);

		if (cheat.globals.change_materials)
		{
			if (configs.visuals.nightmode && configs.esp.enabled)
				nightmode::get().apply();
			else
				nightmode::get().remove();

			cheat.globals.change_materials = false;
		}

		worldesp::get().skybox_changer();
		worldesp::get().fog_changer();

		misc::get().full_bright();
		misc::get().view_model();

		//static auto cl_foot_contact_shadows = m_cvar()->FindVar(crypt_str("cl_foot_contact_shadows"));
		//if (cl_foot_contact_shadows->GetBool())
		//	cl_foot_contact_shadows->SetValue(false);

		static auto zoom_sensitivity_ratio_mouse = m_cvar2()->FindVar(crypt_str("zoom_sensitivity_ratio_mouse"));

		if (configs.esp.enabled && configs.visuals.removals[REMOVALS_ZOOM] && configs.visuals.fix_zoom_sensivity && zoom_sensitivity_ratio_mouse->GetFloat() == 1.0f)
			zoom_sensitivity_ratio_mouse->SetValue(0);
		else if ((!configs.esp.enabled || !configs.visuals.removals[REMOVALS_ZOOM] || !configs.visuals.fix_zoom_sensivity) && !zoom_sensitivity_ratio_mouse->GetFloat())
			zoom_sensitivity_ratio_mouse->SetValue(1);

		static auto r_modelAmbientMin = m_cvar2()->FindVar(crypt_str("r_modelAmbientMin"));

		if (configs.visuals.world_modulation && configs.visuals.ambient && r_modelAmbientMin->GetFloat() != configs.visuals.ambient * 0.05f)
			r_modelAmbientMin->SetValue(configs.visuals.ambient * 0.05f);
		else if ((!configs.visuals.world_modulation || !configs.visuals.ambient) && r_modelAmbientMin->GetFloat())
			r_modelAmbientMin->SetValue(0);
	}

	if (stage == FRAME_NET_UPDATE_END)
	{
		auto current_shot = cheat.shots.end();

		auto net_channel = m_engine()->GetNetChannelInfo();
		auto latency = net_channel ? net_channel->GetLatency(FLOW_OUTGOING) + net_channel->GetLatency(FLOW_INCOMING) + 1.0f : 0.0f;

		for (auto shot = cheat.shots.begin(); shot != cheat.shots.end(); ++shot)
		{
			if (shot->end)
			{
				current_shot = shot;
				break;
			}
			else if (shot->impacts && m_globals()->m_tickcount - 1 > shot->event_fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				break;
			}
			else if (cheat.globals.backup_tickbase - TIME_TO_TICKS(latency) > shot->fire_tick)
			{
				current_shot = shot;
				current_shot->end = true;
				current_shot->latency = true;
				break;
			}
		}

		if (current_shot != cheat.shots.end())
		{
			if (!current_shot->latency)
			{
				current_shot->shot_info.should_log = true;

				if (!current_shot->hurt_player)
				{
					misc::get().lag_hitbox();

					if (current_shot->impact_hit_player)
					{
						current_shot->shot_info.result = crypt_str("desync");

						cheat.globals.missed_shots[current_shot->last_target]++;
						animation_system::get().player_resolver[current_shot->last_target].random_number = math::random_int(-3, 3);
						animation_system::get().player_resolver[current_shot->last_target].last_side = static_cast<ResolverSide>(current_shot->side);
						animation_system::get().player_resolver[current_shot->last_target].backup_side = ResolverSide::RESOLVER_ORIGINAL;

						if (configs.misc.events_to_log[EVENTLOG_HIT])
							eventlogs::get().add(crypt_str("Missed"), crypt_str("Missed shot due to desync"), true);
					}
					else if (current_shot->occlusion)
					{
						current_shot->shot_info.result = crypt_str("occlusion");
						if (configs.misc.events_to_log[EVENTLOG_HIT])
							eventlogs::get().add(crypt_str("Missed"), crypt_str("Missed shot due to occlusion"), true);
					}
					else if (current_shot->shot_info.hitchance > 100)
					{
						current_shot->shot_info.result = crypt_str("prediction");
						if (configs.misc.events_to_log[EVENTLOG_HIT])
							eventlogs::get().add(crypt_str("Missed"), crypt_str("Missed shot due to prediction error"), true);
					}
					else if (current_shot->shot_info.hitchance == 100)
					{
						current_shot->shot_info.result = crypt_str("unknown");
						if (configs.misc.events_to_log[EVENTLOG_HIT])
							eventlogs::get().add(crypt_str("Missed"), crypt_str("Missed shot due to unknown"), true);
					}
					else
					{
						current_shot->shot_info.result = crypt_str("spread");
						if (configs.misc.events_to_log[EVENTLOG_HIT])
							eventlogs::get().add(crypt_str("Missed"), crypt_str("Missed shot due to spread"), true);
					}
				}
			}

			if (configs.ragebot.adaptive_resolver && current_shot->shot_info.result != crypt_str("desync") && (current_shot->shot_info.server_hitbox == "Head" || !configs.ragebot.adaptive_resolver_ignore_body))
				animation_system::get().player_resolver[current_shot->last_target].backup_side = static_cast<ResolverSide>(current_shot->side);

			if (cheat.globals.loaded_script && current_shot->shot_info.should_log)
			{
				current_shot->shot_info.should_log = false;

				for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_shot")))
					current.func(current_shot->shot_info);
			}

			cheat.shots.erase(current_shot);
		}
	}
	
	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_framestage")))
			current.func(stage);

	animation_system::get().run(stage);
	client_hook.callOriginal<void, 37>(stage);

	engineprediction::get().restore_netvars();

	if (configs.esp.enabled && configs.visuals.removals[REMOVALS_RECOIL] && localPlayer->is_alive() && aim_punch && view_punch)
	{
		*aim_punch = backup_aim_punch;
		*view_punch = backup_view_punch;
	}
}
