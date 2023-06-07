#include "ragebot.h"
#include "fakelag.h"
#include "..\misc\misc.h"
#include "..\serverside\engine_prediction.h"
#include "..\misc\logs.h"

void fakelag::run(CUserCmd* m_pcmd)
{
	if (configs.fakelag.enabled && !cheat.globals.exploits)
	{
		static auto force_choke = false;

		if (force_choke)
		{
			force_choke = false;
			cheat.send_packet = false;
			return;
		}

		if (cheat.local()->m_fFlags() & FL_ONGROUND && !(engineprediction::get().backup_data.flags & FL_ONGROUND))
		{
			force_choke = true;
			cheat.send_packet = false;
			return;
		}
	}

	static auto fluctuate_ticks = 0;
	static auto switch_ticks = false;
	static auto random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);

	auto choked = m_clientstate()->m_iChokedCommands;
	auto flags = engineprediction::get().backup_data.flags;
	auto velocity = engineprediction::get().backup_data.velocity.Length();
	auto velocity2d = engineprediction::get().backup_data.velocity.Length2D();

	auto max_speed = 260.0f;
	auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (weapon_info)
		max_speed = cheat.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

	switch (configs.fakelag.type)
	{
	case 0:
		max_choke = configs.fakelag.triggers_amount;
		break;
	case 1:
		max_choke = random_factor;
		break;
	case 2:
		if (velocity2d >= 5.0f)
		{
			auto dynamic_factor = std::ceilf(64.0f / (velocity2d * m_globals()->m_intervalpertick));

			if (dynamic_factor > 16)
				dynamic_factor = configs.fakelag.triggers_amount;

			max_choke = dynamic_factor;
		}
		else
			max_choke = configs.fakelag.triggers_amount;
		break;
	case 3:
		max_choke = fluctuate_ticks;
		break;
	case 4:
		auto currentSpeed = cheat.local()->m_vecVelocity().Length2D();
		auto extrapolatePerTick = currentSpeed * m_globals()->m_intervalpertick;
		int chokeReq = std::ceilf(64.f / extrapolatePerTick);
		max_choke = max(chokeReq, configs.fakelag.amount);
		break;
	}

	if (m_gamerules()->m_bIsValveDS())
		max_choke = m_engine()->IsVoiceRecording() ? 1 : min(max_choke, cheat.getMaxChokecCMD(true));

	if (misc::get().recharging_double_tap)
		max_choke = cheat.globals.weapon->get_max_tickbase_shift();

	//if (cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND && key_binds::get().get_key_bind_state(key_bind_features::FAKE_DUCK)) // && !m_gamerules()->m_bIsValveDS()&&
	//{
	//	max_choke = 14;
	//	cheat.send_packet = choked >= max_choke;
	//}
	//else
	if (!(cheat.local()->m_fFlags() & FL_ONGROUND) || !(engineprediction::get().backup_data.flags & FL_ONGROUND) || !key_binds::get().get_key_bind_state(key_bind_features::FAKE_DUCK)) // && !m_gamerules()->m_bIsValveDS()&&
	{
		if (configs.ragebot.enabled && cheat.globals.current_weapon != -1 && !cheat.globals.exploits && configs.fakelag.enabled && configs.fakelag.enablers[FAKELAG_PEEK] && configs.fakelag.triggers_amount > 6 && !started_peeking && velocity >= 5.0f)
		{
			auto predicted_eye_pos = cheat.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * static_cast<float>(configs.fakelag.amount) * 0.5f;

			for (auto i = 1; i < m_globals()->m_maxclients; i++)
			{
				auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

				if (!e->valid(true))
					continue;

				auto records = &player_records[i];

				if (records->empty())
					continue;

				auto record = &records->front();

				if (!record->valid())
					continue;

				scan_data predicted_data;
				ragebot::get().scan(record, predicted_data, predicted_eye_pos, true);

				if (predicted_data.valid())
				{
					scan_data data;
					ragebot::get().scan(record, data, cheat.globals.eye_pos, true);

					if (!data.valid())
					{
						random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
						switch_ticks = !switch_ticks;
						fluctuate_ticks = switch_ticks ? configs.fakelag.triggers_amount : max(configs.fakelag.triggers_amount - 2, 1);

						cheat.send_packet = true;
						started_peeking = true;

						return;
					}
				}
			}
		}

		if (!cheat.globals.exploits && configs.fakelag.enabled && configs.fakelag.enablers[FAKELAG_PEEK] && started_peeking)
		{
			if (choked < max_choke)
				cheat.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? configs.fakelag.triggers_amount : max(configs.fakelag.triggers_amount - 2, 1);

				cheat.send_packet = true;
			}
		}
		else if (!cheat.globals.exploits && configs.fakelag.enabled && velocity >= 5.0f && cheat.globals.slowwalking && configs.fakelag.enablers[FAKELAG_SLOW_WALK])
		{
			if (choked < max_choke)
				cheat.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? configs.fakelag.triggers_amount : max(configs.fakelag.triggers_amount - 2, 1);

				cheat.send_packet = true;
			}
		}
		else if (!cheat.globals.exploits && configs.fakelag.enabled && velocity >= 5.0f && !cheat.globals.slowwalking && cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND && configs.fakelag.enablers[FAKELAG_MOVE])
		{
			if (choked < max_choke)
				cheat.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? configs.fakelag.triggers_amount : max(configs.fakelag.triggers_amount - 2, 1);

				cheat.send_packet = true;
			}
		}
		else if (!cheat.globals.exploits && configs.fakelag.enabled && !cheat.globals.slowwalking && !(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND) && configs.fakelag.enablers[FAKELAG_AIR])
		{
			if (choked < max_choke)
				cheat.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? configs.fakelag.triggers_amount : max(configs.fakelag.triggers_amount - 2, 1);

				cheat.send_packet = true;
			}
		}
		else if (!cheat.globals.exploits && configs.fakelag.enabled)
		{
			max_choke = configs.fakelag.amount;

			if (m_gamerules()->m_bIsValveDS())
				max_choke = min(max_choke, cheat.getMaxChokecCMD(true));

			if (choked < max_choke)
				cheat.send_packet = false;
			else
			{
				started_peeking = false;

				random_factor = max(math::random_int(configs.fakelag.min_amount, configs.fakelag.amount), configs.fakelag.min_amount);
				switch_ticks = !switch_ticks;
				fluctuate_ticks = switch_ticks ? configs.fakelag.amount : max(configs.fakelag.amount - 2, 1);

				cheat.send_packet = true;
			}
		}
		else if (cheat.globals.exploits || !antiaim::get().condition(m_pcmd, false) && (antiaim::get().type == ANTIAIM_LEGIT || configs.antiaim.type[antiaim::get().type].desync))
		{
			condition = true;
			started_peeking = false;
			cheat.send_packet = (choked >= 1);
		}
		else
			condition = true;
	}
}

void fakelag::create_move(CUserCmd* m_pcmd)
{
	if (fakelag_condition(m_pcmd))
		return;

	run(m_pcmd);

	if (!m_gamerules()->m_bIsValveDS() && m_clientstate()->m_iChokedCommands < 17)
	{
		static auto Fn = util::FindSignature(crypt_str("engine.dll"), crypt_str("B8 ? ? ? ? 3B F0 0F 4F F0 89 5D FC")) + 1;
		DWORD old = 0;
		VirtualProtect((void*)Fn, sizeof(uint32_t), PAGE_EXECUTE_READWRITE, &old);
		*reinterpret_cast<uint32_t*>(Fn) = 17;
		VirtualProtect((void*)Fn, sizeof(uint32_t), old, &old);
	}
}

bool fakelag::fakelag_condition(CUserCmd* m_pcmd)
{
	condition = false;

	if (cheat.local()->m_bGunGameImmunity() || cheat.local()->m_fFlags() & FL_FROZEN)
		condition = true;

	if ((antiaim::get().freeze_check || antiaim::get().cs_match_win) && !misc::get().double_tap_enabled && !misc::get().hide_shots_enabled)
		condition = true;

	return condition;
}