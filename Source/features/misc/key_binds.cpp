#include "key_binds.h"
#include "..\..\includes.hpp"
#include "misc.h"

void key_binds::update_key_bind(key_bind* key_bind, key_bind_features key_bind_id)
{
	auto is_button_down = util::is_button_down(key_bind->key);

	if (key_bind->mode != TOGGLE || (!key_bind->holding && is_button_down))
	{
		switch (static_cast<int>(key_bind_id))
		{
		case static_cast<int>(key_bind_features::DOUBLE_TAP):
			if (misc::get().recharging_double_tap)
				break;

			if (misc::get().double_tap_key)
				misc::get().double_tap_disable = key_bind->mode != TOGGLE ? !is_button_down : misc::get().double_tap_key;
			else
				misc::get().double_tap_key = key_bind->mode != TOGGLE ? is_button_down : !misc::get().double_tap_key;

			if (misc::get().double_tap_key && configs.ragebot.double_tap_key.key != configs.ragebot.hide_shots_key.key)
				misc::get().hide_shots_key = false;

			break;
		case static_cast<int>(key_bind_features::HIDE_SHOTS):
			if (misc::get().hide_shots_key)
				misc::get().hide_shots_disable = key_bind->mode != TOGGLE ? !is_button_down : misc::get().hide_shots_key;
			else
				misc::get().hide_shots_key = key_bind->mode != TOGGLE ? is_button_down : !misc::get().hide_shots_key;

			if (misc::get().hide_shots_key && configs.ragebot.hide_shots_key.key != configs.ragebot.double_tap_key.key)
				misc::get().double_tap_key = false;

			break;
		case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_BACK):
			if (key_bind->mode != TOGGLE)
			{
				if (is_button_down)
					antiaim::get().manual_side = SIDE_BACK;
				else if (antiaim::get().manual_side == SIDE_BACK)
					antiaim::get().manual_side = SIDE_NONE;
			}
			else
			{
				if (antiaim::get().manual_side == SIDE_BACK)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_BACK;
			}

			break;
		case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_LEFT):
			if (key_bind->mode != TOGGLE)
			{
				if (is_button_down)
					antiaim::get().manual_side = SIDE_LEFT;
				else if (antiaim::get().manual_side == SIDE_LEFT)
					antiaim::get().manual_side = SIDE_NONE;
			}
			else
			{
				if (antiaim::get().manual_side == SIDE_LEFT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_LEFT;
			}

			break;
		case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_RIGHT):
			if (key_bind->mode != TOGGLE)
			{
				if (is_button_down)
					antiaim::get().manual_side = SIDE_RIGHT;
				else if (antiaim::get().manual_side == SIDE_RIGHT)
					antiaim::get().manual_side = SIDE_NONE;
			}
			else
			{
				if (antiaim::get().manual_side == SIDE_RIGHT)
					antiaim::get().manual_side = SIDE_NONE;
				else
					antiaim::get().manual_side = SIDE_RIGHT;
			}

			break;
		default:
			switch (key_bind->mode)
			{
			case HOLD:
				keys[static_cast<int>(key_bind_id)] = is_button_down;
				break;
			case TOGGLE:
				keys[static_cast<int>(key_bind_id)] = !keys[static_cast<int>(key_bind_id)];
				break;
			case ALWAYS:
				keys[static_cast<int>(key_bind_id)] = true;
				break;
			}
			break;
		}
		key_bind->holding = true;
	}
	else if (key_bind->mode == TOGGLE && key_bind->holding && !is_button_down)
		key_bind->holding = false;

	mode[static_cast<int>(key_bind_id)] = (key_bind_mode) key_bind->mode;
}

void key_binds::initialize_key_binds()
{
	for (auto i = 0; i < (static_cast<int>(key_bind_features::KEYBIND_MAX) - 1); i++)
	{
		keys[i] = false;

		if (i == static_cast<int>(key_bind_features::DOUBLE_TAP) || i == static_cast<int>(key_bind_features::HIDE_SHOTS) || i >= static_cast<int>(key_bind_features::ANTIAIM_MANUAL_BACK) && i <= static_cast<int>(key_bind_features::ANTIAIM_MANUAL_RIGHT))
			mode[i] = TOGGLE;
		else
			mode[i] = HOLD;
	}
}

void key_binds::update_key_binds()
{
	update_key_bind(&configs.legitbot.trigger, key_bind_features::TRIGGER_BOT);
	update_key_bind(&configs.legitbot.key, key_bind_features::LEGIT_BOT);
	update_key_bind(&configs.ragebot.double_tap_key, key_bind_features::DOUBLE_TAP);
	update_key_bind(&configs.ragebot.hide_shots_key, key_bind_features::HIDE_SHOTS);
	update_key_bind(&configs.ragebot.safe_point_key, key_bind_features::SAFE_POINT);
	update_key_bind(&configs.ragebot.body_aim_key, key_bind_features::BODY_AIM);
	update_key_bind(&configs.ragebot.resolver_override, key_bind_features::RESOLVER_OVERRIDE);
	update_key_bind(&configs.ragebot.damage_override_key, key_bind_features::DAMAGE_OVERRIDE);
	update_key_bind(&configs.antiaim.manual_back, key_bind_features::ANTIAIM_MANUAL_BACK);
	update_key_bind(&configs.antiaim.manual_left, key_bind_features::ANTIAIM_MANUAL_LEFT);
	update_key_bind(&configs.antiaim.manual_right, key_bind_features::ANTIAIM_MANUAL_RIGHT);
	update_key_bind(&configs.antiaim.flip_desync, key_bind_features::ANTIAIM_DESYNC_FLIP);
	update_key_bind(&configs.antiaim.zero_pitch, key_bind_features::ANTIAIM_ZERO_PITCH);
	update_key_bind(&configs.antiaim.fake_flick_180, key_bind_features::ANTIAIM_FAKE_FLICK_180);
	update_key_bind(&configs.misc.thirdperson_toggle, key_bind_features::THIRD_PERSON);
	update_key_bind(&configs.misc.automatic_peek, key_bind_features::AUTO_PEEK);
	update_key_bind(&configs.misc.edge_jump, key_bind_features::EDGE_JUMP);
	update_key_bind(&configs.misc.jump_bug, key_bind_features::JUMP_BUG);
	update_key_bind(&configs.misc.long_jump, key_bind_features::LONG_JUMP);
	update_key_bind(&configs.misc.mini_jump, key_bind_features::MINI_JUMP);
	update_key_bind(&configs.misc.fake_duck_key, key_bind_features::FAKE_DUCK);
	update_key_bind(&configs.misc.slowwalk_key, key_bind_features::SLOW_WALK);
	update_key_bind(&configs.blockbot.key, key_bind_features::BLOCK_BOT);
	update_key_bind(&configs.misc.walkbot_key, key_bind_features::WALK_BOT);
	update_key_bind(&configs.backtrack.pingSpike, key_bind_features::PING_SPIKE);
}

bool key_binds::get_key_bind_state(key_bind_features key_bind)
{
	return keys[static_cast<int>(key_bind)];
}

void key_binds::set_key_bind_state(int key_bind_id, bool state)
{
	keys[static_cast<int>(key_bind_id)] = state;
}

bool key_binds::get_key_bind_state_lua(int key_bind_id)
{
	if (key_bind_id < 0 || key_bind_id > (static_cast<int>(key_bind_features::KEYBIND_MAX) - 1))
		return false;

	switch (key_bind_id)
	{
	case static_cast<int>(key_bind_features::DOUBLE_TAP):
		return misc::get().double_tap_key;
	case static_cast<int>(key_bind_features::HIDE_SHOTS):
		return misc::get().hide_shots_key;
	case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_BACK):
		return antiaim::get().manual_side == SIDE_BACK;
	case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_LEFT):
		return antiaim::get().manual_side == SIDE_LEFT;
	case static_cast<int>(key_bind_features::ANTIAIM_MANUAL_RIGHT):
		return antiaim::get().manual_side == SIDE_RIGHT;
	default:
		return keys[key_bind_id];
	}
}

bool key_binds::get_key_bind_mode(int key_bind_id)
{
	return mode[key_bind_id];
}
