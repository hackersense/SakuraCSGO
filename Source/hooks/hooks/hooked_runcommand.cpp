#include "..\hooks.hpp"
#include "..\..\features\serverside\engine_prediction.h"
#include "..\..\features\misc\misc.h"
#include "..\..\features\misc\logs.h"

#define FRAME_ADDRESS() (std::uintptr_t(_AddressOfReturnAddress()) - sizeof(std::uintptr_t))

/*
void FixAttackPacket(CUserCmd* m_pCmd, bool m_bPredict)
{
	static bool m_bLastAttack = false;
	static bool m_bInvalidCycle = false;
	static float m_flLastCycle = 0.f;

	if (m_bPredict)
	{
		m_bLastAttack = m_pCmd->m_weaponselect || (m_pCmd->m_buttons & IN_ATTACK);
		m_flLastCycle = localPlayer->m_flCycle();
	}
	else if (m_bLastAttack && !m_bInvalidCycle)
		m_bInvalidCycle = localPlayer->m_flCycle() == 0.f && m_flLastCycle > 0.f;

	if (m_bInvalidCycle)
		localPlayer->m_flCycle() = m_flLastCycle;
}
*/

void __fastcall Hooks::hooked_runcommand(void* ecx, void* edx, player_t* player, CUserCmd* m_pcmd, IMoveHelper* move_helper) noexcept
{
	if (!m_pcmd || !player || player != localPlayer.get())
		return prediction_hook.callOriginal<void, void*, 19>(ecx, player, m_pcmd, move_helper);

	if (m_pcmd->m_tickcount > m_globals()->m_tickcount + m_cvar()->FindVar("sv_max_usercmd_future_ticks")->GetInt())
	{
		m_pcmd->m_predicted = true;
		player->set_abs_origin(player->m_vecOrigin());

		if (m_globals()->m_frametime > 0.0f && !m_prediction()->m_bEnginePaused)//!m_engine()->IsPaused())
			++player->m_nTickBase();

		return;
	}

	if (configs.ragebot.enabled && player->is_alive())
	{
		auto weapon = player->getActiveWeapon();

		if (weapon)
		{
			static float tickbase_records[MULTIPLAYER_BACKUP];
			static bool in_attack[MULTIPLAYER_BACKUP];
			static bool can_shoot[MULTIPLAYER_BACKUP];

			tickbase_records[m_pcmd->m_command_number % MULTIPLAYER_BACKUP] = player->m_nTickBase();
			in_attack[m_pcmd->m_command_number % MULTIPLAYER_BACKUP] = m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2;
			can_shoot[m_pcmd->m_command_number % MULTIPLAYER_BACKUP] = weapon->can_fire(false);

			if (weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER) 
			{
				auto postpone_fire_ready_time = FLT_MAX;
				auto tickrate = static_cast<int>(1.0f / m_globals()->m_intervalpertick);

				if (tickrate >> 1 > 1)
				{
					auto command_number = m_pcmd->m_command_number - 1;
					auto shoot_number = 0;
			
					for (auto i = 1; i < tickrate >> 1; i++)
					{
						shoot_number = command_number;

						if (!in_attack[command_number % MULTIPLAYER_BACKUP] || !can_shoot[command_number % MULTIPLAYER_BACKUP])
							break;

						--command_number;
					}

					if (shoot_number)
					{
						auto tick = 1 - static_cast<int>(-0.03348f / m_globals()->m_intervalpertick);

						if (m_pcmd->m_command_number - shoot_number >= tick)
							postpone_fire_ready_time = TICKS_TO_TIME(tickbase_records[(tick + shoot_number) % MULTIPLAYER_BACKUP]) + 0.2f;
					}
				}

				weapon->m_flPostponeFireReadyTime() = postpone_fire_ready_time;
			}
		}

		auto backup_velocity_modifier = player->m_flVelocityModifier();

		//FixAttackPacket(m_pcmd, true);

		if (cheat.globals.in_createmove && m_pcmd->m_command_number == m_clientstate()->m_nLastCommandAck + 1)
			player->m_flVelocityModifier() = cheat.globals.last_velocity_modifier;

		prediction_hook.callOriginal<void, void*, 19>(ecx, player, m_pcmd, move_helper);

		if (!cheat.globals.in_createmove)
			player->m_flVelocityModifier() = backup_velocity_modifier;

		//FixAttackPacket(m_pcmd, false);

		//engineprediction::get().store_netvars();
	}
	else
		prediction_hook.callOriginal<void, void*, 19>(ecx, player, m_pcmd, move_helper);
}

bool __stdcall Hooks::hooked_inprediction() noexcept
{
	static auto maintain_sequence_transitions = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 74 17 8B 87"));
	static auto setupbones_timing = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 74 0A F3 0F 10 05 ? ? ? ? EB 05"));
	static auto calcplayerview_return = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 0B 8B 0D ? ? ? ? 8B 01 FF 50 4C"));

	if (maintain_sequence_transitions && cheat.globals.setuping_bones && uintptr_t(_ReturnAddress()) == maintain_sequence_transitions)
		return true;

	if (setupbones_timing && uintptr_t(_ReturnAddress()) == setupbones_timing)
		return false;

	if (m_engine()->IsInGame() && uintptr_t(_ReturnAddress()) == calcplayerview_return)
		return true;

	return prediction_hook.callOriginal<bool, 14>();
}

void __fastcall Hooks::hooked_gamemovement(void* ecx, void* edx, player_t* ent, CMoveData* move_data) noexcept
{
	move_data->m_bGameCodeMovedPlayer = false;
	return gamemovement_hook.callOriginal<void, void*, 1>(ecx, ent, move_data);
}

void __stdcall Hooks::hooked_onjump(float stamina) noexcept
{
	gamemovement_hook.callOriginal<void, 32>(stamina);

	if (localPlayer && localPlayer->is_alive() && localPlayer->get_animation_state())
		localPlayer->get_animation_state()->doAnimationEvent(PLAYERANIMEVENT_JUMP);
}

void WriteUserCmd(bufferWrite* buf, CUserCmd* incmd, CUserCmd* outcmd) noexcept
{
	using WriteUserCmd_t = void(__fastcall*)(bufferWrite*, CUserCmd*, CUserCmd*);

	//static auto Fn = (WriteUserCmd_t)util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 51 53 56 8B D9"));
	static auto Fn = (WriteUserCmd_t)util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 51 53 56 8B D9 8B 0D"));

	__asm
	{
		mov     ecx, buf
		mov     edx, incmd
		push    outcmd
		call    Fn
		add     esp, 4
	}
}

bool __fastcall Hooks::hooked_writeusercmddeltatobuffer(void* ecx, void* edx, int slot, bufferWrite* buffer, int from, int to, bool is_new_command) noexcept
{
	static auto original_fn = client_hook.getOriginal<bool, 24>(slot, buffer, from, to, is_new_command);

	if (!localPlayer
		|| !m_engine()->IsConnected()
		|| !localPlayer->is_alive()
		//|| m_gamerules()->m_bFreezePeriod()
		//|| localPlayer->m_bGunGameImmunity()
		//|| localPlayer->m_fFlags() & FL_FROZEN
		|| !cheat.globals.tickbase_shift)
		return original_fn(ecx, slot, buffer, from, to, is_new_command);
	
	//if (from != -1)
	//	return true;

	const int extraCommands = cheat.globals.tickbase_shift;
	cheat.globals.tickbase_shift = 0;

	int* backupCommandsPointer = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x30);
	int* newCommandsPointer = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x2C);

	const int newCommands = *newCommandsPointer;
	const int nextCommand = m_clientstate()->m_iChokedCommands + m_clientstate()->m_nLastOutgoingCommand + 1;

	*backupCommandsPointer = 0;

	for (to = nextCommand - newCommands + 1; to <= nextCommand; to++)
	{
		if (!original_fn(ecx, slot, buffer, from, to, true))
			return false;

		from = to;
	}

	*newCommandsPointer = newCommands + extraCommands;

	CUserCmd* cmd = m_input()->GetUserCmd(slot, from);
	if (!cmd)
		return true;

	CUserCmd toCmd = *cmd;
	CUserCmd fromCmd = *cmd;

	toCmd.m_command_number++;
	toCmd.m_tickcount += 200;
	//toCmd.m_tickcount += (static_cast<int>(1.0f / m_globals()->m_intervalpertick) * 3);

	for (int i = 0; i < extraCommands; i++)
	{
		WriteUserCmd(buffer, &toCmd, &fromCmd);

		toCmd.m_tickcount++;
		toCmd.m_command_number++;

		fromCmd.m_tickcount = toCmd.m_tickcount - 1;
		fromCmd.m_command_number = toCmd.m_command_number - 1;
	}

	return true;

	/*
	auto final_from = -1;

	int* backup_commands = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x30);
	int* new_commands = reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(buffer) - 0x2C);

	//auto backup_commands = reinterpret_cast<int*>(FRAME_ADDRESS() + 0xFD8);
	//auto new_commands = reinterpret_cast<int*>(FRAME_ADDRESS() + 0xFDC);

	auto newcmds = *new_commands;
	auto shift = cheat.globals.tickbase_shift;

	cheat.globals.tickbase_shift = 0;
	*backup_commands = 0;

	auto choked_modifier = newcmds + shift;

	if (choked_modifier > 62)
		choked_modifier = 62;

	*new_commands = choked_modifier;

	auto next_cmdnr = m_clientstate()->m_iChokedCommands + m_clientstate()->m_nLastOutgoingCommand + 1;
	auto final_to = next_cmdnr - newcmds + 1;

	if (final_to <= next_cmdnr)
	{
		while (original_fn(ecx, slot, buffer, final_from, final_to, true))
		{
			final_from = final_to++;

			if (final_to > next_cmdnr)
				goto next_cmd;
		}

		return false;
	}
next_cmd:

	auto user_cmd = m_input()->GetUserCmd(final_from);

	if (!user_cmd)
		return true;

	CUserCmd to_cmd;
	CUserCmd from_cmd;

	from_cmd = *user_cmd;
	to_cmd = from_cmd;

	to_cmd.m_command_number++;
	//to_cmd.m_tickcount++;
	to_cmd.m_tickcount += (static_cast<int>(1.0f / m_globals()->m_intervalpertick) * 3);
	//to_cmd.m_tickcount += 200;

	if (newcmds > choked_modifier)
		return true;

	for (auto i = choked_modifier - newcmds + 1; i > 0; --i)
	{
		WriteUserCmd(buffer, &to_cmd, &from_cmd);

		//from_cmd = to_cmd;
		
		to_cmd.m_command_number++;
		to_cmd.m_tickcount++;
		
		from_cmd.m_command_number = to_cmd.m_command_number - 1;
		from_cmd.m_tickcount = to_cmd.m_tickcount - 1;
		//to_cmd.m_viewangles = cheat.get_command()->m_viewangles;
		//to_cmd.m_buttons |= IN_ATTACK;
	}

	return true;

	/*
	auto newcmds = *new_commands;
	auto shift = cheat.globals.tickbase_shift;

	cheat.globals.tickbase_shift = 0;
	*backup_commands = 0;

	auto choked_modifier = min(newcmds + shift, 62);

	*new_commands = choked_modifier;

	auto next_cmdnr = m_clientstate()->m_iChokedCommands + m_clientstate()->m_nLastOutgoingCommand + 1;
	auto final_to = next_cmdnr - newcmds + 1;

	if (final_to <= next_cmdnr)
	{
		while (original_fn(ecx, slot, buf, final_from, final_to, true))
		{
			final_from = final_to++;

			if (final_to > next_cmdnr)
				goto next_cmd;
		}

		return false;
	}
next_cmd:
	auto user_cmd = m_input()->GetUserCmd(final_from);

	if (!user_cmd)
		return true;

	CUserCmd to_cmd;
	CUserCmd from_cmd;

	from_cmd = *user_cmd;
	to_cmd = from_cmd;

	to_cmd.m_command_number++;

	to_cmd.m_tickcount += (static_cast<int>(1.0f / m_globals()->m_intervalpertick) * 3);
	//to_cmd.m_tickcount += 200;

	if (newcmds > choked_modifier)
		return true;

	for (auto i = (choked_modifier - newcmds + 1); i > 0; --i)
	{
		WriteUserCmd(buf, &to_cmd, &from_cmd);

		from_cmd = to_cmd;
		to_cmd.m_command_number++;
		to_cmd.m_tickcount++;
	}

	return true;
	*/
}
