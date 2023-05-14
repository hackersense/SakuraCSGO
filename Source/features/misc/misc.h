#pragma once

#include <utility>

#include "..\..\includes.hpp"

struct m_keybind
{
	std::string m_name;
	std::string m_mode;

	m_keybind(std::string name, std::string mode) :
		m_name(std::move(name)), m_mode(std::move(mode))
	{

	}
};

class misc : public singleton <misc>
{
public:
	void watermark();
	void no_duck_cooldown(CUserCmd* cmd);
	void clan_tag();
	void force_crosshair();
	void auto_crouch(CUserCmd* cmd);
	void slide_walk(CUserCmd* cmd);
	void fake_prime();
	void automatic_peek(CUserCmd* cmd, float current_yaw);
	void view_model();
	void full_bright();
	void pov_arrows(Vector origin, Color color);
	void night_mode();
	void zeus_range();
	void desync_arrows();
	void lag_hitbox();
	void ragdolls();
	void fix_animation_lod();
	void rank_reveal(CUserCmd* m_pcmd);
	void air_static_legs();
	void fast_stop(CUserCmd* m_pcmd);
	void spectators_list();
	bool double_tap(CUserCmd* m_pcmd);
	void hide_shots(CUserCmd* m_pcmd, bool should_work);
	void blockbot(CUserCmd* cmd);
	void anti_crasher();
	void anti_exploit();
	void draw_server_hitboxes();
	void killfeed_changer(IGameEvent& event);
	void preserve_killfeed(bool roundStart = false);
	void force_relay_cluster();

	void anti_afk(CUserCmd* cmd);

	void vote_revealer(IGameEvent& event);
	void vote_revealer_call(const void*& data, int& size, ECstrike15UserMessages& type);

	void get_keys();
	void keybinds();

	bool recharging_double_tap = false;

	bool double_tap_enabled = false;
	bool double_tap_key = false;
	bool double_tap_disable = false;

	bool hide_shots_enabled = false;
	bool hide_shots_key = false;
	bool hide_shots_disable = false;
private:
	std::vector<m_keybind> m_keybinds;

	void applyKeyBinds(const char* name, int mode)
	{
		switch (mode)
		{
		case HOLD:
			m_keybinds.emplace_back(name, crypt_str("[HOLD]"));
			break;
		case TOGGLE:
			m_keybinds.emplace_back(name, crypt_str("[TOGGLE]"));
			break;
		case ALWAYS:
			m_keybinds.emplace_back(name, crypt_str("[ALWAYS]"));
			break;
		}
	}
};