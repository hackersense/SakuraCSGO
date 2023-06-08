#include "..\hooks.hpp"
#include "..\..\features\serverside\engine_prediction.h"
#include "..\..\features\ragebot\ragebot.h"
#include "..\..\features\legitbot\legitbot.h"
#include "..\..\features\legitbot\backtrack.h"
#include "..\..\features\movement\bunnyhop.h"
#include "..\..\features\movement\movement.h"
#include "..\..\features\movement\airstrafe.h"
#include "..\..\features\movement\slowwalk.h"
#include "..\..\features\movement\walkbot.h"
#include "..\..\features\misc\misc.h"
#include "..\..\features\visuals\grenade_prediction.h"
#include "..\..\features\ragebot\knifebot.h"
#include "..\..\features\ragebot\zeusbot.h"
#include "..\..\features\serverside\local_animations.h"

#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))

bool __stdcall Hooks::hooked_createmove(float inputSampleTime, int sequenceNumber, bool active, CUserCmd* cmd, bool& sendPacket)
{
	// /*
	if (!cheat.available())
		return active;

	misc::get().rank_reveal(cmd);
	misc::get().clan_tag();
	backtrack::get().updateIncomingSequences();

	static auto sky = m_cvar()->FindVar(crypt_str("r_3dsky"));
	static auto shadows = m_cvar()->FindVar(crypt_str("cl_csm_enabled"));
	sky->SetValue(~configs.esp.removals[REMOVALS_3DSKY]);
	shadows->SetValue(~configs.esp.removals[REMOVALS_SHADOWS]);

	if (!cheat.local()->is_alive())
		return active;

	cheat.globals.weapon = cheat.local()->m_hActiveWeapon().Get();
	// */
	if (!cheat.globals.weapon)
		return active;

	cheat.globals.in_createmove = true;
	cheat.set_command(cmd);
	cheat.globals.original = cmd->m_viewangles;

	/*
	
	if (Hooks::menu_open && cheat.globals.focused_on_input)
	{
		cmd->m_buttons = 0;
		cmd->m_forwardmove = cmd->m_sidemove = cmd->m_upmove = 0.0f;
	}

	*/

	misc::get().force_crosshair();

	if (cheat.globals.is_shifting)
	{
		sendPacket = cheat.globals.need_shift_tickbase == 1;
		cmd->m_buttons &= ~(IN_ATTACK | IN_ATTACK2);
		cmd->m_buttons &= ~IN_JUMP;

		if (cmd->m_sidemove > 5.0f)
			cmd->m_sidemove = 450.0f;
		else if (cmd->m_sidemove < -5.0f)
			cmd->m_sidemove = -450.0f;

		if (cmd->m_forwardmove > 5.0f)
			cmd->m_forwardmove = 450.0f;
		else if (cmd->m_forwardmove < -5.0f)
			cmd->m_forwardmove = -450.0f;

		//const auto speed = util::get_speed_move(cmd);
		//cmd->m_forwardmove = speed.x;
		//cmd->m_sidemove = speed.y;

		//if (configs.misc.anti_untrusted)
		//{
		//	math::normalize_angles(cmd->m_viewangles);
		//	cmd->m_viewangles.z = 0.0f;
		//}
		//else
		//	cmd->m_viewangles.y = math::normalize_yaw(cmd->m_viewangles.y);

		cheat.globals.in_createmove = false;
		return false;
	}

	const auto tickAllowed = ((m_gamerules()->m_bIsValveDS() && !configs.misc.anti_untrusted) || (misc::get().hide_shots_enabled && !misc::get().double_tap_enabled)) ? (m_gamerules()->m_bIsValveDS() ? 6 : 8) : 16;

	if (cheat.globals.should_recharge)
	{
		++cheat.globals.ticks_allowed;

		cmd->m_tickcount = INT_MAX;
		cmd->m_forwardmove = cmd->m_sidemove = cmd->m_upmove = 0.0f;
		cmd->m_buttons &= ~(IN_ATTACK | IN_ATTACK2);
		cmd->m_buttons &= ~IN_JUMP;

		if (cheat.globals.ticks_allowed >= tickAllowed)
		{
			cheat.globals.should_recharge = false;
			sendPacket = true;
		}
		else
			sendPacket = false;

		return false;
	}

	if (cheat.globals.ticks_allowed < tickAllowed && (misc::get().double_tap_key || misc::get().hide_shots_key) && !cheat.globals.fakeducking)
		cheat.globals.should_recharge = true;

	cheat.globals.backup_tickbase = cheat.local()->m_nTickBase();
	cheat.globals.fixed_tickbase = (cheat.globals.next_tickbase_shift > 0) ? cheat.globals.backup_tickbase - cheat.globals.next_tickbase_shift : cheat.globals.backup_tickbase;

	util::server_time(cmd);

	/*

	if (Hooks::menu_open)
	{
		cmd->m_buttons &= ~IN_ATTACK;
		cmd->m_buttons &= ~IN_ATTACK2;
	}

	*/

	if (configs.ragebot.enabled && (cheat.globals.weapon->is_pistol() || cheat.globals.weapon->is_sniper() || cheat.globals.weapon->is_rifle() || cheat.globals.weapon->is_shotgun() || cheat.globals.weapon->is_smg() || cheat.globals.weapon->is_knife()) && !cheat.globals.weapon->can_fire(true)) //  && !antiaim::get().freeze_check
	{
		if (cmd->m_buttons & IN_ATTACK)
			cmd->m_buttons &= ~IN_ATTACK;

		if (cmd->m_buttons & IN_ATTACK2)
			cmd->m_buttons &= ~IN_ATTACK2;
	}

	if (configs.legitbot.enabled && configs.legitbot.autopistol && cheat.globals.weapon->is_pistol() && !cheat.globals.weapon->can_fire(true)) //  && !antiaim::get().freeze_check
	{
		if (cmd->m_buttons & IN_ATTACK)
			cmd->m_buttons &= ~IN_ATTACK;

		if (cmd->m_buttons & IN_ATTACK2)
			cmd->m_buttons &= ~IN_ATTACK2;
	}

	/*
	if (cmd->m_buttons & IN_FORWARD && cmd->m_buttons & IN_BACK)
	{
		cmd->m_buttons &= ~IN_FORWARD;
		cmd->m_buttons &= ~IN_BACK;
	}

	if (cmd->m_buttons & IN_MOVELEFT && cmd->m_buttons & IN_MOVERIGHT)
	{
		cmd->m_buttons &= ~IN_MOVELEFT;
		cmd->m_buttons &= ~IN_MOVERIGHT;
	}
	*/

	cheat.send_packet = true;
	cheat.globals.tickbase_shift = 0;
	cheat.globals.double_tap_fire = false;
	cheat.globals.force_send_packet = false;
	cheat.globals.exploits = misc::get().double_tap_key || misc::get().hide_shots_key;
	cheat.globals.current_weapon = cheat.globals.weapon->get_weapon_group(configs.ragebot.enabled);
	cheat.globals.slowwalking = false;
	cheat.globals.original_forwardmove = cmd->m_forwardmove;
	cheat.globals.original_sidemove = cmd->m_sidemove;

	antiaim::get().breaking_lby = false;

	static auto previousViewAngles{ cmd->m_viewangles };
	auto currentViewAngles{ cmd->m_viewangles };
	//auto currentPredictedTick{ m_prediction()->split->m_iCommandsPredicted - 1 };

	walkbot::get().move(cmd);

	misc::get().fast_stop(cmd);

	misc::get().anti_afk(cmd);

	bunnyhop::get().create_move(cmd);

	misc::get().slide_walk(cmd);

	misc::get().air_static_legs();

	misc::get().no_duck_cooldown(cmd);

	misc::get().auto_crouch(cmd);

	if (configs.misc.crouch_in_air && !(cheat.local()->m_fFlags() & FL_ONGROUND))
		cmd->m_buttons |= IN_DUCK;

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_preprediction")))
			current.func(cmd);

	engineprediction::get().prediction_data.reset();
	engineprediction::get().predict(cmd);

	local_animations::get().update_prediction_animations();

	cheat.globals.eye_pos = cheat.local()->get_eye_position();

	grenade_prediction::get().run(cmd);

	if (configs.misc.airstrafe)
		airstrafe::get().create_move(cmd, currentViewAngles);

	misc::get().blockbot(cmd);

	movement::get().longjump(cmd);
	movement::get().jumpbug(cmd);

	movement::get().edgejump(cmd);

	if (key_binds::get().get_key_bind_state(key_bind_features::SLOW_WALK))
		slowwalk::get().create_move(cmd);

	if (configs.ragebot.enabled && !cheat.globals.weapon->is_non_aim() && engineprediction::get().backup_data.flags & FL_ONGROUND && cheat.local()->m_fFlags() & FL_ONGROUND)
		slowwalk::get().create_move(cmd, 0.95f + 0.003125f * (16.f - m_clientstate()->m_iChokedCommands));

	if (!cheat.globals.should_recharge)
		fakelag::get().create_move(cmd);

	cheat.globals.aimbot_working = false;
	cheat.globals.revolver_working = false;

	auto& backup_velocity = cheat.local()->m_vecVelocity();
	auto& backup_abs_velocity = cheat.local()->m_vecAbsVelocity();

	cheat.local()->m_vecVelocity() = engineprediction::get().backup_data.velocity;
	cheat.local()->m_vecAbsVelocity() = engineprediction::get().backup_data.velocity;

	cheat.globals.weapon->update_accuracy_penality();

	cheat.local()->m_vecVelocity() = backup_velocity;
	cheat.local()->m_vecAbsVelocity() = backup_abs_velocity;

	cheat.globals.inaccuracy = cheat.globals.weapon->get_inaccuracy();
	cheat.globals.spread = cheat.globals.weapon->get_spread();

	backtrack::get().run(cmd);
	ragebot::get().run(cmd);
	legitbot::get().run(cmd);
	zeusbot::get().run(cmd);
	knifebot::get().run(cmd);

	misc::get().automatic_peek(cmd, currentViewAngles.y);

	antiaim::get().desync_angle = 0.0f;
	antiaim::get().create_move(cmd);

	if (m_clientstate()->m_iChokedCommands >= (m_gamerules()->m_bIsValveDS() ? 6 : 16))
	{
		cheat.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (cheat.globals.should_send_packet)
	{
		cheat.globals.force_send_packet = true;
		cheat.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (cheat.globals.should_choke_packet && !(cmd->m_buttons & IN_ATTACK))
	{
		cheat.globals.should_choke_packet = false;
		cheat.globals.should_send_packet = true;
		cheat.send_packet = false;
	}

	if (!cheat.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (cmd->m_buttons & IN_ATTACK && cheat.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			cheat.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !cheat.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

		if ((cmd->m_buttons & IN_ATTACK && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER) || revolver_shoot)
		{
			static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));

			if (configs.ragebot.enabled && cheat.globals.weapon->requires_recoil_control())
			{
				cmd->m_viewangles.x -= cheat.local()->m_aimPunchAngle().x * weapon_recoil_scale->GetFloat();
				cmd->m_viewangles.y -= cheat.local()->m_aimPunchAngle().y * weapon_recoil_scale->GetFloat();
			}

			if (!cheat.globals.fakeducking)
			{
				cheat.globals.force_send_packet = true;
				cheat.globals.should_choke_packet = true;
				cheat.send_packet = true;
				fakelag::get().started_peeking = false;
			}

			ragebot::get().last_shoot_position = cheat.globals.eye_pos;

			if (!double_tap_aim_check)
				cheat.globals.double_tap_aim = false;
		}
	}
	else if (!cheat.globals.fakeducking && cheat.globals.weapon->is_knife() && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2))
	{
		cheat.globals.force_send_packet = true;
		cheat.globals.should_choke_packet = true;
		cheat.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (!cheat.globals.fakeducking && (misc::get().hide_shots_disable || misc::get().double_tap_disable))
	{
		cheat.globals.force_send_packet = true;
		cheat.globals.should_choke_packet = true;
		cheat.send_packet = true;
		fakelag::get().started_peeking = false;
	}

	if (cheat.globals.fakeducking)
		cheat.globals.force_send_packet = cheat.send_packet;

	for (auto& backup : ragebot::get().backup)
		backup.adjust_player();

	auto backup_ticks_allowed = cheat.globals.ticks_allowed;

	if (misc::get().double_tap(cmd))
	{
		misc::get().hide_shots(cmd, false);
	}
	else
	{
		cheat.globals.ticks_allowed = backup_ticks_allowed;
		misc::get().hide_shots(cmd, true);
	}

	if (!cheat.globals.weapon->is_non_aim())
	{
		auto double_tap_aim_check = false;

		if (cmd->m_buttons & IN_ATTACK && cheat.globals.double_tap_aim_check)
		{
			double_tap_aim_check = true;
			cheat.globals.double_tap_aim_check = false;
		}

		auto revolver_shoot = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !cheat.globals.revolver_working && (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2);

		if (!double_tap_aim_check && cmd->m_buttons & IN_ATTACK && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER || revolver_shoot)
			cheat.globals.double_tap_aim = false;
	}

	if ((m_globals()->m_tickcount - cheat.globals.last_aimbot_shot) > 16)
	{
		cheat.globals.double_tap_aim = false;
		cheat.globals.double_tap_aim_check = false;
	}

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_prediction")))
			current.func(cmd);

	//engineprediction::get().finish();

	//if (cheat.local()->is_alive())
	//	memory->restoreEntityToPredictedFrame(0, currentPredictedTick);

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_restoreprediction")))
			current.func(cmd);

	util::fix_mouse_delta(cmd);

	if (configs.misc.anti_untrusted)
	{
		math::normalize_angles(cmd->m_viewangles);
		cmd->m_viewangles.z = 0.0f;
	}
	else
		cmd->m_viewangles.y = math::normalize_yaw(cmd->m_viewangles.y);

	util::movement_fix(currentViewAngles, cmd);

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_createmove")))
			current.func(cmd);

	previousViewAngles = cmd->m_viewangles;

	if (cheat.globals.should_recharge)
		cheat.send_packet = true;

	static auto previous_ticks_allowed = cheat.globals.ticks_allowed;

	if (cheat.send_packet && m_clientstate()->m_pNetChannel)
	{
		auto choked_packets = m_clientstate()->m_pNetChannel->m_nChokedPackets;

		if (choked_packets >= 0)
		{
			auto ticks_allowed = cheat.globals.ticks_allowed;
			auto command_number = cmd->m_command_number - choked_packets;

			do
			{
				auto command = &m_input()->m_pCommands[cmd->m_command_number - MULTIPLAYER_BACKUP * (command_number / MULTIPLAYER_BACKUP) - choked_packets];

				if (!command || (command->m_tickcount > (m_globals()->m_tickcount + 72)))
				{
					if (--ticks_allowed < 0)
						ticks_allowed = 0;

					cheat.globals.ticks_allowed = ticks_allowed;
				}

				++command_number;
				--choked_packets;
			} while (choked_packets >= 0);
		}
	}

	if (cheat.globals.ticks_allowed > 17)
		cheat.globals.ticks_allowed = math::clamp(cheat.globals.ticks_allowed - 1, 0, 17);

	if (previous_ticks_allowed && !cheat.globals.ticks_allowed)
		cheat.globals.ticks_choke = 16;

	previous_ticks_allowed = cheat.globals.ticks_allowed;

	if (cheat.globals.ticks_choke)
	{
		cheat.send_packet = cheat.globals.force_send_packet;
		--cheat.globals.ticks_choke;
	}

	auto& correct = cheat.globals.data.emplace_front();

	correct.command_number = cmd->m_command_number;
	correct.choked_commands = m_clientstate()->m_iChokedCommands + 1;
	correct.tickcount = m_globals()->m_tickcount;

	if (cheat.send_packet)
		cheat.globals.choked_number.clear();
	else
		cheat.globals.choked_number.emplace_back(correct.command_number);

	while (cheat.globals.data.size() > static_cast<int>(2.0f / m_globals()->m_intervalpertick))
		cheat.globals.data.pop_back();

	auto& out = cheat.globals.commands.emplace_back();

	out.is_outgoing = cheat.send_packet;
	out.is_used = false;
	out.command_number = cmd->m_command_number;
	out.previous_command_number = 0;

	while (cheat.globals.commands.size() > static_cast<int>(1.0f / m_globals()->m_intervalpertick))
		cheat.globals.commands.pop_front();

	if (!cheat.send_packet && (!m_gamerules()->m_bIsValveDS() || !configs.misc.anti_untrusted))
	{
		auto net_channel = m_clientstate()->m_pNetChannel;

		if (net_channel->m_nChokedPackets > 0 && !(net_channel->m_nChokedPackets % 4))
		{
			auto backup_choke = net_channel->m_nChokedPackets;
			net_channel->m_nChokedPackets = 0;

			net_channel->SendDatagram();
			--net_channel->m_nOutSequenceNr;

			net_channel->m_nChokedPackets = backup_choke;
		}
	}

	if (cheat.send_packet && !cheat.globals.should_send_packet && (!cheat.globals.should_choke_packet || (!misc::get().hide_shots_enabled && !cheat.globals.double_tap_fire)))
	{
		local_animations::get().local_data.fake_angles = cmd->m_viewangles;
		local_animations::get().local_data.real_angles = local_animations::get().local_data.stored_real_angles;
	}

	if (!antiaim::get().breaking_lby)
		local_animations::get().local_data.stored_real_angles = cmd->m_viewangles;

	if (cheat.send_packet && cheat.globals.should_send_packet)
		cheat.globals.should_send_packet = false;

	if (configs.misc.buybot_enabled && cheat.globals.should_buy)
	{
		--cheat.globals.should_buy;

		if (!cheat.globals.should_buy)
		{
			std::string buy;

			switch (configs.misc.buybot1)
			{
			case 1:
				buy += crypt_str("buy g3sg1; ");
				break;
			case 2:
				buy += crypt_str("buy awp; ");
				break;
			case 3:
				buy += crypt_str("buy ssg08; ");
				break;
			}

			switch (configs.misc.buybot2)
			{
			case 1:
				buy += crypt_str("buy elite; ");
				break;
			case 2:
				buy += crypt_str("buy deagle; buy revolver; ");
				break;
			}

			if (configs.misc.buybot3[BUY_ARMOR])
				buy += crypt_str("buy vesthelm; buy vest; ");

			if (configs.misc.buybot3[BUY_TASER])
				buy += crypt_str("buy taser; ");

			if (configs.misc.buybot3[BUY_GRENADES])
				buy += crypt_str("buy molotov; buy hegrenade; buy smokegrenade; buy flashbang; buy flashbang; buy decoy; ");

			if (configs.misc.buybot3[BUY_DEFUSER])
				buy += crypt_str("buy defuser; ");

			m_engine()->ExecuteClientCmd(buy.c_str());
		}
	}

	cheat.globals.in_createmove = false;

	sendPacket = cheat.send_packet;
	return false;
}

void __stdcall Hooks::CHLCreateMove(int sequenceNumber, float inputSampleTime, bool active, bool& sendPacket)
{
	client_hook.callOriginal<void, 22>(sequenceNumber, inputSampleTime, active);

	//cheat.globals.in_createmove = false;

	CUserCmd* cmd = m_input()->GetUserCmd(sequenceNumber);
	CVerifiedUserCmd* verified = m_input()->GetVerifiedUserCmd(sequenceNumber);
	if (!cmd || !cmd->m_command_number || !verified)
		return;

	hooked_createmove(inputSampleTime, sequenceNumber, active, cmd, sendPacket);

	verified->m_cmd = *cmd;
	verified->m_crc = cmd->GetChecksum();
}
