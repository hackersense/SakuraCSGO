#include "misc.h"

#include "..\ragebot\ragebot.h"
#include "..\visuals\world_esp.h"
#include "..\..\sdk\MusicStatus.hpp"
#include "..\..\sdk\ProtobufReader.h"
#include "..\serverside\engine_prediction.h"
#include "logs.h"

MStatus musicS = MStatus{ L"", L"", false, false, MusicPlayerType::None };
float lasttime = 0.f;

void StatusCheck()
{
	if (m_globals()->m_realtime > lasttime + 5.f) {
		const auto spotify = musicStatus.getSpotify();
		const auto ypm = musicStatus.getYesPlayMusic();
		std::vector<MStatus> musicStatus = { spotify, ypm };
		MStatus curMStatus = MStatus{ L"", L"", false, false, MusicPlayerType::None };

		for (MStatus status : musicStatus) {
			if (status.find) {
				if (musicS.musicPlayerType == status.musicPlayerType && !status.playing)
					musicS = status;

				if (!musicS.playing && status.playing && musicS.musicPlayerType != status.musicPlayerType)
					curMStatus = status;
				else if (status.playing && musicS.musicPlayerType == status.musicPlayerType)
					curMStatus = status;
				else if (musicS.musicPlayerType == MusicPlayerType::None)
					curMStatus = status;
			}
		}

		musicS = curMStatus;

		lasttime = m_globals()->m_realtime;
	}
}

void misc::watermark()
{
	if (!configs.menu.watermark)
		return;

	auto width = 0, height = 0;
	m_engine()->GetScreenSize(width, height);

	std::string watermarkText = cheat.cheatname + crypt_str(" [") + cheat.edition + crypt_str("] | ") + cheat.username + crypt_str(" | ");

	if (m_engine()->IsInGame())
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (nci)
		{
			auto server = nci->GetAddress();
			bool isLocal = false;
			bool isValve = false;

			if (!strcmp(server, crypt_str("loopback")))
			{
				server = crypt_str("Local server");
				isLocal = true;
			}
			else if (m_gamerules()->m_bIsValveDS())
			{
				server = crypt_str("Valve server");
				isValve = true;
			}

			auto tickrate = std::to_string(static_cast<int>(1.0f / m_globals()->m_intervalpertick));
			watermarkText.append(crypt_str(" | ")).append(server).append(crypt_str(" | ")).append(isLocal ? "" : std::to_string(cheat.globals.ping) + crypt_str(" ms | ")).append(isValve ? "" : tickrate + crypt_str(" tick | "));
		}
	}

	watermarkText.append(cheat.globals.time);

	if (configs.menu.music_status)
	{
		StatusCheck();
		if (musicS.find)
			if (musicS.playing)
			{
				watermarkText.append(crypt_str(" | ")).append(StringUtils::wstringToString(musicS.artist.c_str()));
				watermarkText.append(crypt_str(" - ")).append(StringUtils::wstringToString(musicS.name.c_str()));
			}
			else
				watermarkText.append(crypt_str(" | ")).append(MusicPlayerTypeStr[static_cast<int>(musicS.musicPlayerType)]);
	}

	auto box_width = render::get().text_width(fonts[NAME], watermarkText.c_str()) + 10;

	if (configs.menu.watermark == 1)
	{
		int x, y, w, h;

		w = width / 2; h = height / 2;

		x = width - 10 - box_width, y = 10, w = box_width, h = 20;

		auto watermark = [](int x, int y, int w, int h, Color c) {
			render::get().line(x, y, x, y + h, c);
			render::get().line(x, y + h, x + w + 1, y + h, c);
			render::get().line(x + w, y, x + w, y + h, c);
			render::get().line(x, y, x + w, y, c);
		};

		render::get().rect_filled(x, y, w, h, Color(11, 11, 11));
		watermark(x - 0, y - 0, w + 0 * 2, h + 0 * 2, Color::Black);
		watermark(x - 1, y - 1, w + 1 * 2, h + 1 * 2, Color(53, 53, 53));
		watermark(x - 2, y - 2, w + 2 * 2, h + 2 * 2, Color(28, 28, 28));
		watermark(x - 3, y - 3, w + 3 * 2, h + 3 * 2, Color(28, 28, 28));
		watermark(x - 4, y - 4, w + 4 * 2, h + 4 * 2, Color(28, 28, 28));
		watermark(x - 5, y - 5, w + 5 * 2, h + 5 * 2, Color(53, 53, 53));
		watermark(x - 6, y - 6, w + 6 * 2, h + 6 * 2, Color::Black);

		int rb = floor(sin(m_globals()->m_realtime * 2) * 127 + 128);
		int gb = floor(sin(m_globals()->m_realtime * 2 + 2) * 127 + 128);
		int bb = floor(sin(m_globals()->m_realtime * 2 + 4) * 127 + 128);
		int rbb = floor(sin(m_globals()->m_realtime * 3) * 127 + 128);
		int gbb = floor(sin(m_globals()->m_realtime * 3 + 2) * 127 + 128);
		int bbb = floor(sin(m_globals()->m_realtime * 3 + 4) * 127 + 128);
		render::get().gradient(x + 0.1f, y + 0.5f, w + 1 * 2, 1, Color(rb, gb, bb, 170), Color(rbb, gbb, bbb, 240), GRADIENT_HORIZONTAL);

		render::get().text(fonts[NAME], width - 10 - box_width + 5, 14.9f, Color(255, 255, 255, 255), HFONT_OFF_SCREEN, watermarkText.c_str());
	}
	else
	{
		//render::get().gradient(width - 10 - box_width, 10, box_width / 2, 1, Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 170), Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 240), GRADIENT_HORIZONTAL);
		//render::get().gradient(width - 10 - box_width + (box_width / 2), 10, box_width / 2, 1, Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 240), Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 170), GRADIENT_HORIZONTAL);
		int rb = floor(sin(m_globals()->m_realtime * 2) * 127 + 128);
		int gb = floor(sin(m_globals()->m_realtime * 2 + 2) * 127 + 128);
		int bb = floor(sin(m_globals()->m_realtime * 2 + 4) * 127 + 128);
		int rbb = floor(sin(m_globals()->m_realtime * 3) * 127 + 128);
		int gbb = floor(sin(m_globals()->m_realtime * 3 + 2) * 127 + 128);
		int bbb = floor(sin(m_globals()->m_realtime * 3 + 4) * 127 + 128);
		render::get().gradient(width - 10 - box_width, 10, box_width, 2, Color(rb, gb, bb, 170), Color(rbb, gbb, bbb, 240), GRADIENT_HORIZONTAL);
		//render::get().gradient(width - 10 - box_width + (box_width / 2), 10, box_width / 2, 1, Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 240), Color(configs.menu.menu_theme.r(), configs.menu.menu_theme.g(), configs.menu.menu_theme.b(), 170), GRADIENT_HORIZONTAL);

		render::get().rect_filled(width - 10 - box_width, 11, box_width, 18, Color(10, 10, 10, 150));

		render::get().text(fonts[NAME], width - 10 - box_width + 5, 20, Color(255, 255, 255, 220), HFONT_CENTERED_Y, watermarkText.c_str());
	}
}

void misc::no_duck_cooldown(CUserCmd* cmd)
{
	if (configs.misc.no_duck_cooldown)
		cmd->m_buttons |= IN_BULLRUSH;
}

std::vector<std::wstring> sakura_tags =
{
L" ",
L"\u2744 ",
L"\u2744S ",
L"\u2744Sa ",
L"\u2744Sak ",
L"\u2744Saku ",
L"\u2744Sakur ",
L"\u2744Sakura ",
L"\u2744Sakura. ",
L"\u2744Sakura.c ",
L"\u2744Sakura.cc ",
L"\u2744Sakura.cc\u2744 ",
L"\u2744Sakura.cc\u2744 ",
L"Sakura.cc\u2744 ",
L"akura.cc\u2744 ",
L"kura.cc\u2744 ",
L"ura.cc\u2744 ",
L"ra.cc\u2744 ",
L"a.cc\u2744 ",
L".cc\u2744 ",
L"cc\u2744 ",
L"c\u2744 ",
L"\u2744 ",
L" "
};

std::vector<std::string> chocolate_tags = { "", "|", "|\\ ", "|\\|", "C ", "C3 ", "Ch ", "Ch\\ ", "Ch\\/ ", "Cho ", "Cho3 ", "Choc ", "Choc| ", "Choc|2 ", "Choco| ", "Chocol ", "Chocol4 ", "Chocola ", "Chocolat| ", "Chocolat|D ", "Chocolat ", "Chocolate ", "Chocolate. ", "Chocolate.< ", "Chocolate.m ", "Chocolate.m< ", "Chocolate.me ", "Chocolate.me ", "Chocolate.m< ", "Chocolate.m ", "Chocolate.< ", "Chocolate. ", "Chocolate ", "Chocola|D ", "Chocola| ", "Chocola_ ", "Chocol4 ", "Chocol ", "Choco_ ", "Choco| ", "Choco_ ", "Choc|2 ", "Choc| ", "Choc_ ", "Cho3 ", "Cho_ ", "Ch\\/ ", "Ch\\ ", "Ch_ ", "C3 ", "C_ ", "|\\|", "|\\ ", "|", "" };

void misc::clan_tag()
{
	static auto removed = false;

	if (!configs.misc.clan_tag && !removed)
	{
		removed = true;
		memory->setClanTag(crypt_str(""));
		return;
	}

	if (configs.misc.clan_tag)
	{
		static auto time = -1;

		const auto isSakura = !strcmp(cheat.cheatname.c_str(), crypt_str("Sakura"));
		auto main_time = static_cast<int>(m_globals()->m_curtime * 6); // 6 is speed, 3 is fastest, INT_MAX is slowest

		if (main_time != time)
		{
			if (isSakura)
				memory->setClanTag(StringUtils::wstringToString(sakura_tags.at(main_time % sakura_tags.size())).c_str());
			else
				memory->setClanTag(chocolate_tags.at(main_time % chocolate_tags.size()).c_str());
		}

		time = main_time;
		removed = false;
	}
}

void misc::force_crosshair()
{
	static auto weapon_debug_spread_show = m_cvar()->FindVar("weapon_debug_spread_show");

	if (configs.esp.force_crosshair)
	{
		if (weapon_debug_spread_show->GetInt() != 3 && !cheat.globals.scoped)
			weapon_debug_spread_show->SetValue(3);
		else if (cheat.globals.scoped && weapon_debug_spread_show->GetInt() == 3)
			weapon_debug_spread_show->SetValue(0);
	}
	else if (weapon_debug_spread_show->GetInt() == 3)
		weapon_debug_spread_show->SetValue(0);

}

void misc::auto_crouch(CUserCmd* cmd)
{
	if (antiaim::get().freeze_check || antiaim::get().cs_match_win)
	{
		cheat.globals.fakeducking = false;
		return;
	}

	//if (fakelag::get().condition)
	//{
	//	cheat.globals.fakeducking = false;
	//	return;
	//}

	if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
	{
		cheat.globals.fakeducking = false;
		return;
	}

	if (!key_binds::get().get_key_bind_state(key_bind_features::FAKE_DUCK))
	{
		cheat.globals.fakeducking = false;
		return;
	}

	cheat.globals.fakeducking = true;
	
	if (m_clientstate()->m_iChokedCommands <= 6)
		cmd->m_buttons &= ~IN_DUCK;
	else
		cmd->m_buttons |= IN_DUCK;

	cheat.send_packet = m_clientstate()->m_iChokedCommands >= 14;

	/*
	if (m_clientstate()->m_iChokedCommands <= 6)
		cmd->m_buttons &= ~IN_DUCK;
	else
		cmd->m_buttons |= IN_DUCK;

	cheat.send_packet = m_clientstate()->m_iChokedCommands >= 12;

	if (!cheat.globals.fakeducking && m_clientstate()->iChokedCommands != 7)
		return;

	if (m_clientstate()->iChokedCommands >= 7)
		cmd->m_buttons |= IN_DUCK;
	else
		cmd->m_buttons &= ~IN_DUCK;

	cheat.globals.fakeducking = true;
	*/
}

void misc::slide_walk(CUserCmd* cmd)
{
	if (!cheat.local()->is_alive())
		return;

	if (cheat.local()->get_move_type() == MOVETYPE_LADDER)
		return;

	if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	if (antiaim::get().condition(cmd, true) && ((configs.misc.slide_walk == 2 && !math::random_int(0, 2)) || configs.misc.slide_walk == 1))
	{
		if (cmd->m_forwardmove > 0.0f)
		{
			cmd->m_buttons |= IN_BACK;
			cmd->m_buttons &= ~IN_FORWARD;
		}
		else if (cmd->m_forwardmove < 0.0f)
		{
			cmd->m_buttons |= IN_FORWARD;
			cmd->m_buttons &= ~IN_BACK;
		}

		if (cmd->m_sidemove > 0.0f)
		{
			cmd->m_buttons |= IN_MOVELEFT;
			cmd->m_buttons &= ~IN_MOVERIGHT;
		}
		else if (cmd->m_sidemove < 0.0f)
		{
			cmd->m_buttons |= IN_MOVERIGHT;
			cmd->m_buttons &= ~IN_MOVELEFT;
		}
	}
	else
	{
		auto buttons = cmd->m_buttons & ~(IN_MOVERIGHT | IN_MOVELEFT | IN_BACK | IN_FORWARD);

		if ((configs.misc.slide_walk == 2 && math::random_int(0, 1)) || configs.misc.slide_walk == 1)
		{
			if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
				return;

			if (cmd->m_forwardmove <= 0.0f)
				buttons |= IN_BACK;
			else
				buttons |= IN_FORWARD;

			if (cmd->m_sidemove > 0.0f)
				goto LABEL_15;
			else if (cmd->m_sidemove >= 0.0f)
				goto LABEL_18;

			goto LABEL_17;
		}
		else
			goto LABEL_18;

		if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
			return;

		if (cmd->m_forwardmove <= 0.0f)
			buttons |= IN_FORWARD;
		else
			buttons |= IN_BACK;

		if (cmd->m_sidemove > 0.0f)
		{
		LABEL_17:
			buttons |= IN_MOVELEFT;
			goto LABEL_18;
		}

		if (cmd->m_sidemove < 0.0f)
			LABEL_15:

		buttons |= IN_MOVERIGHT;

	LABEL_18:
		cmd->m_buttons = buttons;
	}
}

void misc::fake_prime()
{
	static bool lastState = false;

	if (configs.profileChanger.fake_prime != lastState)
	{
		lastState = configs.profileChanger.fake_prime;

		if (DWORD oldProtect; VirtualProtect(memory->fakePrime, 4, PAGE_EXECUTE_READWRITE, &oldProtect))
		{
			constexpr uint8_t patch[]{ 0x31, 0xC0, 0x40, 0xC3 };
			std::memcpy(memory->fakePrime, patch, 4);
			VirtualProtect(memory->fakePrime, 4, oldProtect, nullptr);
		}
	}
}

void misc::automatic_peek(CUserCmd* cmd, float current_yaw)
{
	if (!cheat.globals.weapon->is_non_aim() && key_binds::get().get_key_bind_state(key_bind_features::AUTO_PEEK))
	{
		if (cheat.globals.start_position.IsZero())
		{
			cheat.globals.start_position = cheat.local()->GetAbsOrigin();

			if (!(engineprediction::get().backup_data.flags & FL_ONGROUND))
			{
				Ray_t ray;
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				ray.Init(cheat.globals.start_position, cheat.globals.start_position - Vector(0.0f, 0.0f, 1000.0f));
				m_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

				if (trace.fraction < 1.0f)
					cheat.globals.start_position = trace.endpos + Vector(0.0f, 0.0f, 2.0f);
			}
		}
		else
		{
			auto revolver_shoot = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !cheat.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

			if (cmd->m_buttons & IN_ATTACK && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
				cheat.globals.fired_shot = true;

			if (cheat.globals.fired_shot)
			{
				auto& current_position = cheat.local()->GetAbsOrigin();
				auto difference = current_position - cheat.globals.start_position;

				if (difference.Length2D() > 5.0f)
				{
					auto velocity = Vector(difference.x * cos(current_yaw / 180.0f * M_PI) + difference.y * sin(current_yaw / 180.0f * M_PI), difference.y * cos(current_yaw / 180.0f * M_PI) - difference.x * sin(current_yaw / 180.0f * M_PI), difference.z);

					cmd->m_forwardmove = -velocity.x * 20.0f;
					cmd->m_sidemove = velocity.y * 20.0f;
				}
				else
				{
					cheat.globals.fired_shot = false;
					cheat.globals.start_position.Zero();
				}
			}
		}
	}
	else
	{
		cheat.globals.fired_shot = false;
		cheat.globals.start_position.Zero();
	}
}

void misc::view_model()
{
	if (configs.esp.viewmodel_fov)
	{
		auto viewFOV = (float)configs.esp.viewmodel_fov + 68.0f;
		static auto viewFOVcvar = m_cvar()->FindVar(crypt_str("viewmodel_fov"));

		if (viewFOVcvar->GetFloat() != viewFOV)
		{
			viewFOVcvar->m_fnChangeCallbacks.m_Size = 0.0f;
			viewFOVcvar->SetValue(viewFOV);
		}
	}

	if (configs.esp.viewmodel_x)
	{
		auto viewX = (float)configs.esp.viewmodel_x / 2.0f;
		static auto viewXcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_x"));

		if (viewXcvar->GetFloat() != viewX)
		{
			viewXcvar->m_fnChangeCallbacks.m_Size = 0.0f;
			viewXcvar->SetValue(viewX);
		}
	}

	if (configs.esp.viewmodel_y)
	{
		auto viewY = (float)configs.esp.viewmodel_y / 2.0f;
		static auto viewYcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_y"));

		if (viewYcvar->GetFloat() != viewY)
		{
			viewYcvar->m_fnChangeCallbacks.m_Size = 0.0f;
			viewYcvar->SetValue(viewY);
		}
	}

	if (configs.esp.viewmodel_z)
	{
		auto viewZ = (float)configs.esp.viewmodel_z / 2.0f;
		static auto viewZcvar = m_cvar()->FindVar(crypt_str("viewmodel_offset_z"));

		if (viewZcvar->GetFloat() != viewZ)
		{
			viewZcvar->m_fnChangeCallbacks.m_Size = 0.0f;
			viewZcvar->SetValue(viewZ);
		}
	}
}

void misc::full_bright()
{
	if (!configs.player.enabled)
		return;

	static auto mat_fullbright = m_cvar()->FindVar(crypt_str("mat_fullbright"));

	if (mat_fullbright->GetBool() != configs.esp.bright)
		mat_fullbright->SetValue(configs.esp.bright);
}

void misc::pov_arrows(Vector origin, Color color)
{
	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int iScreenWidth, iScreenHeight;
		m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

		auto xOk = iScreenWidth > screen.x;
		auto yOk = iScreenHeight > screen.y;

		return xOk && yOk;
	};

	Vector screenPos;

	if (isOnScreen(origin, screenPos))
		return;

	Vector viewAngles;
	m_engine()->GetViewAngles(viewAngles);

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto screenCenter = Vector2D(width * 0.5f, height * 0.5f);
	auto angleYawRad = DEG2RAD(viewAngles.y - math::calculate_angle(cheat.globals.eye_pos, origin).y - 90.0f);

	auto radius = configs.player.distance;
	auto size = configs.player.size;

	auto newPointX = screenCenter.x + ((((width - (size * 3)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
	auto newPointY = screenCenter.y + ((((height - (size * 3)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));

	std::array<Vector2D, 3> points
	{
		Vector2D(newPointX - size, newPointY - size),
		Vector2D(newPointX + size, newPointY),
		Vector2D(newPointX - size, newPointY + size)
	};

	math::rotate_triangle(points, viewAngles.y - math::calculate_angle(cheat.globals.eye_pos, origin).y - 90.0f);
	imRender->TriangleFilled(points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, color);
}

void misc::zeus_range()
{
	if (!configs.player.enabled)
		return;

	if (!configs.esp.taser_range)
		return;

	if (!m_input()->m_bCameraInThirdPerson)
		return;

	if (!cheat.local()->is_alive())
		return;

	auto weapon = cheat.local()->m_hActiveWeapon().Get();

	if (!weapon)
		return;

	if (weapon->m_iItemDefinitionIndex() != WEAPON_TASER)
		return;

	auto weapon_info = weapon->get_weapon_info();

	if (!weapon_info)
		return;

	render::get().Draw3DRainbowCircle(cheat.local()->get_eye_position(), weapon_info->flRange);
}

void misc::night_mode()
{
	static auto in_game = false;

	if (m_engine()->IsInGame() && !in_game)
	{
		in_game = true;

		cheat.globals.change_materials = true;
		worldesp::get().changed = true;

		static auto skybox = m_cvar()->FindVar(crypt_str("sv_skyname"));
		worldesp::get().backup_skybox = skybox->GetString();
		return;
	}
	else if (!m_engine()->IsInGame() && in_game)
		in_game = false;

	static auto player_enable = configs.player.enabled;

	if (player_enable != configs.player.enabled)
	{
		player_enable = configs.player.enabled;
		cheat.globals.change_materials = true;
		return;
	}

	static auto setting = configs.esp.nightmode;

	if (setting != configs.esp.nightmode)
	{
		setting = configs.esp.nightmode;
		cheat.globals.change_materials = true;
		return;
	}

	static auto setting_world = configs.esp.world_color;

	if (setting_world != configs.esp.world_color)
	{
		setting_world = configs.esp.world_color;
		cheat.globals.change_materials = true;
		return;
	}

	static auto setting_props = configs.esp.props_color;

	if (setting_props != configs.esp.props_color)
	{
		setting_props = configs.esp.props_color;
		cheat.globals.change_materials = true;
	}
}

void misc::desync_arrows()
{
	if (!cheat.local()->is_alive())
		return;

	if (!configs.ragebot.enabled)
		return;

	if (!configs.antiaim.enabled)
		return;

	if ((configs.antiaim.manual_back.key <= KEY_NONE || configs.antiaim.manual_back.key >= KEY_MAX) && (configs.antiaim.manual_left.key <= KEY_NONE || configs.antiaim.manual_left.key >= KEY_MAX) && (configs.antiaim.manual_right.key <= KEY_NONE || configs.antiaim.manual_right.key >= KEY_MAX))
		antiaim::get().manual_side = SIDE_NONE;

	if (!configs.antiaim.flip_indicator)
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	static auto alpha = 1.0f;
	static auto switch_alpha = false;

	if (alpha <= 0.0f || alpha >= 1.0f)
		switch_alpha = !switch_alpha;

	alpha += switch_alpha ? 2.0f * m_globals()->m_frametime : -2.0f * m_globals()->m_frametime;
	alpha = math::clamp(alpha, 0.0f, 1.0f);

	auto color = configs.antiaim.flip_indicator_color;
	color.SetAlpha((int)(min(255.0f * alpha, color.a())));

	if (antiaim::get().manual_side == SIDE_BACK)
		imRender->TriangleFilled(width / 2, height / 2 + 80, width / 2 - 10, height / 2 + 60, width / 2 + 10, height / 2 + 60, color);
	else if (antiaim::get().manual_side == SIDE_LEFT)
		imRender->TriangleFilled(width / 2 - 55, height / 2 + 10, width / 2 - 75, height / 2, width / 2 - 55, height / 2 - 10, color);
	else if (antiaim::get().manual_side == SIDE_RIGHT)
		imRender->TriangleFilled(width / 2 + 55, height / 2 - 10, width / 2 + 75, height / 2, width / 2 + 55, height / 2 + 10, color);
}

void misc::lag_hitbox()
{
	if (!configs.player.enabled)
		return;

	if (!configs.player.lag_hitbox)
		return;

	auto player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(ragebot::get().last_target_index));

	if (!player)
		return;

	auto model = player->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto hitbox_set = studio_model->getHitboxSet(player->m_nHitboxSet());

	if (!hitbox_set)
		return;

	for (auto i = 0; i < hitbox_set->numhitboxes; i++)
	{
		auto hitbox = hitbox_set->getHitbox(i);

		if (!hitbox)
			continue;

		if (hitbox->radius == -1.0f)
			continue;

		auto min = ZERO;
		auto max = ZERO;

		math::vector_transform(hitbox->bbmin, ragebot::get().last_target[ragebot::get().last_target_index].record.matrixes_data.main[hitbox->bone], min);
		math::vector_transform(hitbox->bbmax, ragebot::get().last_target[ragebot::get().last_target_index].record.matrixes_data.main[hitbox->bone], max);

		m_debugoverlay()->AddCapsuleOverlay(min, max, hitbox->radius, configs.player.lag_hitbox_color.r(), configs.player.lag_hitbox_color.g(), configs.player.lag_hitbox_color.b(), configs.player.lag_hitbox_color.a(), 4.0f, 0, 1);
	}
}

void misc::ragdolls()
{
	if (!configs.misc.ragdolls)
		return;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->IsDormant())
			continue;

		auto client_class = e->GetClientClass();

		if (!client_class)
			continue;

		if (client_class->m_ClassID != CCSRagdoll)
			continue;

		static_cast<ragdoll_t*>(e)->m_vecForce().z = 800000.0f;
	}
}

void misc::fix_animation_lod()
{
	if (!m_engine()->IsInGame())
		return;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		player_t* entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
		if (!entity || entity == cheat.local() || entity->IsDormant() || !entity->is_alive())
			continue;

		entity->get_occlusion_flags() = 0;
		entity->get_occlusion_framecount() = m_globals()->m_framecount;
	}
}

void misc::rank_reveal(CUserCmd* m_pcmd)
{
	if (!configs.misc.rank_reveal || !(m_pcmd->m_buttons & IN_SCORE))
		return;

	m_client()->DispatchUserMessage(50, 0, 0, nullptr);

	//using RankReveal_t = bool(__cdecl*)(int*);
	//static auto rankreveal = (RankReveal_t)(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 51 A1 ? ? ? ? 85 C0 75 37")));

	//rankreveal(new int[] { 0, 0, 0 });
}

void misc::air_static_legs()
{
	if (!configs.misc.air_static_legs || !cheat.local()->get_animation_state())
		return;

	if (cheat.local()->m_fFlags() & FL_ONGROUND)
	{
		cheat.local()->get_animation_state()->m_flDurationInAir = .5f;
	}
	else
	{
		cheat.local()->get_animation_state()->m_flDurationInAir = 99;
		cheat.local()->get_animation_state()->m_flLandingDuckAdditiveSomething = 0;
		cheat.local()->get_animation_state()->m_bInHitGroundAnimation = false;
	}
}

void misc::fast_stop(CUserCmd* m_pcmd)
{
	if (!configs.misc.fast_stop)
		return;

	if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto pressed_move_key = m_pcmd->m_buttons & IN_FORWARD || m_pcmd->m_buttons & IN_MOVELEFT || m_pcmd->m_buttons & IN_BACK || m_pcmd->m_buttons & IN_MOVERIGHT || m_pcmd->m_buttons & IN_JUMP;

	if (pressed_move_key)
		return;

	if (antiaim::get().condition(m_pcmd))
	{
		auto& velocity = cheat.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;
		}
	}
	else
	{
		auto& velocity = cheat.local()->m_vecVelocity();

		if (velocity.Length2D() > 20.0f)
		{
			Vector direction;
			Vector real_view;

			math::vector_angles(velocity, direction);
			m_engine()->GetViewAngles(real_view);

			direction.y = real_view.y - direction.y;

			Vector forward;
			math::angle_vectors(direction, forward);

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));

			auto negative_forward_speed = -cl_forwardspeed->GetFloat();
			auto negative_side_speed = -cl_sidespeed->GetFloat();

			auto negative_forward_direction = forward * negative_forward_speed;
			auto negative_side_direction = forward * negative_side_speed;

			m_pcmd->m_forwardmove = negative_forward_direction.x;
			m_pcmd->m_sidemove = negative_side_direction.y;
		}
		else
		{
			auto speed = 1.01f;

			if (m_pcmd->m_buttons & IN_DUCK || cheat.globals.fakeducking)
				speed *= 2.94117647f;

			static auto switch_move = false;

			if (switch_move)
				m_pcmd->m_sidemove += speed;
			else
				m_pcmd->m_sidemove -= speed;

			switch_move = !switch_move;
		}
	}
}

bool can_shift_shot(int ticks)
{
	if (!cheat.local() || !cheat.local()->m_hActiveWeapon())
		return false;

	auto tickbase = cheat.local()->m_nTickBase();
	auto curtime = m_globals()->m_intervalpertick * (tickbase - ticks);

	if (curtime < cheat.local()->m_flNextAttack())
		return false;

	if (curtime < cheat.local()->m_hActiveWeapon()->m_flNextPrimaryAttack())
		return false;

	return true;
}

bool misc::double_tap(CUserCmd* m_pcmd)
{
	double_tap_enabled = true;

	static auto recharge_double_tap = false;
	static auto last_double_tap = 0;
	const auto max_tickbase_shift = cheat.globals.weapon->get_max_tickbase_shift_default();

	if (recharge_double_tap)
	{
		recharging_double_tap = true;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		recharge_double_tap = false;
		return false;
	}

	if (recharging_double_tap)
	{
		auto recharge_time = cheat.globals.weapon->can_double_tap() ? TIME_TO_TICKS(0.5f) : TIME_TO_TICKS(1.f);
		//auto recharge_time = cheat.globals.weapon->can_double_tap() ? TIME_TO_TICKS(0.5f) : TIME_TO_TICKS(0.75f);

		//if (can_shift_shot(max_tickbase_shift) && !ragebot::get().should_stop)
		if (!ragebot::get().should_stop && fabs(cheat.globals.fixed_tickbase - last_double_tap) > recharge_time)
		{
			recharging_double_tap = false;
			last_double_tap = 0;

			double_tap_enabled = true;
			double_tap_key = true;
		}
		else if (m_pcmd->m_buttons & IN_ATTACK)
			last_double_tap = cheat.globals.fixed_tickbase;
	}

	if (!configs.ragebot.enabled)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}

	if (!configs.ragebot.double_tap)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}

	/*
	if (cheat.local()->m_bGunGameImmunity() || cheat.local()->m_fFlags() & FL_FROZEN)
	{
		double_tap_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}
	*/

	if (cheat.globals.fakeducking)
	{
		double_tap_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}

	if (configs.ragebot.double_tap_key.key <= KEY_NONE || configs.ragebot.double_tap_key.key >= KEY_MAX)
	{
		double_tap_enabled = false;
		double_tap_key = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}

	if (double_tap_key && configs.ragebot.double_tap_key.key != configs.ragebot.hide_shots_key.key)
		hide_shots_key = false;

	if (!double_tap_key)
	{
		double_tap_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return false;
	}

	//if (m_gamerules()->m_bIsValveDS())
	//{
	//	double_tap_enabled = false;
	//	cheat.globals.ticks_allowed = 0;
	//	cheat.globals.next_tickbase_shift = 0;
	//	return false;
	//}

	//if (antiaim::get().freeze_check)
	//	return true;

	//if (double_tap_disable && configs.ragebot.disable_teleport)
	//{
	//	double_tap_enabled = false;
	//	double_tap_key = false;
	//	double_tap_disable = false;
	//}
	if ((double_tap_disable && cheat.send_packet) || (!cheat.globals.weapon->is_grenade() && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && cheat.send_packet && (m_pcmd->m_buttons & IN_ATTACK || (m_pcmd->m_buttons & IN_ATTACK2 && cheat.globals.weapon->is_knife()))))
	{
		//const auto speed = util::get_speed_move(user_cmd);
		//user_cmd->m_forwardmove = speed.x;
		//user_cmd->m_sidemove = speed.y;

		//if ((max_tickbase_shift < 16 && !double_tap_disable) || configs.ragebot.disable_teleport)
		cheat.globals.need_shift_tickbase = max_tickbase_shift;// cheat.globals.weapon->get_max_tickbase_shift();
		//else
		//{
		//	auto next_command_number = m_pcmd->m_command_number + 1;
		//	auto user_cmd = m_input()->GetUserCmd(next_command_number);

		//	memcpy(user_cmd, m_pcmd, sizeof(CUserCmd));
		//	user_cmd->m_command_number = next_command_number;

		//	util::copy_command(user_cmd, cheat.globals.weapon->get_max_tickbase_shift(), true, !double_tap_disable);
		//}

		if (cheat.globals.aimbot_working)
		{
			cheat.globals.double_tap_aim = true;
			cheat.globals.double_tap_aim_check = true;
		}

		if (!double_tap_disable)
			recharge_double_tap = true;
		double_tap_enabled = false;
		double_tap_key = false;
		double_tap_disable = false;
		last_double_tap = cheat.globals.fixed_tickbase;
		cheat.globals.tickbase_shift = max_tickbase_shift;
	}
	//else if (!cheat.globals.weapon->is_grenade() && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
	//	cheat.globals.tickbase_shift = max_tickbase_shift;

	return true;
}

void misc::hide_shots(CUserCmd* m_pcmd, bool should_work)
{
	hide_shots_enabled = true;

	if (!configs.ragebot.enabled)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			cheat.globals.ticks_allowed = 0;
			cheat.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (!configs.ragebot.hide_shots)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			cheat.globals.ticks_allowed = 0;
			cheat.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (configs.ragebot.hide_shots_key.key <= KEY_NONE || configs.ragebot.hide_shots_key.key >= KEY_MAX)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;

		if (should_work)
		{
			cheat.globals.ticks_allowed = 0;
			cheat.globals.next_tickbase_shift = 0;
		}

		return;
	}

	if (!should_work && double_tap_key)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		return;
	}

	if (!hide_shots_key)
	{
		hide_shots_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return;
	}

	double_tap_key = false;

	/*
	if (cheat.local()->m_bGunGameImmunity() || cheat.local()->m_fFlags() & FL_FROZEN)
	{
		hide_shots_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return;
	}
	*/

	if (cheat.globals.fakeducking)
	{
		hide_shots_enabled = false;
		cheat.globals.ticks_allowed = 0;
		cheat.globals.next_tickbase_shift = 0;
		return;
	}

	//if (antiaim::get().freeze_check)
	//	return;

	cheat.globals.next_tickbase_shift = cheat.globals.ticks_allowed;

	auto revolver_shoot = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !cheat.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);
	auto weapon_shoot = m_pcmd->m_buttons & IN_ATTACK && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || (m_pcmd->m_buttons & IN_ATTACK2 && cheat.globals.weapon->is_knife()) || revolver_shoot;

	if (hide_shots_disable && configs.ragebot.disable_teleport)
	{
		hide_shots_enabled = false;
		hide_shots_key = false;
		hide_shots_disable = false;
	}
	else if (hide_shots_disable && cheat.send_packet)
	{
		auto next_command_number = m_pcmd->m_command_number + 1;
		auto user_cmd = m_input()->GetUserCmd(next_command_number);

		memcpy(user_cmd, m_pcmd, sizeof(CUserCmd));
		user_cmd->m_command_number = next_command_number;

		//if (cheat.globals.ticks_allowed > 8)
		//cheat.globals.need_shift_tickbase = cheat.globals.weapon->get_max_tickbase_shift();
		//else
			util::copy_command(user_cmd, cheat.globals.next_tickbase_shift, cheat.globals.ticks_allowed > 8, false);

		hide_shots_enabled = false;
		hide_shots_key = false;
		hide_shots_disable = false;
		cheat.globals.tickbase_shift = cheat.globals.next_tickbase_shift;
	}
	else if (cheat.send_packet && !cheat.globals.weapon->is_grenade() && weapon_shoot)
		cheat.globals.tickbase_shift = cheat.globals.next_tickbase_shift;
	//else if (!cheat.globals.aimbot_working)
	//	m_pcmd->m_buttons &= ~IN_ATTACK;
}

void misc::blockbot(CUserCmd* cmd)
{
	if (!configs.blockbot.enabled || !key_binds::get().get_key_bind_state(key_bind_features::BLOCK_BOT))
		return;

	float flBestDistance = 250.0f;
	int iBestIndex = -1;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!entity || entity->IsDormant() || !entity->is_alive() || entity == cheat.local())
			continue;

		float flDistance = cheat.local()->m_vecOrigin().DistTo(entity->m_vecOrigin());
		if (flDistance < flBestDistance)
		{
			flBestDistance = flDistance;
			iBestIndex = i;
		}
	}

	auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(iBestIndex));

	if (!entity)
		return;

	float flBestSpeed = !configs.blockbot.type ? entity->m_vecVelocity().Length() : 450.0f;

	Vector angLocal;
	m_engine()->GetViewAngles(angLocal);

	Vector vecForward = entity->m_vecOrigin() - cheat.local()->m_vecOrigin();
	if (entity->hitbox_position(6).z < cheat.local()->m_vecOrigin().z && cheat.local()->m_vecOrigin().DistTo(entity->m_vecOrigin()) < 100.0f)
	{
		cmd->m_forwardmove = ((sin(DEG2RAD(angLocal.y)) * vecForward.y) + (cos(DEG2RAD(angLocal.y)) * vecForward.x)) * flBestSpeed;
		cmd->m_sidemove = ((cos(DEG2RAD(angLocal.y)) * -vecForward.y) + (sin(DEG2RAD(angLocal.y)) * vecForward.x)) * flBestSpeed;
	}
	else
	{
		auto yaw_delta = (atan2(vecForward.y, vecForward.x) * 180.0f / M_PI) - angLocal.y;
		if (yaw_delta > 180)
		{
			yaw_delta -= 360;
		}
		else if (yaw_delta < -180)
		{
			yaw_delta += 360;
		}
		if (yaw_delta > 0.25)
		{
			cmd->m_sidemove = -flBestSpeed;
		}
		else if (yaw_delta < -0.25)
		{
			cmd->m_sidemove = flBestSpeed;
		}
	}
}

void misc::anti_crasher()
{
	if (configs.misc.anti_crasher)
		m_engine()->GetNetChannelInfo()->SetTimeoutSeconds(3600, false);
}

void misc::anti_exploit()
{
	static auto cl_lagcompensation = m_cvar()->FindVar(crypt_str("cl_lagcompensation"));

	static auto is_command_set = configs.misc.anti_exploit;
	if (is_command_set != configs.misc.anti_exploit)
	{
		auto old_team = cheat.local()->m_iTeamNum();

		std::string command = "kill; jointeam 1; cl_lagcompensation ";
		command += std::to_string(configs.misc.anti_exploit ? 0 : 1) + ";";
		command += "jointeam " + std::to_string(old_team) + "; ";

		m_engine()->ExecuteClientCmd(command.data());
		is_command_set = configs.misc.anti_exploit;
	}

	cl_lagcompensation->m_fnChangeCallbacks.m_Size = 0;
	cl_lagcompensation->SetValue(!configs.misc.anti_exploit);
}

void misc::color_console() {
	static NEWMaterial* material[5];
	if (!material[0] || !material[1] || !material[2] || !material[3] || !material[4]) {
		for (short h = m_materialsystem2()->FirstMaterial(); h != m_materialsystem2()->InvalidMaterial(); h = m_materialsystem2()->NextMaterial(h)) {
			const auto mat = m_materialsystem2()->GetMaterial(h);

			if (!mat)
				continue;

			if (strstr(mat->GetName(), "vgui_white"))
				material[0] = mat;
			else if (strstr(mat->GetName(), "800corner1"))
				material[1] = mat;
			else if (strstr(mat->GetName(), "800corner2"))
				material[2] = mat;
			else if (strstr(mat->GetName(), "800corner3"))
				material[3] = mat;
			else if (strstr(mat->GetName(), "800corner4"))
				material[4] = mat;
		}
	}
	else {
		for (unsigned int num = 0; num < 5; num++) {
			//if (reset == 1)
			//{
			//	material[num]->ColorModulate(1.f, 1.f, 1.f);
			//	material[num]->AlphaModulate(1.f);
			//	continue;
			//}

			if (!configs.esp.console_color_enabled || !m_engine()->Con_IsVisible()) {
				material[num]->ColorModulate(1.f, 1.f, 1.f);
				material[num]->AlphaModulate(1.f);
				continue;
			}

			//if (config->visuals.console.rainbow) {
			//	material[num]->ColorModulate(rainbowColor(config->visuals.console.rainbowSpeed));
			//  material[num]->AlphaModulate(configs.esp.console_color.get_alpha());
			//}
			//else {
			material[num]->ColorModulate(configs.esp.console_color.get_red() / 255.f, configs.esp.console_color.get_green() / 255.f, configs.esp.console_color.get_blue() / 255.f);
			material[num]->AlphaModulate(configs.esp.console_color.get_alpha() / 255.f);
			//}
		}
	}
}

void misc::draw_server_hitboxes() {
	if (!configs.misc.server_hitbox)
		return;

	if (!m_engine()->IsConnected() || !m_engine()->IsInGame())
		return;

	if (!cheat.local() || !cheat.local()->is_alive())
		return;

	if (!m_input()->m_bCameraInThirdPerson)
		return;
	 
	auto get_player_by_index = [](int index) -> player_t* { // i dont need this shit func for anything else so it can be lambda
		typedef player_t* (__fastcall* player_by_index)(int);
		static auto player_index = reinterpret_cast<player_by_index>(util::FindSignature(crypt_str("server.dll"), crypt_str("85 C9 7E 32 A1")));

		if (!player_index)
			return false;

		return player_index(index);
	};

	static auto fn = util::FindSignature(crypt_str("server.dll"), crypt_str("55 8B EC 81 EC ? ? ? ? 53 56 8B 35 ? ? ? ? 8B D9 57 8B CE"));
	auto duration = -1.f;
	player_t* entity = get_player_by_index(cheat.local()->EntIndex());

	if (!entity)
		return;

	__asm {
		pushad
		movss xmm1, duration
		push 0 // 0 - colored, 1 - blue
		mov ecx, entity
		call fn
		popad
	}
}

void misc::killfeed_changer(IGameEvent& event)
{
	if (!configs.killfeedChanger.enabled)
		return;

	if (!cheat.local()->is_alive())
		return;

	if (const auto localUserId = cheat.local()->get_player_info()->userId; event.GetInt("attacker") != localUserId || event.GetInt("userid") == localUserId)
		return;

	if (configs.killfeedChanger.headshot)
		event.SetInt("headshot", 1);

	if (configs.killfeedChanger.dominated)
		event.SetInt("Dominated", 1);

	if (configs.killfeedChanger.revenge)
		event.SetInt("Revenge", 1);

	if (configs.killfeedChanger.penetrated)
		event.SetInt("penetrated", 1);

	if (configs.killfeedChanger.noscope)
		event.SetInt("noscope", 1);

	if (configs.killfeedChanger.thrusmoke)
		event.SetInt("thrusmoke", 1);

	if (configs.killfeedChanger.attackerblind)
		event.SetInt("attackerblind", 1);
}

void misc::preserve_killfeed(bool roundStart)
{
	if (!configs.esp.preserve_killfeed)
		return;

	static auto nextUpdate = 0.0f;

	if (roundStart) {
		nextUpdate = m_globals()->m_realtime + 10.0f;
		return;
	}

	if (nextUpdate > m_globals()->m_realtime)
		return;

	nextUpdate = m_globals()->m_realtime + 2.0f;

	const auto deathNotice = std::uintptr_t(memory->findHudElement(memory->hud, "CCSGO_HudDeathNotice"));
	if (!deathNotice)
		return;

	const auto deathNoticePanel = *reinterpret_cast<UIPanel**>(*reinterpret_cast<std::uintptr_t*>(deathNotice - 20 + 88) + sizeof(std::uintptr_t));

	const auto childPanelCount = deathNoticePanel->getChildCount();

	for (int i = 0; i < childPanelCount; ++i) {
		const auto child = deathNoticePanel->getChild(i);
		if (!child)
			continue;

		if (child->hasClass("DeathNotice_Killer"))
			child->setAttributeFloat("SpawnTime", m_globals()->m_curtime);
	}
}

void misc::force_relay_cluster()
{
	const std::string dataCentersList[] = { "", "syd", "vie", "gru", "scl", "dxb", "par", "fra", "hkg",
"maa", "bom", "tyo", "lux", "ams", "limc", "man", "waw", "sgp", "jnb",
"mad", "sto", "lhr", "atl", "eat", "ord", "lax", "mwh", "okc", "sea", "iad" };

	*memory->relayCluster = dataCentersList[configs.misc.force_relay_cluster];
}

void misc::anti_afk(CUserCmd* cmd)
{
	if (configs.misc.anti_afk && cmd->m_command_number % 2)
		cmd->m_buttons |= 1 << 27;
}

void misc::vote_revealer(IGameEvent& event)
{
	if (!configs.misc.reveal_votes)
		return;

	const auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(event.GetInt("entityid")));
	if (!entity || !entity->is_player())
		return;

	const auto votedYes = event.GetInt("vote_option") == 0;
	const auto isLocal = cheat.local() && entity == cheat.local();
	const char color = votedYes ? '\x06' : '\x07';

	m_clientmode()->getHudChat()->printf(0, " \x0C\u2022Sakura\u2022 %c%s\x01 voted %c%s\x01", isLocal ? '\x01' : color, isLocal ? "You" : entity->get_player_info()->getUsername().c_str(), color, votedYes ? "Yes" : "No");
}

void misc::vote_revealer_call(const void*& data, int& size, ECstrike15UserMessages& type)
{
	if (!configs.misc.reveal_votes)
		return;

	constexpr auto voteName = [](int index) {
		switch (index) {
		case 0: return "Kick";
		case 1: return "Change Level";
		case 6: return "Surrender";
		case 13: return "Start TimeOut";
		default: return "ERROR";
		}
	};

	if (type == ECstrike15UserMessages::CS_UM_VoteStart) {
		const auto reader = ProtobufReader{ static_cast<const std::uint8_t*>(data), size };
		const auto entityIndex = reader.readInt32(2);
		const auto vote_type = reader.readInt32(3);

		const auto entity = reinterpret_cast<player_t*>(entityIndex);
		if (!entity || !entity->is_player())
			return;

		const auto isLocal = cheat.local() && entity == cheat.local();

		const auto voteType = reader.readInt32(3);
		m_clientmode()->getHudChat()->printf(0, " \x0C\u2022Sakura\u2022 %c%s\x01 call vote (\x06%s\x01)", isLocal ? '\x01' : '\x06', isLocal ? "You" : entity->get_player_info()->getUsername().c_str(), voteName(voteType));
	}
	else if (type == ECstrike15UserMessages::CS_UM_VotePass)
		m_clientmode()->getHudChat()->printf(0, " \x0C\u2022Sakura\u2022\x01 vote\x06 PASS");
	else if (type == ECstrike15UserMessages::CS_UM_VoteFailed)
		m_clientmode()->getHudChat()->printf(0, " \x0C\u2022Sakura\u2022\x01 vote\x07 FAILED");
}

void misc::chat_revealer(IGameEvent& event, IGameEvent* events)
{
	if (!configs.misc.chat_revealer)
		return;

	if (!cheat.local())
		return;

	const auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(m_engine()->GetPlayerForUserID(events->GetInt("userid"))));
	if (!entity)
		return;

	std::string output = "\x0C\u2022Osiris\u2022\x01 ";

	auto team = entity->m_iTeamNum();
	bool isAlive = entity->is_alive();
	bool dormant = entity->IsDormant();
	if (dormant) {
		if (const auto pr = m_playerresource())
			isAlive = pr->getICSPlayerResource()->is_alive(entity->EntIndex());
	}

	const char* text = event.GetString("text");
	const char* lastLocation = entity->m_szLastPlaceName();
	const std::string name = entity->get_player_info()->getUsername();

	if (team == cheat.local()->m_iTeamNum())
		return;

	if (!isAlive)
		output += "*DEAD* ";

	team == 3 ? output += "(Counter-Terrorist) " : output += "(Terrorist) ";

	output = output + name + " @ " + lastLocation + " : " + text;
	m_clientmode()->getHudChat()->printf(0, output.c_str());
}

void misc::get_keys()
{
	if (!(configs.misc.keybind_list && m_engine()->IsInGame()))
		return;

	auto weapon = cheat.globals.weapon;

	if (key_binds::get().get_key_bind_state(key_bind_features::TRIGGER_BOT))
		applyKeyBinds(crypt_str("Trigger bot"), configs.legitbot.trigger.mode);

	if (configs.ragebot.double_tap && configs.ragebot.double_tap_key.key > KEY_NONE && configs.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key)
		applyKeyBinds(crypt_str("Double tap"), configs.ragebot.double_tap_key.mode);

	if (configs.ragebot.hide_shots && configs.ragebot.hide_shots_key.key > KEY_NONE && configs.ragebot.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key)
		applyKeyBinds(crypt_str("Hide shots"), configs.ragebot.hide_shots_key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::SAFE_POINT))
		applyKeyBinds(crypt_str("Force safe points"), configs.ragebot.safe_point_key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::BODY_AIM))
		applyKeyBinds(crypt_str("Force body aim"), configs.ragebot.body_aim_key.mode);

	if (configs.ragebot.enabled && key_binds::get().get_key_bind_state(key_bind_features::DAMAGE_OVERRIDE) && weapon && !weapon->is_non_aim())
		m_keybinds.push_back(m_keybind(crypt_str("Damage override"), std::to_string(configs.ragebot.weapon[cheat.globals.current_weapon].minimum_override_damage)));
		//applyKeyBinds(crypt_str("Damage override"), configs.ragebot.damage_override_key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::RESOLVER_OVERRIDE) && configs.ragebot.resolver[RESOLVER_YAW])
		applyKeyBinds(crypt_str("Resolver override"), configs.ragebot.resolver_override.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_DESYNC_FLIP))
		applyKeyBinds(crypt_str("Desync invert"), configs.antiaim.flip_desync.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_ZERO_PITCH))
		applyKeyBinds(crypt_str("Zero pitch"), configs.antiaim.zero_pitch.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_FAKE_FLICK_180))
		applyKeyBinds(crypt_str("180 Fake flick"), configs.antiaim.fake_flick_180.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::PING_SPIKE))
		m_keybinds.push_back(m_keybind(crypt_str("Ping spike"), std::to_string(configs.backtrack.pingSpikeAmount)));
		//applyKeyBinds(crypt_str("Ping spike"), configs.backtrack.pingSpike.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::THIRD_PERSON))
		applyKeyBinds(crypt_str("Third person"), configs.misc.thirdperson_toggle.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::AUTO_PEEK))
		applyKeyBinds(crypt_str("Quick peek"), configs.misc.automatic_peek.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::EDGE_JUMP))
		applyKeyBinds(crypt_str("Edge jump"), configs.misc.edge_jump.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::JUMP_BUG))
		applyKeyBinds(crypt_str("Jump bug"), configs.misc.jump_bug.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::LONG_JUMP))
		applyKeyBinds(crypt_str("Long jump"), configs.misc.long_jump.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::MINI_JUMP))
		applyKeyBinds(crypt_str("Mini jump"), configs.misc.mini_jump.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::FAKE_DUCK))
		applyKeyBinds(crypt_str("Fake duck"), configs.misc.fake_duck_key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::SLOW_WALK))
		applyKeyBinds(crypt_str("Slow motion"), configs.misc.slowwalk_key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::BLOCK_BOT))
		applyKeyBinds(crypt_str("Block bot"), configs.blockbot.key.mode);

	if (key_binds::get().get_key_bind_state(key_bind_features::WALK_BOT))
		applyKeyBinds(crypt_str("Walk bot"), configs.misc.walkbot_key.mode);
}

void misc::spectators_list()
{
	if (!configs.misc.spectators_list || !m_engine()->IsInGame() || !cheat.local()->is_alive())
		return;

	std::vector<std::string> spectators;
	static int spectator_x;
	static int spectator_y;
	int h = 0;
	int h2 = 0;
	int a;
	int h4;
	int spectator_width = 180;
	int spectator_height = 25;
	static int _drag_x = 300;
	static int _drag_y = 300;
	static bool _dragging = false;
	bool _click = false;

	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		_click = true;

	const Vector2D _mouse_pos = m_surface()->GetMousePosition();
	const Vector2D _screen_size = m_surface()->GetScreenSize2D();

	if (_dragging && !_click)
		_dragging = false;

	if (_dragging && _click)
	{
		configs.misc.spectatorlist_pos_x = max(min(_mouse_pos.x - _drag_x, _screen_size.x - spectator_width), 0);
		configs.misc.spectatorlist_pos_y = max(min(_mouse_pos.y - _drag_y, _screen_size.y - spectator_height), 0);
		spectator_x = configs.misc.spectatorlist_pos_x;
		spectator_y = configs.misc.spectatorlist_pos_y;
	}

	if (m_surface()->MouseInRegion(configs.misc.spectatorlist_pos_x, configs.misc.spectatorlist_pos_y, spectator_width, spectator_height) && !m_engine()->Con_IsVisible())
	{
		_dragging = true;
		_drag_x = _mouse_pos.x - configs.misc.spectatorlist_pos_x;
		_drag_y = _mouse_pos.y - configs.misc.spectatorlist_pos_y;
	}

	for (auto i = 0; i < spectators.size(); i++)
		h2 += 12;

	int rb = floor(sin(m_globals()->m_realtime * 2) * 127 + 128);
	int gb = floor(sin(m_globals()->m_realtime * 2 + 2) * 127 + 128);
	int bb = floor(sin(m_globals()->m_realtime * 2 + 4) * 127 + 128);
	int rbb = floor(sin(m_globals()->m_realtime * 3) * 127 + 128);
	int gbb = floor(sin(m_globals()->m_realtime * 3 + 2) * 127 + 128);
	int bbb = floor(sin(m_globals()->m_realtime * 3 + 4) * 127 + 128);
	Color gradient1 = Color(rb, gb, bb);
	Color gradient2 = Color(rbb, gbb, bbb);

	if (h2 < 1)
	{
		a = 0;
		h4 = 0;
	}
	else
	{
		a = 245;
		h4 = 15;
	}

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e)
			continue;

		if (e->is_alive())
			continue;

		if (e->IsDormant())
			continue;

		if (e->m_hObserverTarget().Get() != cheat.local())
			continue;

		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		spectators.push_back(player_info.name);
	}

	const auto title = crypt_str("Spectators");
	render::get().rect_filled(configs.misc.spectatorlist_pos_x, configs.misc.spectatorlist_pos_y, spectator_width, h2 + spectator_height + h4, Color(30, 30, 30, 245));
	render::get().rect(configs.misc.spectatorlist_pos_x, configs.misc.spectatorlist_pos_y, spectator_width, h2 + spectator_height + h4, Color(15, 15, 15, 245));
	render::get().gradient(configs.misc.spectatorlist_pos_x, configs.misc.spectatorlist_pos_y + 24, spectator_width, spectator_height - 24, gradient1, gradient2, GRADIENT_HORIZONTAL);
	auto text_size = render::get().text_width(fonts[NAME], title);
	render::get().text(fonts[NAME], configs.misc.spectatorlist_pos_x + (static_cast<float>(spectator_width) - text_size) / 2, configs.misc.spectatorlist_pos_y + 12, Color(255, 255, 255), HFONT_CENTERED_Y, title);
	render::get().rect_filled(configs.misc.spectatorlist_pos_x + 5, configs.misc.spectatorlist_pos_y + 30, spectator_width - 10, h2 + 5, Color(20, 20, 20, a));
	render::get().rect(configs.misc.spectatorlist_pos_x + 5, configs.misc.spectatorlist_pos_y + 30, spectator_width - 10, h2 + 5, Color(15, 15, 15, a));

	for (std::string name : spectators) {
		render::get().text(fonts[NAME], configs.misc.spectatorlist_pos_x + 10, configs.misc.spectatorlist_pos_y + 38 + h, Color(255, 255, 255), HFONT_CENTERED_Y, name.c_str());
		h += 12;
	}

	spectators.clear();
}

void misc::keybinds()
{
	if (!configs.misc.keybind_list || !m_engine()->IsInGame())
		return;

	static int kb_x;
	static int kb_y;

	static int _drag_x = 300;
	static int _drag_y = 300;
	int kb_width = 180;
	int kb_height = 25;
	static bool _dragging = false;
	bool _click = false;

	if (GetAsyncKeyState(VK_LBUTTON) & 0x8000)
		_click = true;

	const Vector2D _mouse_pos = m_surface()->GetMousePosition();
	const Vector2D _screen_size = m_surface()->GetScreenSize2D();

	if (_dragging && !_click)
		_dragging = false;

	if (_dragging && _click)
	{
		configs.misc.keybinds_pos_x = max(min(_mouse_pos.x - _drag_x, _screen_size.x - kb_width), 0);
		configs.misc.keybinds_pos_y = max(min(_mouse_pos.y - _drag_y, _screen_size.y - kb_height), 0);
		kb_x = configs.misc.keybinds_pos_x;
		kb_y = configs.misc.keybinds_pos_y;
	}

	if (m_surface()->MouseInRegion(configs.misc.keybinds_pos_x, configs.misc.keybinds_pos_y, kb_width, kb_height) && !m_engine()->Con_IsVisible())
	{
		_dragging = true;
		_drag_x = _mouse_pos.x - configs.misc.keybinds_pos_x;
		_drag_y = _mouse_pos.y - configs.misc.keybinds_pos_y;
	}

	int h = 0;
	int h2 = 0;

	for (auto& keybind : m_keybinds)
		h2 += 12;

	int rb = floor(sin(m_globals()->m_realtime * 2) * 127 + 128);
	int gb = floor(sin(m_globals()->m_realtime * 2 + 2) * 127 + 128);
	int bb = floor(sin(m_globals()->m_realtime * 2 + 4) * 127 + 128);
	int rbb = floor(sin(m_globals()->m_realtime * 3) * 127 + 128);
	int gbb = floor(sin(m_globals()->m_realtime * 3 + 2) * 127 + 128);
	int bbb = floor(sin(m_globals()->m_realtime * 3 + 4) * 127 + 128);
	Color gradient1 = Color(rb, gb, bb);
	Color gradient2 = Color(rbb, gbb, bbb);

	int a;
	int h4;

	if (h2 < 1)
	{
		a = 0;
		h4 = 0;
	}
	else
	{
		a = 245;
		h4 = 15;
	}

	const auto title = crypt_str("Keybinds");
	render::get().rect_filled(configs.misc.keybinds_pos_x, configs.misc.keybinds_pos_y, kb_width, h2 + kb_height + h4, Color(30, 30, 30, 245));
	render::get().rect(configs.misc.keybinds_pos_x, configs.misc.keybinds_pos_y, kb_width, h2 + kb_height + h4, Color(15, 15, 15, 245));
	render::get().gradient(configs.misc.keybinds_pos_x, configs.misc.keybinds_pos_y + 24, kb_width, kb_height - 24, gradient1, gradient2, GRADIENT_HORIZONTAL);
	auto text_size = render::get().text_width(fonts[NAME], title);
	render::get().text(fonts[NAME], configs.misc.keybinds_pos_x + (kb_width - text_size) / 2, configs.misc.keybinds_pos_y + 12, Color(255, 255, 255), HFONT_CENTERED_Y, title);

	for (auto& keybind : m_keybinds)
	{
		render::get().text(fonts[NAME], configs.misc.keybinds_pos_x + 10, configs.misc.keybinds_pos_y + 38 + h, Color(255, 255, 255), HFONT_CENTERED_Y, keybind.m_name.c_str());
		auto h3 = render::get().text_width(fonts[NAME], keybind.m_mode.c_str());
		render::get().text(fonts[NAME], configs.misc.keybinds_pos_x + kb_width - 10 - h3, configs.misc.keybinds_pos_y + 38 + h, Color(255, 255, 255), HFONT_CENTERED_Y, keybind.m_mode.c_str());
		h += 12;
	}

	m_keybinds.clear();
}
