#include "..\hooks.hpp"
#include "..\..\features\ragebot\ragebot.h"
#include "..\..\features\misc\logs.h"
#include "..\..\features\visuals\other_esp.h"
#include "..\..\features\visuals\bullet_tracers.h"
#include "..\..\features\visuals\player_esp.h"
#include "..\..\features\misc\misc.h"
#include "..\..\features\visuals\dormant_esp.h"
#include "..\..\features\movement\walkbot.h"

bool weapon_is_aim(const std::string& weapon)
{
	return weapon.find(crypt_str("decoy")) == std::string::npos && weapon.find(crypt_str("flashbang")) == std::string::npos &&
		weapon.find(crypt_str("hegrenade")) == std::string::npos && weapon.find(crypt_str("inferno")) == std::string::npos &&
		weapon.find(crypt_str("molotov")) == std::string::npos && weapon.find(crypt_str("smokegrenade")) == std::string::npos;
}

void C_HookedEvents::FireGameEvent(IGameEvent* event)
{
	auto event_name = event->GetName();

	if (cheat.globals.loaded_script)
	{
		for (auto& script : c_lua::get().scripts)
		{
			auto script_id = c_lua::get().get_script_id(script);

			if (c_lua::get().events.find(script_id) == c_lua::get().events.end())
				continue;

			if (c_lua::get().events[script_id].find(event_name) == c_lua::get().events[script_id].end())
				continue;

			c_lua::get().events[script_id][event_name](event);
		}
	}

	if (!strcmp(event->GetName(), crypt_str("player_footstep")))
	{
		auto userid = m_engine()->GetPlayerForUserID(event->GetInt(crypt_str("userid")));
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(userid));

		if (e->valid(false, false))
		{
			auto type = ENEMY;

			if (e == cheat.local())
				type = LOCAL;
			else if (e->m_iTeamNum() == cheat.local()->m_iTeamNum())
				type = TEAM;

			if (configs.player.type[type].footsteps)
			{
				static auto model_index = m_modelinfo()->GetModelIndex(crypt_str("sprites/physbeam.vmt"));

				if (cheat.globals.should_update_beam_index)
					model_index = m_modelinfo()->GetModelIndex(crypt_str("sprites/physbeam.vmt"));

				BeamInfo_t info;

				info.m_nType = TE_BEAMRINGPOINT;
				info.m_pszModelName = crypt_str("sprites/physbeam.vmt");
				info.m_nModelIndex = model_index;
				info.m_nHaloIndex = -1;
				info.m_flHaloScale = 3.0f;
				info.m_flLife = 2.0f;
				info.m_flWidth = (float)configs.player.type[type].thickness;
				info.m_flFadeLength = 1.0f;
				info.m_flAmplitude = 0.0f;
				info.m_flRed = (float)configs.player.type[type].footsteps_color.r();
				info.m_flGreen = (float)configs.player.type[type].footsteps_color.g();
				info.m_flBlue = (float)configs.player.type[type].footsteps_color.b();
				info.m_flBrightness = (float)configs.player.type[type].footsteps_color.a();
				info.m_flSpeed = 0.0f;
				info.m_nStartFrame = 0.0f;
				info.m_flFrameRate = 60.0f;
				info.m_nSegments = -1;
				info.m_nFlags = FBEAM_FADEOUT;
				info.m_vecCenter = e->GetAbsOrigin() + Vector(0.0f, 0.0f, 5.0f);
				info.m_flStartRadius = 5.0f;
				info.m_flEndRadius = (float)configs.player.type[type].radius;
				info.m_bRenderable = true;

				auto beam_draw = m_viewrenderbeams()->CreateBeamRingPoint(info);

				if (beam_draw)
					m_viewrenderbeams()->DrawBeam(beam_draw);
			}
		}

	}
	else if (!strcmp(event_name, crypt_str("weapon_fire")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id);

		if (user == m_engine()->GetLocalPlayer())
		{
			aim_shot* current_shot = nullptr;

			for (auto& shot : cheat.shots)
			{
				if (shot.start)
				{
					shot.end = true;
					continue;
				}

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (current_shot)
			{
				current_shot->start = true;
				current_shot->event_fire_tick = m_globals()->m_tickcount;

				++cheat.globals.fired_shots[ragebot::get().last_target[current_shot->last_target].record.index];
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("bullet_impact")))
	{
		auto user_id = event->GetInt(crypt_str("userid"));
		auto user = m_engine()->GetPlayerForUserID(user_id);

		if (user == m_engine()->GetLocalPlayer())
		{
			Vector position(event->GetFloat(crypt_str("x")), event->GetFloat(crypt_str("y")), event->GetFloat(crypt_str("z")));

			if (configs.esp.server_bullet_impacts)
				m_debugoverlay()->AddBoxOverlay(position, Vector(-2.0f, -2.0f, -2.0f), Vector(2.0f, 2.0f, 2.0f), Vector(0.0f, 0.0f, 0.0f), configs.esp.server_bullet_impacts_color.r(), configs.esp.server_bullet_impacts_color.g(), configs.esp.server_bullet_impacts_color.b(), configs.esp.server_bullet_impacts_color.a(), 4.0f);

			aim_shot* current_shot = nullptr;

			for (auto& shot : cheat.shots)
			{
				if (!shot.start)
					continue;

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (current_shot && ragebot::get().last_target[current_shot->last_target].record.player->valid(true, false))
			{
				auto backup_data = adjust_data(ragebot::get().last_target[current_shot->last_target].record.player);
				ragebot::get().last_target[current_shot->last_target].record.adjust_player();

				CGameTrace trace, trace_zero, trace_first, trace_second;
				Ray_t ray;

				ray.Init(ragebot::get().last_shoot_position, position);
				m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ragebot::get().last_target[current_shot->last_target].record.player, &trace);

				if (ragebot::get().last_target[current_shot->last_target].data.point.safe)
				{
					memcpy(ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), ragebot::get().last_target[current_shot->last_target].record.matrixes_data.zero, ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ragebot::get().last_target[current_shot->last_target].record.player, &trace_zero);

					memcpy(ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), ragebot::get().last_target[current_shot->last_target].record.matrixes_data.first, ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ragebot::get().last_target[current_shot->last_target].record.player, &trace_first);

					memcpy(ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Base(), ragebot::get().last_target[current_shot->last_target].record.matrixes_data.second, ragebot::get().last_target[current_shot->last_target].record.player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));
					m_trace()->ClipRayToEntity(ray, MASK_SHOT_HULL | CONTENTS_HITBOX, ragebot::get().last_target[current_shot->last_target].record.player, &trace_second);
				}

				auto hit = trace.entity == ragebot::get().last_target[current_shot->last_target].record.player;

				if (ragebot::get().last_target[current_shot->last_target].data.point.safe)
					hit = (hit && trace_zero.entity == ragebot::get().last_target[current_shot->last_target].record.player && trace_first.entity == ragebot::get().last_target[current_shot->last_target].record.player && trace_second.entity == ragebot::get().last_target[current_shot->last_target].record.player);

				if (hit)
					current_shot->impact_hit_player = true;
				else 
					current_shot->occlusion = (ragebot::get().last_shoot_position.DistTo(position) < ragebot::get().last_target[current_shot->last_target].distance);

				current_shot->impacts = true;
				backup_data.adjust_player();
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("player_death")))
	{
		auto attacker = event->GetInt(crypt_str("attacker"));
		auto user = event->GetInt(crypt_str("userid"));

		auto attacker_id = m_engine()->GetPlayerForUserID(attacker);
		auto user_id = m_engine()->GetPlayerForUserID(user);

		if (user_id == m_engine()->GetLocalPlayer())
			walkbot::get().marker = walkbot::get().prishel = 0;
		if (cheat.local() && cheat.local()->is_alive() && attacker_id == m_engine()->GetLocalPlayer() && user_id != m_engine()->GetLocalPlayer())
		{
			auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(user_id));

			if (configs.esp.lightning_on_hit && configs.esp.lightning_on_hit_killonly)
			{
				auto tesla_info = tesla_info_t{};
				tesla_info.m_flbeamwidth = configs.esp.lightning_on_hit_width;
				tesla_info.m_flradius = configs.esp.lightning_on_hit_radius;
				tesla_info.m_entindex = user_id;
				tesla_info.m_color = Vector(1, 1, 1);
				tesla_info.m_pos = entity->hitbox_position(HITBOX_UPPER_CHEST);
				tesla_info.m_ang = ZERO;
				tesla_info.m_fltimevis = 0.75f;
				tesla_info.m_nbeams = configs.esp.lightning_on_hit_beams;
				tesla_info.m_spritename = "sprites/physbeam.vmt"; // "sprites/purplelaser1.vmt"

				memory->fs_tesla(tesla_info);
			}

			if (configs.player.enabled && configs.esp.kill_effect)
				cheat.local()->m_flHealthShotBoostExpirationTime() = m_globals()->m_curtime + configs.esp.kill_effect_duration;

			if (configs.esp.killsound && configs.player.enabled)
			{
				switch (configs.esp.killsound)
				{
				case 1:
					m_surface()->PlaySound(crypt_str("buttons\\arena_switch_press_02.wav"));
					break;
				case 2:
					m_surface()->PlaySound(crypt_str("cod.wav"));
					break;
				case 3:
					m_surface()->PlaySound(crypt_str("bubble.wav"));
					break;
				case 4:
					m_surface()->PlaySound(crypt_str("stapler.wav"));
					break;
				case 5:
					m_surface()->PlaySound(crypt_str("bell.wav"));
					break;
				case 6:
					m_surface()->PlaySound(crypt_str("flick.wav"));
					break;
				case 7:
					m_surface()->PlaySound(configs.esp.custom_killsound.c_str());
					break;
				}
			}

			cheat.globals.kills++;

			SkinChanger::overrideHudIcon(event);
			SkinChanger::updateStatTrak(event);
			misc::get().killfeed_changer(*event);
		}
	}
	else if (!strcmp(event_name, crypt_str("player_hurt")))
	{
		auto attacker = event->GetInt(crypt_str("attacker"));
		auto user = event->GetInt(crypt_str("userid"));

		auto attacker_id = m_engine()->GetPlayerForUserID(attacker);
		auto user_id = m_engine()->GetPlayerForUserID(user);

		if (attacker_id == m_engine()->GetLocalPlayer() && user_id != m_engine()->GetLocalPlayer())
		{
			auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(user_id));
			
			if (configs.esp.lightning_on_hit && !configs.esp.lightning_on_hit_killonly)
			{
				auto tesla_info = tesla_info_t{};
				tesla_info.m_flbeamwidth = configs.esp.lightning_on_hit_width;
				tesla_info.m_flradius = configs.esp.lightning_on_hit_radius;
				tesla_info.m_entindex = user_id;
				tesla_info.m_color = Vector(1, 1, 1);
				tesla_info.m_pos = entity->hitbox_position(HITBOX_UPPER_CHEST);
				tesla_info.m_ang = ZERO;
				tesla_info.m_fltimevis = 0.75f;
				tesla_info.m_nbeams = configs.esp.lightning_on_hit_beams;
				tesla_info.m_spritename = "sprites/physbeam.vmt"; // "sprites/purplelaser1.vmt"

				memory->fs_tesla(tesla_info);
			}

			if (configs.esp.hitsound && configs.player.enabled)
			{
				switch (configs.esp.hitsound)
				{
				case 1:
					m_surface()->PlaySound(crypt_str("buttons\\arena_switch_press_02.wav"));
					break;
				case 2:
					m_surface()->PlaySound(crypt_str("cod.wav"));
					break;
				case 3:
					m_surface()->PlaySound(crypt_str("bubble.wav"));
					break;
				case 4:
					m_surface()->PlaySound(crypt_str("stapler.wav"));
					break;
				case 5:
					m_surface()->PlaySound(crypt_str("bell.wav"));
					break;
				case 6:
					m_surface()->PlaySound(crypt_str("flick.wav"));
					break;
				case 7:
					m_surface()->PlaySound(configs.esp.custom_hitsound.c_str());
					break;
				}
			}

			std::string weapon = event->GetString(crypt_str("weapon"));
			auto damage = event->GetInt(crypt_str("dmg_health"));
			auto hitgroup = event->GetInt(crypt_str("hitgroup"));

			static auto get_hitbox_by_hitgroup = [](int hitgroup) -> int
			{
				switch (hitgroup)
				{
				case HITGROUP_HEAD:
					return HITBOX_HEAD;
				case HITGROUP_CHEST:
					return HITBOX_CHEST;
				case HITGROUP_STOMACH:
					return HITBOX_STOMACH;
				case HITGROUP_LEFTARM:
					return HITBOX_LEFT_HAND;
				case HITGROUP_RIGHTARM:
					return HITBOX_RIGHT_HAND;
				case HITGROUP_LEFTLEG:
					return HITBOX_RIGHT_CALF;
				case HITGROUP_RIGHTLEG:
					return HITBOX_LEFT_CALF;
				default:
					return HITBOX_PELVIS;
				}
			};

			static auto get_hitbox_name = [](int hitbox) -> std::string
			{
				switch (hitbox)
				{
				case HITBOX_HEAD:
					return crypt_str("Head");
				case HITBOX_CHEST:
					return crypt_str("Chest");
				case HITBOX_STOMACH:
					return crypt_str("Stomach");
				case HITBOX_PELVIS:
					return crypt_str("Pelvis");
				case HITBOX_RIGHT_UPPER_ARM:
				case HITBOX_RIGHT_LOWER_ARM:
				case HITBOX_RIGHT_HAND:
					return crypt_str("Left arm");
				case HITBOX_LEFT_UPPER_ARM:
				case HITBOX_LEFT_LOWER_ARM:
				case HITBOX_LEFT_HAND:
					return crypt_str("Right arm");
				case HITBOX_RIGHT_THIGH:
				case HITBOX_RIGHT_CALF:
					return crypt_str("Left leg");
				case HITBOX_LEFT_THIGH:
				case HITBOX_LEFT_CALF:
					return crypt_str("Right leg");
				case HITBOX_RIGHT_FOOT:
					return crypt_str("Left foot");
				case HITBOX_LEFT_FOOT:
					return crypt_str("Right foot");
				}
			};

			aim_shot* current_shot = nullptr;

			for (auto& shot : cheat.shots)
			{
				if (!shot.start)
					continue;

				if (shot.end)
					continue;

				current_shot = &shot;
				break;
			}

			if (weapon_is_aim(weapon))
			{
				if (current_shot && entity == ragebot::get().last_target[current_shot->last_target].record.player)
					misc::get().lag_hitbox();

				otheresp::get().hitmarker.hurt_time = m_globals()->m_curtime;
				otheresp::get().hitmarker.point = entity->hitbox_position_matrix(get_hitbox_by_hitgroup(hitgroup), current_shot && entity == ragebot::get().last_target[current_shot->last_target].record.player ? ragebot::get().last_target[current_shot->last_target].record.matrixes_data.main : entity->m_CachedBoneData().Base());
				otheresp::get().damage_marker[user_id] = otheresp::Damage_marker
				{
					entity->hitbox_position_matrix(get_hitbox_by_hitgroup(hitgroup), current_shot && entity == ragebot::get().last_target[current_shot->last_target].record.player ? ragebot::get().last_target[current_shot->last_target].record.matrixes_data.main : entity->m_CachedBoneData().Base()),
					m_globals()->m_curtime,
					Color::White,
					damage,
					hitgroup
				};
			}
			
			auto headshot = hitgroup == HITGROUP_HEAD;
			auto health = entity->m_iHealth() - damage;

			if (health <= 0 && headshot)
				otheresp::get().hitmarker.hurt_color = Color::Red;
			else if (health <= 0)
				otheresp::get().hitmarker.hurt_color = Color::Yellow;
			else
				otheresp::get().hitmarker.hurt_color = Color::White;

			otheresp::get().damage_marker[user_id].hurt_color = headshot ? Color::Red : Color::White;

			if (current_shot)
			{
				current_shot->shot_info.result = crypt_str("hit");
				current_shot->shot_info.server_hitbox = get_hitbox_name(get_hitbox_by_hitgroup(hitgroup));
				current_shot->shot_info.server_damage = damage;

				current_shot->end = true;
				current_shot->hurt_player = true;
			}
		}
	}
	else if (!strcmp(event_name, crypt_str("round_start")))
	{
		//if (!configs.misc.show_default_log && eventlogs::get().last_log && configs.misc.log_output[EVENTLOG_OUTPUT_CONSOLE])
		//{
		//	eventlogs::get().last_log = false;
		//	m_cvar()->ConsolePrintf(crypt_str("-------------------------\n-------------------------\n-------------------------\n"));
		//}

		for (auto i = 1; i < m_globals()->m_maxclients; i++)
		{
			cheat.globals.fired_shots[i] = 0;
			cheat.globals.missed_shots[i] = 0;
			animation_system::get().is_dormant[i] = false;
			animation_system::get().player_resolver[i].reset();
			
			playeresp::get().esp_alpha_fade[i] = 0.0f;
			playeresp::get().health[i] = 100;
			c_dormant_esp::get().m_cSoundPlayers[i].reset();
		}

		misc::get().preserve_killfeed(true);
		antiaim::get().cs_match_win = false;
		antiaim::get().freeze_check = true;
		cheat.globals.bomb_timer_enable = true;
		cheat.globals.should_buy = 2;
		cheat.globals.should_clear_death_notices = true;
		cheat.globals.should_update_playerresource = true;
		cheat.globals.should_update_gamerules = true;
		cheat.globals.kills = 0;
		cheat.shots.clear();
	}
	else if (!strcmp(event_name, crypt_str("cs_win_panel_match")))
		antiaim::get().cs_match_win = true;
	else if (!strcmp(event_name, crypt_str("round_freeze_end")))
		antiaim::get().freeze_check = false;
	else if (!strcmp(event_name, crypt_str("bomb_defused")))
		cheat.globals.bomb_timer_enable = false;
	else if (!strcmp(event_name, crypt_str("round_mvp")))
	{
		auto user_id = m_engine()->GetPlayerForUserID(event->GetInt(crypt_str("userid")));

		if (configs.profileChanger.musickit_stattrak > -1 && cheat.local() && user_id == m_engine()->GetLocalPlayer() && m_gamerules()->m_bIsValveDS())
		{
			if (util::getGameMode() == GameMode::Competitive || util::getGameMode() == GameMode::Wingman) {
				event->SetInt("musickitmvps", configs.profileChanger.musickit_stattrak + 1);
				configs.profileChanger.musickit_stattrak++;
			}
		}
	}

	if ((configs.esp.bullet_tracer || configs.esp.enemy_bullet_tracer) && configs.player.enabled)
		bullettracers::get().events(event);

	eventlogs::get().events(event);
}

int C_HookedEvents::GetEventDebugID(void)
{
	return EVENT_DEBUG_ID_INIT;
}

void C_HookedEvents::AddListener(const char* name)
{
	static auto eventmanager = m_eventmanager();
	eventmanager->AddListener(this, name, false);
	cheat.globals.events.emplace_back(name);
}

void C_HookedEvents::Initialize()
{
	m_iDebugId = EVENT_DEBUG_ID_INIT;

	AddListener(crypt_str("player_footstep"));
	AddListener(crypt_str("player_hurt"));
	AddListener(crypt_str("player_death"));
	AddListener(crypt_str("weapon_fire"));
	AddListener(crypt_str("item_purchase"));
	AddListener(crypt_str("bullet_impact"));
	AddListener(crypt_str("round_start"));
	AddListener(crypt_str("round_freeze_end"));
	AddListener(crypt_str("bomb_defused"));
	AddListener(crypt_str("bomb_begindefuse"));
	AddListener(crypt_str("bomb_beginplant"));
	AddListener(crypt_str("cs_win_panel_match"));
	AddListener(crypt_str("player_team"));
	AddListener(crypt_str("round_mvp"));
}

void C_HookedEvents::Uninstall()
{
	m_eventmanager()->RemoveListener(this);
}
