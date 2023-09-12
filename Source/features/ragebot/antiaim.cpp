#include "antiaim.h"
#include "..\serverside\engine_prediction.h"
#include "..\serverside\local_animations.h"

void antiaim::create_move(CUserCmd* m_pcmd)
{
	auto velocity = localPlayer->m_vecVelocity().Length();

	type = ANTIAIM_STAND;

	if (configs.antiaim.antiaim_type)
		type = ANTIAIM_LEGIT;
	//else if (velocity >= 5.0f && cheat.globals.slowwalking)
	//	type = ANTIAIM_SLOW_WALK;
	else if (velocity >= 5.0f && localPlayer->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)
		type = ANTIAIM_MOVE;
	else if (!(localPlayer->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		type = ANTIAIM_AIR;

	if (!configs.ragebot.enabled && type != ANTIAIM_LEGIT)
		return;

	if (condition(m_pcmd) || localPlayer->m_fFlags() & FL_ATCONTROLS)
		return;

	/*
	if ((type == ANTIAIM_LEGIT ? configs.antiaim.desync : configs.antiaim.type[type].desync) && (type == ANTIAIM_LEGIT ? !configs.antiaim.legit_lby_type : !configs.antiaim.lby_type) && !configs.misc.fast_stop && (!cheat.globals.weapon->is_grenade() || (!(m_pcmd->m_buttons & IN_ATTACK) && !(m_pcmd->m_buttons & IN_ATTACK2))) && engineprediction::get().backup_data.velocity.Length2D() <= 20.0f)
	{
		auto speed = 1.01f;

		if ((m_pcmd->m_buttons & IN_DUCK) || cheat.globals.fakeducking)
			speed *= 2.94117647f;

		static auto switch_move = false;

		if (switch_move)
			m_pcmd->m_sidemove += speed;
		else
			m_pcmd->m_sidemove -= speed;

		switch_move = !switch_move;
	}
	*/

	if (type != ANTIAIM_LEGIT)
		m_pcmd->m_viewangles.x = get_pitch(m_pcmd);

	m_pcmd->m_viewangles.y = get_yaw(m_pcmd);

	if (type != ANTIAIM_LEGIT && (localPlayer->m_vecVelocity().Length2D() < 1.3f))
		m_pcmd->m_viewangles.z = max(min(configs.antiaim.type[type].roll, 50), -50);
	else if (type == ANTIAIM_LEGIT)
		m_pcmd->m_viewangles.z = 0.0f;

	if (type != ANTIAIM_LEGIT)
	{
		if (configs.antiaim.type[type].fake_flick)
		{
			if ((m_globals()->m_tickcount % 17) == 15)
				if (configs.antiaim.fake_flick_180.key > KEY_NONE && configs.antiaim.fake_flick_180.key < KEY_MAX && key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_FAKE_FLICK_180))
					m_pcmd->m_viewangles.y += ((configs.antiaim.flip_desync.key > KEY_NONE && configs.antiaim.flip_desync.key < KEY_MAX && key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_DESYNC_FLIP)) ? 180.0f : -180.0f);
				else if (final_manual_side == SIDE_NONE || final_manual_side == SIDE_BACK || !configs.antiaim.type[type].fake_flick_disable_with_manual)
					m_pcmd->m_viewangles.y += ((configs.antiaim.flip_desync.key > KEY_NONE && configs.antiaim.flip_desync.key < KEY_MAX && key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_DESYNC_FLIP)) ? 90.0f : -90.0f);
		}

		jitter_move(m_pcmd);
	}
}

bool antiaim::anti_brute(bool flip)
{
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
		if (!e->valid(true))
			continue;

		auto weapon = e->getActiveWeapon();
		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);
		auto fov = math::get_fov(angles, math::calculate_angle(cheat.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}

		if (!target)
			return flip;

		// method 1 variable
		auto simtime = target->m_flSimulationTime();
		auto oldsimtime = target->m_flOldSimulationTime();
		float m_flLastShotTime;
		bool m_shot;
		m_flLastShotTime = weapon ? weapon->m_fLastShotTime() : 0.f;
		m_shot = m_flLastShotTime > oldsimtime && m_flLastShotTime <= simtime;
		bool match = false;

		// method 2 variables
		float shoot_time = -1.f;

		if (TIME_TO_TICKS(shoot_time) == TIME_TO_TICKS(simtime))
			match = true;

		if (match || m_flLastShotTime <= simtime && m_shot)
			return !flip;
	}

	return flip;
}

void antiaim::jitter_move(CUserCmd* m_pcmd)
{
	auto forwardmove = m_pcmd->m_forwardmove;
	auto sidemove = m_pcmd->m_sidemove;
	auto upmove = m_pcmd->m_upmove;
	auto tickrate = static_cast<int>(1.0f / m_globals()->m_intervalpertick);
	const auto speed_per_tick = 5.0f / tickrate;

	if (!configs.antiaim.jitter_move || (localPlayer->m_fFlags() & FL_FROZEN) || localPlayer->m_bGunGameImmunity())
		return;

	const float move_length = Vector(forwardmove, sidemove, upmove).Length();

	const float target_velocity = ((m_pcmd->m_command_number % tickrate) * speed_per_tick) + 95.0f;
	if (target_velocity <= 100.0)
		if (target_velocity >= 95.0)
			accelerationSpeed = target_velocity;
		else
			accelerationSpeed = 100.0;

	const float origin_velocity = (accelerationSpeed / 100.0) * localPlayer->get_max_player_speed();
	if (origin_velocity <= 0.0f || move_length < 10.0 || move_length < origin_velocity)
		return;

	m_pcmd->m_forwardmove = (forwardmove / move_length) * origin_velocity;
	m_pcmd->m_sidemove = (sidemove / move_length) * origin_velocity;
	m_pcmd->m_upmove = (upmove / move_length) * origin_velocity;
}

float antiaim::get_pitch(CUserCmd* m_pcmd)
{
	/*
	static auto invert_jitter = false;
	static auto should_invert = false;

	if (cheat.send_packet)
	{
		should_invert = true;
	}
	else if (!cheat.send_packet && should_invert)
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}
	*/
	static auto invert_jitter = false;
	if (cheat.send_packet)
		invert_jitter = !invert_jitter;

	auto pitch = m_pcmd->m_viewangles.x;

	auto fakeDown = (configs.misc.anti_untrusted || configs.antiaim.type[type].roll != 0.0f) ? 89.0f : 540.0f;
	auto fakeUp = (configs.misc.anti_untrusted || configs.antiaim.type[type].roll != 0.0f) ? -89.0f : -540.0f;

	switch (configs.antiaim.type[type].pitch)
	{
	case 1:
		pitch = 89.0f;
		break;
	case 2:
		pitch = -89.0f;
		break;
	case 3:
		pitch = 0.0f;
		break;
	case 4:
		pitch = math::random_float(-89.0f, 89.0f);
		break;
	case 5:
		pitch = invert_jitter ? 89.0f : -89.0f;
		break;
	case 6:
		pitch = fakeDown;
		break;
	case 7:
		pitch = fakeUp;
		break;
	case 8:
		pitch = invert_jitter ? fakeDown : fakeUp;
		break;
	case 9:
		pitch = configs.antiaim.type[type].custom_pitch;
		break;
	}

	if (key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_ZERO_PITCH))
		pitch = 0.0f;

	return pitch;
}

float antiaim::get_yaw(CUserCmd* m_pcmd)
{
	/*
	static auto invert_jitter = false;
	static auto should_invert = false;

	if (cheat.send_packet)
	{
		should_invert = true;
	}
	else if (!cheat.send_packet && should_invert)
	{
		should_invert = false;
		invert_jitter = !invert_jitter;
	}
	*/
	static auto invert_jitter = false;

	if (cheat.send_packet)
		invert_jitter = !invert_jitter;

	auto max_desync_delta = localPlayer->get_max_desync_delta();

	auto yaw = 0.0f;
	auto lby_type = 0;

	if (type == ANTIAIM_LEGIT)
	{
		yaw = m_pcmd->m_viewangles.y;

		if (!configs.antiaim.desync)
			return yaw;

		if (configs.antiaim.desync != 2 && (configs.antiaim.flip_desync.key <= KEY_NONE || configs.antiaim.flip_desync.key >= KEY_MAX))
			flip = automatic_direction();
		else if (configs.antiaim.desync == 1)
			flip = key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_DESYNC_FLIP);
		else if (configs.antiaim.desync == 2)
			flip = invert_jitter;

		desync_angle = max_desync_delta;

		if (configs.antiaim.legit_lby_type && localPlayer->m_vecVelocity().Length() < 5.0f && localPlayer->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND)
			desync_angle *= 2.f;

		if (flip)
		{
			desync_angle = -desync_angle;
			max_desync_delta = -max_desync_delta;
		}

		yaw -= desync_angle;
		lby_type = configs.antiaim.legit_lby_type;
	}
	else
	{
		if (manual_side == SIDE_NONE)
		{
			if (configs.antiaim.auto_direction)
				freestanding(m_pcmd);
			else
				final_manual_side = SIDE_BACK;
		}
		else
			final_manual_side = manual_side;

		auto base_angle = m_pcmd->m_viewangles.y + 180.0f;

		if (final_manual_side == SIDE_LEFT)
			base_angle -= 90.0f;
		if (final_manual_side == SIDE_RIGHT)
			base_angle += 90.0f;

		if (configs.antiaim.type[type].base_angle && manual_side == SIDE_NONE)
			base_angle = at_targets();

		if (configs.antiaim.type[type].desync != 2 && (configs.antiaim.flip_desync.key <= KEY_NONE || configs.antiaim.flip_desync.key >= KEY_MAX))
		{
			if (final_manual_side == SIDE_LEFT)
				flip = true;
			else if (final_manual_side == SIDE_RIGHT)
				flip = false;
			else
				flip = automatic_direction();
		}
		else if (configs.antiaim.type[type].desync == 1)
			flip = key_binds::get().get_key_bind_state(key_bind_features::ANTIAIM_DESYNC_FLIP);

		auto yaw_angle = 0.0f;

		const float flick_size = 1.0f;

		switch (configs.antiaim.type[type].yaw)
		{
		case 1:
		{
			auto need_flip = invert_jitter;
			if (configs.antiaim.type[type].yaw_jitter_type == 1)
			{
				auto jitter_tick = math::random_int(configs.antiaim.type[type].yaw_jitter_ticks_min, configs.antiaim.type[type].yaw_jitter_ticks_max);
				need_flip = (m_globals()->m_tickcount % jitter_tick) == (jitter_tick - 1);
			}
			yaw_angle = need_flip ? (float)configs.antiaim.type[type].range * -flick_size : (float)configs.antiaim.type[type].range * flick_size;
			break;
		}
		case 2:
			if (flip)
			{
				auto start_angle = (float)configs.antiaim.type[type].range * flick_size;
				auto end_angle = (float)configs.antiaim.type[type].range * -flick_size;

				static auto angle = start_angle;

				auto angle_add_amount = (float)configs.antiaim.type[type].speed * flick_size;

				if (angle - angle_add_amount >= end_angle)
					angle -= angle_add_amount;
				else
					angle = start_angle;

				yaw_angle = angle;
			}
			else
			{
				auto start_angle = (float)configs.antiaim.type[type].range * -flick_size;
				auto end_angle = (float)configs.antiaim.type[type].range * flick_size;

				static auto angle = start_angle;

				auto angle_add_amount = (float)configs.antiaim.type[type].speed * flick_size;

				if (angle + angle_add_amount <= end_angle)
					angle += angle_add_amount;
				else
					angle = start_angle;

				yaw_angle = angle;
			}
			break;
		case 3:
			yaw_angle = configs.antiaim.type[type].custom_yaw;
			break;
		}

		desync_angle = 0.0f;

		if (configs.antiaim.type[type].desync)
		{
			if (configs.antiaim.type[type].desync == 2)
			{
				switch (configs.antiaim.type[type].jitter_type)
				{
				case 0:
					flip = invert_jitter;
					break;
				case 1:
				{
					auto jitter_tick = math::random_int(configs.antiaim.type[type].jitter_ticks_min, configs.antiaim.type[type].jitter_ticks_max);
					flip = (m_globals()->m_tickcount % jitter_tick) == (jitter_tick - 1);
					break;
				}
				//case 2:
				//	flip = anti_brute(flip);
				//	break;
				}
			}

			auto desync_delta = max_desync_delta;

			if (type == ANTIAIM_STAND && configs.antiaim.lby_type)
			{
				desync_delta *= 2.0f;
			}
			else
			{
				if (configs.antiaim.type[type].desync_mode)
				{
					desync_delta = min(desync_delta, math::random_int(configs.antiaim.type[type].inverted_desync_range, configs.antiaim.type[type].desync_range));
				}
				else
				{
					if (!flip)
						desync_delta = min(desync_delta, (float)configs.antiaim.type[type].desync_range);
					else
						desync_delta = min(desync_delta, (float)configs.antiaim.type[type].inverted_desync_range);
				}
			}

			if (!flip)
			{
				desync_delta = -desync_delta;
				max_desync_delta = -max_desync_delta;
			}

			base_angle -= desync_delta;
			/*
			if (type != ANTIAIM_STAND && type != ANTIAIM_LEGIT || !configs.antiaim.lby_type)
			{
				if (configs.antiaim.type[type].body_lean_mode)
				{
					base_angle += desync_delta * math::random_int(configs.antiaim.type[type].inverted_body_lean, configs.antiaim.type[type].body_lean) * 0.01f;
				}
				else
				{
					if (!flip)
						base_angle += desync_delta * (float)configs.antiaim.type[type].body_lean * 0.01f;
					else
						base_angle += desync_delta * (float)configs.antiaim.type[type].inverted_body_lean * 0.01f;
				}
			}
			*/
			desync_angle = desync_delta;
		}

		yaw = base_angle + yaw_angle;

		if (!desync_angle)
			return yaw;

		lby_type = configs.antiaim.lby_type;
	}

	static auto sway_counter = 0;
	static auto force_choke = false;

	if (lby_type && should_break_lby(m_pcmd))
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

		if (lby_type != 2 || sway_counter > 3)
		{
			if (desync_angle > 0.0f)
				yaw -= 179.0f;
			else
				yaw += 179.0f;
		}

		if (sway_counter < 8)
			++sway_counter;
		else
			sway_counter = 0;

		breaking_lby = true;
		force_choke = true;
		cheat.send_packet = false;

		return yaw;
	}
	else if (force_choke)
	{
		force_choke = false;
		cheat.send_packet = false;

		return yaw;
	}
	else if (cheat.send_packet)
	{
		yaw += desync_angle;
	}

	return yaw;
}

bool antiaim::condition(CUserCmd* m_pcmd, bool dynamic_check)
{
	if (!m_pcmd)
		return true;

	if (!cheat.available())
		return true;

	if (!configs.antiaim.enabled)
		return true;

	if (!localPlayer->is_alive())
		return true;

	if (localPlayer->m_bGunGameImmunity() || localPlayer->m_fFlags() & FL_FROZEN)
		return true;

	if (localPlayer->get_move_type() == MOVETYPE_NOCLIP || localPlayer->get_move_type() == MOVETYPE_LADDER)
		return true;

	if (cheat.globals.aimbot_working)
		return true;

	auto weapon = cheat.globals.weapon;

	if (!weapon)
		return true;

	if (m_pcmd->m_buttons & IN_ATTACK && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_non_aim())
		return true;

	auto revolver_shoot = weapon->m_iItemDefinitionIndex() == WEAPON_REVOLVER && !cheat.globals.revolver_working && (m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2);

	if (revolver_shoot)
		return true;

	if ((m_pcmd->m_buttons & IN_ATTACK || m_pcmd->m_buttons & IN_ATTACK2) && weapon->is_knife())
		return true;

	if (dynamic_check && ((cs_match_win || freeze_check) || m_pcmd->m_buttons & IN_USE || (weapon->is_grenade() && weapon->m_fThrowTime())))
		return true;

	return false;
}

bool antiaim::should_break_lby(CUserCmd* m_pcmd)
{
	if (cheat.globals.fakeducking && m_clientstate()->m_iChokedCommands > 12)
		return false;

	//if (!cheat.globals.fakeducking && m_clientstate()->m_iChokedCommands > 14)
	//{
	//	cheat.send_packet = true;
	//	fakelag::get().started_peeking = false;
	//}

	//if (localPlayer->m_vecVelocity().Length2D() > 0.1f || fabsf(localPlayer->m_vecVelocity().z) > 100.f)
	auto animstate = localPlayer->get_animation_state();

	if (!animstate)
		return false;
	/*
	if (animstate->m_flVelocityLengthXY > 0.1f || fabs(animstate->m_flVelocityLengthZ) > 100.0f)
		cheat.globals.next_lby_update = util::server_time() + 0.22f;
	else if (util::server_time() >= cheat.globals.next_lby_update)
	{
		cheat.globals.next_lby_update = util::server_time() + 1.1f;
		return true;
	}
	*/
	if (animstate->m_flVelocityLengthXY > 0.1f || fabs(animstate->m_flVelocityLengthZ) > 100.0f)
		cheat.globals.next_lby_update = TICKS_TO_TIME(cheat.globals.fixed_tickbase + 14);
	else if (TICKS_TO_TIME(cheat.globals.fixed_tickbase) > cheat.globals.next_lby_update)
	{
		cheat.globals.next_lby_update = 0.0f;
		return true;
	}

	return false;
}

float antiaim::at_targets()
{
	player_t* target = nullptr;
	auto best_fov = FLT_MAX;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true))
			continue;

		auto weapon = e->getActiveWeapon();

		if (!weapon)
			continue;

		if (weapon->is_non_aim())
			continue;

		Vector angles;
		m_engine()->GetViewAngles(angles);

		auto fov = math::get_fov(angles, math::calculate_angle(cheat.globals.eye_pos, e->GetAbsOrigin()));

		if (fov < best_fov)
		{
			best_fov = fov;
			target = e;
		}
	}

	auto angle = 180.0f;

	if (manual_side == SIDE_LEFT)
		angle = 90.0f;
	else if (manual_side == SIDE_RIGHT)
		angle = -90.0f;

	if (!target)
		return cheat.get_command()->m_viewangles.y + angle;

	return math::calculate_angle(cheat.globals.eye_pos, target->GetAbsOrigin()).y + angle;
}

bool antiaim::automatic_direction()
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	CGameTrace tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = localPlayer.get();
	src3D = cheat.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	cheat.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	cheat.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	cheat.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	cheat.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;

	if (Left - Right > 10.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Right - Left > 10.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (right_ticks > 10)
		return true;
	else if (left_ticks > 10)
		return false;

	return flip;
}

void antiaim::freestanding(CUserCmd* m_pcmd)
{
	float Right, Left;
	Vector src3D, dst3D, forward, right, up;
	CGameTrace tr;
	Ray_t ray_right, ray_left;
	CTraceFilter filter;

	Vector engineViewAngles;
	m_engine()->GetViewAngles(engineViewAngles);
	engineViewAngles.x = 0.0f;

	math::angle_vectors(engineViewAngles, &forward, &right, &up);

	filter.pSkip = localPlayer.get();
	src3D = cheat.globals.eye_pos;
	dst3D = src3D + forward * 100.0f;

	ray_right.Init(src3D + right * 35.0f, dst3D + right * 35.0f);

	cheat.globals.autowalling = true;
	m_trace()->TraceRay(ray_right, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	cheat.globals.autowalling = false;

	Right = (tr.endpos - tr.startpos).Length();

	ray_left.Init(src3D - right * 35.0f, dst3D - right * 35.0f);

	cheat.globals.autowalling = true;
	m_trace()->TraceRay(ray_left, MASK_SOLID & ~CONTENTS_MONSTER, &filter, &tr);
	cheat.globals.autowalling = false;

	Left = (tr.endpos - tr.startpos).Length();

	static auto left_ticks = 0;
	static auto right_ticks = 0;
	static auto back_ticks = 0;

	if (Right - Left > 20.0f)
		left_ticks++;
	else
		left_ticks = 0;

	if (Left - Right > 20.0f)
		right_ticks++;
	else
		right_ticks = 0;

	if (fabs(Right - Left) <= 20.0f)
		back_ticks++;
	else
		back_ticks = 0;

	if (right_ticks > 10)
		final_manual_side = SIDE_RIGHT;
	else if (left_ticks > 10)
		final_manual_side = SIDE_LEFT;
	else if (back_ticks > 10)
		final_manual_side = SIDE_BACK;
}