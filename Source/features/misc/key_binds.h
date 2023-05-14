#pragma once

#include <unordered_map>
#include "..\..\sdk\interfaces\IInputSystem.hpp"
#include "..\..\utils\singleton.h"

enum key_bind_mode : int
{
	HOLD,
	TOGGLE,
	ALWAYS
};

enum class key_bind_features : int
{
	TRIGGER_BOT = 0,
	LEGIT_BOT,
	DOUBLE_TAP,
	HIDE_SHOTS,
	SAFE_POINT,
	DAMAGE_OVERRIDE,
	ANTIAIM_MANUAL_BACK,
	ANTIAIM_MANUAL_LEFT,
	ANTIAIM_MANUAL_RIGHT,
	ANTIAIM_DESYNC_FLIP,
	ANTIAIM_ZERO_PITCH,
	ANTIAIM_FAKE_FLICK_180,
	THIRD_PERSON,
	AUTO_PEEK,
	EDGE_JUMP,
	JUMP_BUG,
	LONG_JUMP,
	MINI_JUMP,
	FAKE_DUCK,
	SLOW_WALK,
	BODY_AIM,
	RESOLVER_OVERRIDE,
	BLOCK_BOT,
	WALK_BOT,
	PING_SPIKE,
	KEYBIND_MAX
};

struct key_bind
{
	int key = KEY_NONE;
	int mode = HOLD;
	bool holding = false;

	explicit key_bind(key_bind_mode mode = HOLD)
	{
		this->mode = mode;
	}

	key_bind(ButtonCode_t key, key_bind_mode mode)
	{
		this->key = key;
		this->mode = mode;
	}
};

class key_binds : public singleton <key_binds>
{
	std::unordered_map<int, bool> keys;
	std::unordered_map<int, key_bind_mode> mode;

	void update_key_bind(key_bind* key_bind, key_bind_features key_bind_id);
public:
	void initialize_key_binds();
	void update_key_binds(); 
	bool get_key_bind_state(key_bind_features key_bind);
	void set_key_bind_state(int key_bind_id, bool state);
	bool get_key_bind_state_lua(int key_bind_id);
	bool get_key_bind_mode(int key_bind_id);
};