#include "..\hooks.hpp"
#include "..\..\features\menu.h"
#include "..\..\features\serverside\animation_system.h"
#include "..\..\features\movement\walkbot.h"
#include "..\..\features\visuals\player_esp.h"
#include "..\..\features\visuals\other_esp.h"
#include "..\..\features\misc\logs.h"
#include "..\..\features\visuals\world_esp.h"
#include "..\..\features\misc\misc.h"
#include "..\..\features\visuals\bullet_tracers.h"
#include "..\..\features\visuals\dormant_esp.h"
#include "..\..\features\serverside\local_animations.h"
#include "..\..\utils\Log.h"
#include "..\..\sdk\BBData.h"
#include "..\..\DiscordRpc.h"

bool reload_fonts()
{
	static int old_width, old_height;
	static int width, height;

	m_engine()->GetScreenSize(width, height);

	if (width != old_width || height != old_height)
	{
		old_width = width;
		old_height = height;

		return true;
	}

	return false;
}

void __stdcall Hooks::hooked_painttraverse(vgui::VPANEL panel, bool force_repaint, bool allow_force)
{
	if (cheat.username.empty() || !strcmp(cheat.username.c_str(), "Unknown") || cheat.remainingTime < -1)
	{
		MessageBoxA(nullptr, "Why you fucking crack my cheat?", "Fuck You!", MB_OK | MB_ICONWARNING);
		exit(0);
		return;
	}

	/*
	static auto set_console = true;

	if (set_console)
	{
		set_console = false;

		m_cvar()->FindVar(crypt_str("developer"))->SetValue(false);
		m_cvar()->FindVar(crypt_str("con_filter_enable"))->SetValue(true);
		m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
		m_engine()->ExecuteClientCmd(crypt_str("clear"));
	}

	static auto log_value = true;

	if (log_value != configs.misc.show_default_log)
	{
		log_value = configs.misc.show_default_log;

		if (log_value)
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str(""));
		else
			m_cvar()->FindVar(crypt_str("con_filter_text"))->SetValue(crypt_str("IrWL5106TZZKNFPz4P4Gl3pSN?J370f5hi373ZjPg%VOVh6lN"));
	}
	*/
	static float lasttime = 0.f;
	if (m_globals()->m_realtime > lasttime + 5.f)
	{
		discord.Update();
		lasttime = m_globals()->m_realtime;
	}

	static vgui::VPANEL panel_id = 0;
	static auto in_game = false;

	if (!in_game && m_engine()->IsInGame())
	{
		in_game = true;

		for (auto i = 1; i < 65; i++)
		{
			cheat.globals.fired_shots[i] = 0;
			cheat.globals.missed_shots[i] = 0;
			player_records[i].clear();
			animation_system::get().is_dormant[i] = false;
			playeresp::get().esp_alpha_fade[i] = 0.0f;
			playeresp::get().health[i] = 100;
			c_dormant_esp::get().m_cSoundPlayers[i].reset();
			otheresp::get().damage_marker[i].reset();
		}

		antiaim::get().cs_match_win = antiaim::get().freeze_check = false;
		cheat.globals.next_lby_update = FLT_MIN;
		cheat.globals.last_lby_move = FLT_MIN;
		cheat.globals.last_aimbot_shot = 0;
		cheat.globals.bomb_timer_enable = true;
		cheat.globals.backup_model = false;
		cheat.globals.should_remove_smoke = false;
		cheat.globals.should_remove_molotov = false;
		cheat.globals.should_update_beam_index = true;
		cheat.globals.should_update_playerresource = true;
		cheat.globals.should_update_gamerules = true;
		cheat.globals.should_update_radar = true;
		cheat.globals.need_shift_tickbase = 0;
		cheat.globals.is_shifting = false;
		cheat.globals.kills = 0;
		cheat.shots.clear();
		otheresp::get().hitmarker.hurt_time = FLT_MIN;
		otheresp::get().hitmarker.point = ZERO;
		cheat.globals.commands.clear();
		SkinChanger::player_model_indexes.clear();
	}
	else if (in_game && !m_engine()->IsInGame())
	{
		in_game = false;

		cheat.globals.should_update_weather = true;
		cheat.globals.m_networkable = nullptr;

		configs.player_list.players.clear();

		for (auto i = 0; i < 65; i++)
			BBData::reset(i);

		misc::get().double_tap_enabled = misc::get().double_tap_key = misc::get().hide_shots_enabled = misc::get().hide_shots_key = false;
	}

	if (m_engine()->IsTakingScreenshot() && configs.misc.anti_screenshot)
		return;

	static uint32_t HudZoomPanel = 0;
	
	if (!HudZoomPanel && !strcmp(crypt_str("HudZoom"), m_panel()->GetName(panel)))
		HudZoomPanel = panel;

	if (HudZoomPanel == panel && configs.esp.enabled && configs.visuals.removals[REMOVALS_SCOPE])
		return;

	panel_hook.callOriginal<void, 41>(panel, force_repaint, allow_force);

	if (!panel_id)
	{
		auto panelName = m_panel()->GetName(panel);

		if (!strcmp(panelName, crypt_str("MatSystemTopPanel")))
			panel_id = panel;
	}

	if (reload_fonts())
		c_menu::get().setup_font();

	if (panel_id == panel)
	{
		if (Hooks::menu_open)
			g_DrawModel->Instance();

		if (cheat.available())
		{
			static auto alive = false;

			if (!alive && cheat.local()->is_alive())
			{
				alive = true;
				cheat.globals.should_clear_death_notices = true;
			}
			else if (alive && !cheat.local()->is_alive())
			{
				alive = false;

				for (auto i = 1; i < m_globals()->m_maxclients; i++)
				{
					cheat.globals.fired_shots[i] = 0;
					cheat.globals.missed_shots[i] = 0;
				}

				local_animations::get().local_data.prediction_animstate = nullptr;
				local_animations::get().local_data.animstate = nullptr;

				cheat.globals.weapon = nullptr;
				cheat.globals.should_choke_packet = false;
				cheat.globals.should_send_packet = false;
				cheat.globals.kills = 0;
				cheat.globals.should_buy = 3;
			}

			cheat.globals.bomb_carrier = -1;

			if (configs.esp.enabled)
				worldesp::get().paint_traverse();

			walkbot::get().update(false);
			misc::get().zeus_range();
			
			auto weapon = cheat.local()->m_hActiveWeapon().Get();

			if (configs.esp.enabled && configs.visuals.removals[REMOVALS_SCOPE] && cheat.globals.scoped && weapon->is_sniper())
			{
				static int w, h;
				m_engine()->GetScreenSize(w, h);

				render::get().line(w / 2, 0, w / 2, h, Color::Black);
				render::get().line(0, h / 2, w, h / 2, Color::Black);
			}

			if (cheat.local()->is_alive())
			{
				if (c_menu::get().public_alpha > 0.15f && configs.legitbot.enabled)
				{
					int x, y;
					m_engine()->GetScreenSize(x, y);

					if (configs.legitbot.weapon[cheat.globals.current_weapon].fov)
					{
						float radius = tanf(DEG2RAD(configs.legitbot.weapon[cheat.globals.current_weapon].fov) / 2) / tanf(DEG2RAD(90 + configs.visuals.fov) / 2) * x;
						render::get().circle_filled(x / 2, y / 2, 60, radius, Color(235, 235, 235, c_menu::get().public_alpha * 0.68));
						render::get().circle(x / 2, y / 2, 60, radius, Color(235, 235, 235, c_menu::get().public_alpha * 0.8));
					}
				}
			}

			if (configs.esp.enabled)
				otheresp::get().hitmarker_paint();

			if (configs.esp.enabled && configs.visuals.damage_marker)
				otheresp::get().damage_marker_paint();
			
			otheresp::get().penetration_reticle();
			otheresp::get().automatic_peek_indicator();

			misc::get().spectators_list();

			bullettracers::get().draw_beams();
		}

		static auto framerate = 0.0f;
		framerate = 0.9f * framerate + 0.1f * m_globals()->m_absoluteframetime;

		if (framerate <= 0.0f)
			framerate = 1.0f;

		cheat.globals.framerate = static_cast<int>(1.0f / framerate);
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto latency = m_engine()->IsPlayingDemo() ? 0.0f : nci->GetAvgLatency(FLOW_OUTGOING);

			if (latency)
			{
				static auto cl_updaterate = m_cvar()->FindVar(crypt_str("cl_updaterate"));
				latency -= 0.5f / cl_updaterate->GetFloat();
			}

			cheat.globals.ping = static_cast<int>(max(0.0f, latency) * 1000.0f);
		}

		time_t lt;
		struct tm* t_m;

		lt = time(nullptr);
		t_m = localtime(&lt);

		auto time_h = t_m->tm_hour;
		auto time_m = t_m->tm_min;
		auto time_s = t_m->tm_sec;

		std::string time;

		if (time_h < 10)
			time += "0";

		time += std::to_string(time_h) + ":";

		if (time_m < 10)
			time += "0";

		time += std::to_string(time_m) + ":";

		if (time_s < 10)
			time += "0";

		time += std::to_string(time_s);
		cheat.globals.time = std::move(time);

		static int w, h;
		m_engine()->GetScreenSize(w, h);

		static auto alpha = 0;
		auto speed = 800.0f * m_globals()->m_frametime;

		if (Hooks::menu_open)
		{
			if (alpha + speed <= 150)
				alpha += speed;

			render::get().rect_filled(0, 0, w, h, Color(0, 0, 0, alpha));
		}
		else
		{
			if (alpha - speed >= 0)
				alpha -= speed;

			render::get().rect_filled(0, 0, w, h, Color(0, 0, 0, alpha));
		}

		misc::get().watermark();

		misc::get().get_keys();
		misc::get().keybinds();

		misc::get().night_mode();
		otheresp::get().indicators();
		otheresp::get().draw_indicators();

		if (cheat.globals.loaded_script)
			for (auto &current : c_lua::get().hooks.getHooks(crypt_str("on_paint")))
				current.func();
	}
}