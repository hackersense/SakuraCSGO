#include "knifebot.h"

#include <random>

#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\ragebot\autowall.h"
#include "..\movement\slowwalk.h"
#include "..\serverside\local_animations.h"

void knifebot::run(CUserCmd* cmd)
{
	final_target.reset();

	if (!configs.ragebot.enabled)
		return;

	if (!configs.ragebot.knife_bot)
		return;

	if (!cheat.globals.weapon->is_knife())
		return;

	scan_targets();

	if (!final_target.record)
		return;

	fire(cmd);
}

void knifebot::scan_targets()
{
	if (ragebot::get().targets.empty())
		return;

	for (auto& target : ragebot::get().targets)
	{
		if (target.history_record->valid())
		{
			if (target.last_record->valid())
			{
				auto last_distance = cheat.globals.eye_pos.DistTo(target.last_record->origin);
				auto history_distance = cheat.globals.eye_pos.DistTo(target.history_record->origin);

				final_target.record = last_distance > history_distance ? target.history_record : target.last_record;
			}
			else
			{
				final_target.record = target.history_record;
			}
		}
		else
		{
			final_target.record = target.last_record;
		}
		final_target.record->adjust_player();
	}
}

void knifebot::fire(CUserCmd* cmd)
{
	if (!cheat.globals.weapon->can_fire(false))
		return;

	auto& vecOrigin = final_target.record->player->m_vecOrigin();

	auto bestDistance = cheat.local()->m_vecOrigin().DistTo(vecOrigin);

	if (bestDistance > 75.0f)
		return;

	const auto angle{ math::calculate_relative_angle(cheat.globals.eye_pos, final_target.record->player->bone_position(7), cmd->m_viewangles + cheat.local()->m_aimPunchAngle()) };
	const bool firstSwing = (cheat.globals.weapon->m_flNextPrimaryAttack() + 0.4) < TICKS_TO_TIME(cheat.globals.fixed_tickbase);

	bool backStab = false;

	Vector targetForward;
	math::angle_vectors(final_target.record->player->GetAbsAngles(), targetForward);
	targetForward.z = 0;

	Vector vecLOS = (vecOrigin - cheat.local()->m_vecOrigin());
	vecLOS.z = 0;
	vecLOS.Normalize();

	float dot = vecLOS.Dot(targetForward);

	if (dot > 0.475f)
		backStab = true;

	auto hp = final_target.record->player->m_iHealth();
	auto armor = final_target.record->player->m_ArmorValue() > 1;

	int minDmgSol = 40;
	int minDmgSag = 65;

	if (backStab)
	{
		minDmgSag = 180;
		minDmgSol = 90;
	}
	else if (!firstSwing)
		minDmgSol = 25;

	if (armor)
	{
		minDmgSag = static_cast<int>((float)minDmgSag * 0.85f);
		minDmgSol = static_cast<int>((float)minDmgSol * 0.85f);
	}

	if (hp <= minDmgSag && bestDistance <= 60)
		cmd->m_buttons |= IN_ATTACK2;
	else if (hp <= minDmgSol)
		cmd->m_buttons |= IN_ATTACK;
	else
		cmd->m_buttons |= IN_ATTACK;

	cmd->m_viewangles += angle;
	cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());
	//if (!determinate_hit_type(1, vecDelta))
	//	return;
}

int knifebot::determinate_hit_type(bool stab_type, const Vector& delta)
{
	auto minimum_distance = stab_type ? 32.0f : 48.0f;
	auto end = cheat.globals.eye_pos + delta * minimum_distance;

	CTraceFilter filter;
	filter.pSkip = cheat.local();

	CGameTrace trace;
	Ray_t ray;

	ray.Init(cheat.globals.eye_pos, end, Vector(-16.0f, -16.0f, -18.0f), Vector(16.0f, 16.0f, 18.0f));
	m_trace()->TraceRay(ray, 0x200400B, &filter, &trace);

	if (trace.entity != final_target.record->player)
		return 0;

	auto cos_pitch = cos(DEG2RAD(final_target.record->angles.x));

	auto sin_yaw = 0.0f;
	auto cos_yaw = 0.0f;

	DirectX::XMScalarSinCos(&sin_yaw, &cos_yaw, DEG2RAD(final_target.record->angles.y));

	auto final_delta = final_target.record->origin - cheat.globals.eye_pos;
	return static_cast<int>(cos_yaw * cos_pitch * final_delta.x + sin_yaw * cos_pitch * final_delta.y >= 0.475f) + 1;
}
