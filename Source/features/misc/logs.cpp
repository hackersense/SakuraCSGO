#include "logs.h"

void eventlogs::render(ImDrawList* draw)
{
	if (notifications.empty())
		return;

	ImGui::PushFont(unicodeFont.fonts.chinese14);

	float last_y = 0;
	int x, y;
	m_engine()->GetScreenSize(x, y);
	for (size_t i = 0; i < notifications.size(); i++)
	{
		auto& notify = notifications.at(i);

		const auto pre = notify.type.c_str();
		const auto text = notify.message.c_str();
		ImVec2 textSize = ImGui::CalcTextSize(notify.type.c_str());

		std::string all_text;
		if (pre != "")
			all_text += pre;
		all_text += "";
		all_text += text;

		ImVec2 all_textSize = ImGui::CalcTextSize(all_text.c_str());

		notify.y = math::lerp(notify.y, (i * 17.f), 0.05f);

		if (notify.y > y + 17) {
			continue;
		}

		if (util::epoch_time() - notify.life_ime_local > notify.life_ime)
		{
			if ((notify.x + all_textSize.x + 16) < 0) {
				notifications.erase(notifications.begin() + i);
				continue;
			}

			notify.max_x = all_textSize.x + 16;

			notify.x = math::lerp(notify.x, (notify.max_x * -1) - 10, 0.05f);

			int procent_x = (100 * (notify.max_x + notify.x)) / notify.max_x;

			auto opacity = int((255 / 100) * procent_x);

			if (procent_x >= 0 && procent_x <= 100)
			{
				notify.c_message = Color(notify.c_message);
				notify.c_message.SetAlpha(opacity);
				notify.c_type = Color(notify.c_type);
				notify.c_type.SetAlpha(opacity);
			}
			else
			{
				notify.c_message = Color(notify.c_message);
				notify.c_message.SetAlpha(255);
				notify.c_type = Color(notify.c_type);
				notify.c_type.SetAlpha(255);
			}
		}

		float box_w = (float)fabs(0 - (all_textSize.x + 16));
		auto main_colf = ImColor(39, 39, 39, 240);
		auto main_coll = ImColor(39, 39, 39, 0);
		draw->AddRectFilledMultiColor(ImVec2(0.f, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2), main_colf, main_coll, main_coll, main_colf);
		auto main_colf2 = ImColor(39, 39, 39, 100);
		draw->AddRectFilledMultiColor(ImVec2(0.f, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y + all_textSize.y + 2), main_colf2, main_coll, main_coll, main_colf2);
		draw->AddRectFilled(ImVec2(notify.x + all_textSize.x + 16, last_y + notify.y - 1), ImVec2(notify.x + all_textSize.x + 18, last_y + notify.y + all_textSize.y + 2), ImColor(notify.c_type.r(), notify.c_type.g(), notify.c_type.b(), 150));
		if (pre == "")
			draw->AddText(ImVec2(notify.x + 2, last_y + notify.y), ImColor(notify.c_message.r(), notify.c_message.g(), notify.c_message.b()), text);
		else
		{
			draw->AddText(ImVec2(notify.x + 3, last_y + notify.y), ImColor(notify.c_type.r(), notify.c_type.g(), notify.c_type.b()), pre);
			draw->AddText(ImVec2(notify.x + 9 + textSize.x, last_y + notify.y), ImColor(notify.c_message.r(), notify.c_message.g(), notify.c_message.b()), text);
		}
	}

	ImGui::PopFont();
}

void eventlogs::events(IGameEvent* event)
{
	static auto get_hitgroup_name = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return crypt_str("Head");
		case HITGROUP_CHEST:
			return crypt_str("Chest");
		case HITGROUP_STOMACH:
			return crypt_str("Stomach");
		case HITGROUP_LEFTARM:
			return crypt_str("Left arm");
		case HITGROUP_RIGHTARM:
			return crypt_str("Right arm");
		case HITGROUP_LEFTLEG:
			return crypt_str("Left leg");
		case HITGROUP_RIGHTLEG:
			return crypt_str("Right leg");
		default:
			return crypt_str("Generic");
		}
	};

	if (configs.misc.events_to_log[EVENTLOG_HIT] && !strcmp(event->GetName(), crypt_str("player_hurt")))
	{
		auto userid = event->GetInt(crypt_str("userid")), attacker = event->GetInt(crypt_str("attacker"));

		if (!userid || !attacker)
			return;

		auto player_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker);

		PlayerInfo player_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(player_id, &player_info) || !m_engine()->GetPlayerInfo(attacker_id, &attacker_info))
			return;

		auto m_victim = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(player_id));

		std::stringstream ss;

		if (attacker_id == m_engine()->GetLocalPlayer() && player_id != m_engine()->GetLocalPlayer())
		{
			ss << crypt_str("Hit ") << player_info.getUsername() << crypt_str(" in the ") << get_hitgroup_name(event->GetInt(crypt_str("hitgroup"))) << crypt_str(" for ") << event->GetInt(crypt_str("dmg_health"));
			ss << crypt_str(" damage (") << event->GetInt(crypt_str("health")) << crypt_str(" health remaining)");

			if (configs.misc.events_to_log[EVENTLOG_HIT])
				add("Hit", ss.str());
		}
		else if (player_id == m_engine()->GetLocalPlayer() && attacker_id != m_engine()->GetLocalPlayer())
		{
			ss << crypt_str("Take ") << event->GetInt(crypt_str("dmg_health")) << crypt_str(" damage from ") << attacker_info.getUsername() << crypt_str(" in the ") << get_hitgroup_name(event->GetInt(crypt_str("hitgroup")));
			ss << crypt_str(" (") << event->GetInt(crypt_str("health")) << crypt_str(" health remaining)");

			if (configs.misc.events_to_log[EVENTLOG_HIT])
				add("Take", ss.str());
		}
	}

	if (configs.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] && !strcmp(event->GetName(), crypt_str("item_purchase")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto player_id = m_engine()->GetPlayerForUserID(userid);

		PlayerInfo player_info;

		if (!m_engine()->GetPlayerInfo(player_id, &player_info))
			return;

		auto player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(player_id));

		if (!localPlayer || !player)
			return;

		if (localPlayer.get() == player)
			cheat.globals.should_buy = 0;

		if (player->m_iTeamNum() == localPlayer->m_iTeamNum())
			return;

		std::string weapon = event->GetString(crypt_str("weapon"));

		std::stringstream ss;
		ss << player_info.getUsername() << crypt_str(" bought ") << weapon;

		add("Buy", ss.str());
	}

	if (configs.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_beginplant")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto player_id = m_engine()->GetPlayerForUserID(userid);

		PlayerInfo player_info;

		if (!m_engine()->GetPlayerInfo(player_id, &player_info))
			return;

		auto player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(player_id));

		if (!player)
			return;

		std::stringstream ss;
		ss << player_info.getUsername() << crypt_str(" has began planting the bomb");

		add("Plant", ss.str());
	}

	if (configs.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), crypt_str("bomb_begindefuse")))
	{
		auto userid = event->GetInt(crypt_str("userid"));

		if (!userid)
			return;

		auto player_id = m_engine()->GetPlayerForUserID(userid);

		PlayerInfo player_info;

		if (!m_engine()->GetPlayerInfo(player_id, &player_info))
			return;

		auto player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(player_id));

		if (!player)
			return;

		std::stringstream ss;
		ss << player_info.getUsername() << crypt_str(" has began defusing the bomb ") << (event->GetBool(crypt_str("haskit")) ? crypt_str("with defuse kit") : crypt_str("without defuse kit"));

		add("Defusing", ss.str());
	}
}

void eventlogs::add(std::string pre, std::string text, bool full_display)
{
	if (configs.misc.log_output[EVENTLOG_OUTPUT_DRAW])
	{
		if (!pre.empty())
		{
			std::string type_buf;
			type_buf += "[";
			type_buf += pre;
			type_buf += "]";
			auto color_pre = Color::LightBlue;
			if (pre == crypt_str("Hit") || pre == crypt_str("SUCCESS"))
				color_pre = Color::Green;
			else if (pre == crypt_str("Take") || pre == crypt_str("Missed") || pre == crypt_str("ERROR"))
				color_pre = Color::Red;
			else if (pre == crypt_str("Fired") || pre == crypt_str("WARNING"))
				color_pre = Color::Yellow;

			notifications.push_front(notify_t{ static_cast<float>(util::epoch_time()), 4700.f, type_buf, text, color_pre, Color::White });
		}
		else
			notifications.push_front(notify_t{ static_cast<float>(util::epoch_time()), 4700.f, "[Sakura]", text, configs.misc.log_color, Color::White });
	}
	
	if (configs.misc.log_output[EVENTLOG_OUTPUT_CONSOLE])
	{
		last_log = true;
		m_cvar()->ConsoleColorPrintf(configs.misc.log_color, (crypt_str("[") + cheat.cheatname + crypt_str("] ")).c_str());
		m_cvar()->ConsoleColorPrintf(Color::White, text.c_str());
		m_cvar()->ConsolePrintf(crypt_str("\n"));
	}

	if (configs.misc.log_output[EVENTLOG_OUTPUT_CHAT] && m_engine()->IsConnected())
		m_clientmode()->getHudChat()->printf(0, "[\x0Cc%s\x01] %s", cheat.cheatname, text);
}

void eventlogs::add(std::string text, bool full_display)
{
	add("", text, full_display);
}