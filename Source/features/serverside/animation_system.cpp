#include "animation_system.h"
#include "..\misc\logs.h"

std::deque<adjust_data> player_records[65] = { };

void animation_system::init()
{
	//static auto threadedBoneSetup = m_cvar()->FindVar("cl_threaded_bone_setup");
	//threadedBoneSetup->setValue(1);

	static auto extrapolate = m_cvar()->FindVar("cl_extrapolate");
	extrapolate->SetValue(0);
}

void animation_system::apply_interpolation_flags(player_t* e, int flag) {
	const auto var_map = reinterpret_cast<uintptr_t>(e) + 36;

	for (auto index = 0; index < *reinterpret_cast<int*>(var_map + 20); index++)
		*reinterpret_cast<uintptr_t*>(*reinterpret_cast<uintptr_t*>(var_map) + index * 12) = flag;
}

void animation_system::run(ClientFrameStage_t stage)
{
	if (stage != FRAME_NET_UPDATE_END)
		return;

	if (!configs.ragebot.enabled && !configs.legitbot.enabled)
		return;

	if (m_engine()->IsHLTV())
		return;

	if (!cheat.local()->is_alive())
		return;

	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (e == cheat.local())
			continue;

		if (!valid(i, e))
			continue;

		if (!e->is_alive())
			cheat.globals.missed_shots[e->EntIndex()] = 0;

		//apply_interpolation_flags(e, 0);

		auto time_delta = abs(TIME_TO_TICKS(e->m_flSimulationTime()) - m_globals()->m_tickcount);

		if (time_delta > (1.0f / m_globals()->m_intervalpertick))
			continue;

		auto update = player_records[i].empty() || (e->m_flSimulationTime() != e->m_flOldSimulationTime());

		/*
		if (update && !player_records[i].empty())
		{
			auto server_tick = m_clientstate()->m_iClockDriftMgr.m_iServerTick - i % m_globals()->m_timestamprandomizewindow;
			auto current_tick = server_tick - server_tick % m_globals()->m_timestampnetworkingbase;

			if (TIME_TO_TICKS(e->m_flOldSimulationTime()) < current_tick && TIME_TO_TICKS(e->m_flSimulationTime()) == current_tick)
			{
				auto layer = &e->get_animlayers()[ANIMATION_LAYER_ALIVELOOP];
				auto previous_layer = &player_records[i].front().layers[ANIMATION_LAYER_ALIVELOOP];

				if (layer->m_flCycle == previous_layer->m_flCycle)
				{
					e->m_flSimulationTime() = e->m_flOldSimulationTime();
					update = false;
				}
			}
		}
		*/

		if (update)
		{
			if (!player_records[i].empty() && (e->m_vecOrigin() - player_records[i].front().origin).LengthSqr() > 4096.f)
				for (auto& record : player_records[i])
					record.invalid = true;

			player_records[i].emplace_front(adjust_data());
			update_player_animations(e);

			while (player_records[i].size() > 32)
				player_records[i].pop_back();
		}
	}
}

bool animation_system::valid(int i, player_t* e)
{
	if (!e)
		return false;

	if ((!configs.ragebot.enabled && !configs.legitbot.enabled) || !e->valid(false, !configs.ragebot.dormant) || e == cheat.local())
	{
		if (!e->is_alive())
		{
			is_dormant[i] = false;
			player_resolver[i].reset();

			cheat.globals.fired_shots[i] = 0;
			cheat.globals.missed_shots[i] = 0;
		}
		else if (e->IsDormant())
		{
			is_dormant[i] = true;
		}

		player_records[i].clear();
		return false;
	}

	return true;
}

void animation_system::update_player_animations(player_t* e)
{
	auto animstate = e->get_animation_state();

	if (!animstate)
		return;

	auto records = &player_records[e->EntIndex()];

	if (records->empty())
		return;

	adjust_data* previous_record = nullptr;

	if (records->size() > 1 && cheat.local()->is_alive())
		previous_record = &records->at(1);

	auto record = &records->front();

	AnimationLayer animlayers[15];

	memcpy(animlayers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(record->layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));

	//auto backup_lower_body_yaw_target = e->m_flLowerBodyYawTarget();
	auto backup_duck_amount = e->m_flDuckAmount();
	auto backup_flags = e->m_fFlags();
	auto backup_eflags = e->m_iEFlags();

	auto backup_curtime = m_globals()->m_curtime;
	auto backup_frametime = m_globals()->m_frametime;
	auto backup_realtime = m_globals()->m_realtime;
	auto backup_framecount = m_globals()->m_framecount;
	auto backup_tickcount = m_globals()->m_tickcount;
	auto backup_interpolation_amount = m_globals()->m_interpolation_amount;

	m_globals()->m_curtime = e->m_flSimulationTime();
	m_globals()->m_frametime = m_globals()->m_intervalpertick;

	if (previous_record)
	{
		record->shot = record->last_shot_time > previous_record->simulation_time && record->last_shot_time <= record->simulation_time;

		//auto& velocity = e->m_vecVelocity();
		auto was_in_air = e->m_fFlags() & FL_ONGROUND && previous_record->flags & FL_ONGROUND;

		auto time_difference = max(m_globals()->m_intervalpertick, e->m_flSimulationTime() - previous_record->simulation_time);
		auto origin_delta = e->m_vecOrigin() - previous_record->origin;

		auto animation_speed = 0.0f;

		if (!origin_delta.IsZero() && TIME_TO_TICKS(time_difference) > 0)
		{
			e->m_vecVelocity() = origin_delta * (1.0f / TICKS_TO_TIME(time_difference));

			if (e->m_fFlags() & FL_ONGROUND && animlayers[ANIMATION_LAYER_ALIVELOOP].m_flWeight > 0.0f && animlayers[ANIMATION_LAYER_ALIVELOOP].m_flWeight < 1.0f && animlayers[ANIMATION_LAYER_ALIVELOOP].m_flCycle > previous_record->layers[ANIMATION_LAYER_ALIVELOOP].m_flCycle)
			{
				auto weapon = e->m_hActiveWeapon().Get();

				if (weapon)
				{
					auto max_speed = 260.0f;
					auto weapon_info = weapon->get_weapon_info();

					if (weapon_info)
						max_speed = e->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

					auto modifier = 0.35f * (1.0f - animlayers[ANIMATION_LAYER_ALIVELOOP].m_flWeight);

					if (modifier > 0.0f && modifier < 1.0f)
						animation_speed = max_speed * (modifier + 0.55f);
				}
			}

			if (animation_speed > 0.0f)
			{
				animation_speed /= e->m_vecVelocity().Length2D();

				e->m_vecVelocity().x *= animation_speed;
				e->m_vecVelocity().y *= animation_speed;
			}

			if (records->size() >= 3 && time_difference > m_globals()->m_intervalpertick)
			{
				auto previous_velocity = (previous_record->origin - records->at(2).origin) * (1.0f / time_difference);

				if (!previous_velocity.IsZero() && !was_in_air)
				{
					auto current_direction = math::normalize_yaw(RAD2DEG(atan2(e->m_vecVelocity().y, e->m_vecVelocity().x)));
					auto previous_direction = math::normalize_yaw(RAD2DEG(atan2(previous_velocity.y, previous_velocity.x)));

					auto average_direction = current_direction - previous_direction;
					average_direction = DEG2RAD(math::normalize_yaw(current_direction + average_direction * 0.5f));

					auto direction_cos = cos(average_direction);
					auto dirrection_sin = sin(average_direction);

					auto velocity_speed = e->m_vecVelocity().Length2D();

					e->m_vecVelocity().x = direction_cos * velocity_speed;
					e->m_vecVelocity().y = dirrection_sin * velocity_speed;
				}
			}

			if (!(e->m_fFlags() & FL_ONGROUND))
			{
				static auto sv_gravity = m_cvar()->FindVar(crypt_str("sv_gravity"));

				auto fixed_timing = math::clamp(time_difference, m_globals()->m_intervalpertick, 1.0f);
				e->m_vecVelocity().z -= sv_gravity->GetFloat() * 0.5f * fixed_timing;
			}
			else
			{
				e->m_vecVelocity().z = 0.0f;
			}
		}
	}
	else
	{
		record->shot = (record->last_shot_time == record->simulation_time);
	}

	/*
	auto chokedPackets = -1;

	if (previous_record && record->simulation_time != e->m_flSimulationTime())
	{
		const auto simDifference = fabsf(e->m_flSimulationTime() - record->simulation_time);

		chokedPackets = std::clamp(record->simulation_time != e->m_flSimulationTime() ?
			static_cast<int>(simDifference / m_globals()->m_intervalpertick) - 1 : 0, 0, 17);

		if (!record->origin.IsZero())
		{
			previous_record->velocity = record->velocity;
			record->velocity = (e->m_vecOrigin() - record->origin) * (1.0f / simDifference);
		}
	}

	if (chokedPackets > 0)
	{
		for (int i = 1; i <= chokedPackets + 1; i++)
		{
			const float simulatedTime = record->simulation_time + (m_globals()->m_intervalpertick * i);
			const float lerpValue = 1.f - (e->m_flSimulationTime() - simulatedTime) / (e->m_flSimulationTime() - record->simulation_time);
			const float currentTimeBackup = m_globals()->m_curtime;

			m_globals()->m_curtime = simulatedTime;

			e->m_iEFlags() &= ~0x1000;
			e->m_vecAbsVelocity() = math::lerp2(lerpValue, record->velocity, previous_record->velocity);
			e->m_flDuckAmount() = math::lerp2(lerpValue, record->duck_amount, previous_record->duck_amount);

			e->update_clientside_animation();

			m_globals()->m_curtime = currentTimeBackup;
		}

		e->m_iEFlags() &= ~0x1000;
		if (e->m_fFlags() & FL_ONGROUND && e->m_vecVelocity().Length() > 0.0f && animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight <= 0.0f)
			e->m_vecVelocity().Zero();
		e->m_vecAbsVelocity() = record->velocity;
		e->m_flDuckAmount() = record->duck_amount;
	}
	else
	{*/
	e->m_iEFlags() &= ~0x1000;
	if (e->m_fFlags() & FL_ONGROUND && e->m_vecVelocity().Length() > 0.0f && animlayers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight <= 0.0f)
		e->m_vecVelocity().Zero();
	e->m_vecAbsVelocity() = e->m_vecVelocity();
	//e->m_bClientSideAnimation() = true;
	e->update_clientside_animation();
	//}

	/*
	//Fix jump pose
	if (!(e->m_fFlags() & FL_ONGROUND) && !previous_record)// && entity->moveType() != MoveType::NOCLIP)
	{
		const auto currentActivity = animstate->getLayerActivity(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
		if (currentActivity == ACT_CSGO_FALL || currentActivity == ACT_CSGO_JUMP)
		{
			std::array<AnimationLayer, 13> backupLayers;

			std::memcpy(&backupLayers, e->get_animlayers(), sizeof(AnimationLayer) * e->animlayer_count());
			std::memcpy(e->get_animlayers(), &previous_record->layers, sizeof(AnimationLayer)* e->animlayer_count());
			const auto oldActivity = animstate->getLayerActivity(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
			std::memcpy(e->get_animlayers(), &backupLayers, sizeof(AnimationLayer) * e->animlayer_count());

			float cycle;
			if (oldActivity != currentActivity)
			{
				animstate->m_flDurationInAir = 0.0f;
				cycle = 0.0f;
			}
			else if (animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle < previous_record->layers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle)
			{
				animstate->m_flDurationInAir = 0.0f;
				cycle = 0.0f;
			}
			else
			{
				cycle = previous_record->layers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle;
			}

			int ticks = animstate->getTicksFromCycle(animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate,
				animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle, cycle);

			if (ticks == 0 || animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle >= 0.999f)
			{
				//if animation is finished already, fix up the total time in air
				const int totalCommands = chokedPackets + 1;
				const int ticksLeft = totalCommands - ticks;
				ticks += ticksLeft;
			}

			animstate->m_flDurationInAir = animstate->m_flUpdateTimeDelta * ticks;
			animstate->poseParamMappings[PLAYER_POSE_PARAM_JUMP_FALL].setValue(e, std::clamp(math::smooth_step_bounds(0.72f, 1.52f, animstate->m_flDurationInAir), 0.f, 1.f));
		}
	}
	*/

	if (is_dormant[e->EntIndex()])
	{
		is_dormant[e->EntIndex()] = false;

		if (e->m_fFlags() & FL_ONGROUND)
		{
			animstate->m_bOnGround = true;
			animstate->m_bInHitGroundAnimation = false;
		}

		animstate->m_flDurationInAir = 0.0f;
		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);
	}

	auto updated_animations = false;

	c_baseplayeranimationstate state;
	memcpy(&state, animstate, sizeof(c_baseplayeranimationstate));

	auto ticks_chocked = 1;

	if (previous_record)
	{
		memcpy(e->get_animlayers(), previous_record->layers, e->animlayer_count() * sizeof(AnimationLayer));

		auto simulation_ticks = TIME_TO_TICKS(e->m_flSimulationTime() - previous_record->simulation_time);

		if (simulation_ticks > 0 && simulation_ticks < 17) // 31
			ticks_chocked = simulation_ticks;

		if (ticks_chocked > 1)
		{
			auto land_time = 0.0f;
			auto land_in_cycle = false;
			auto is_landed = false;
			auto on_ground = false;

			if (animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle < 0.5f && (!(e->m_fFlags() & FL_ONGROUND) || !(previous_record->flags & FL_ONGROUND)))
			{
				land_time = e->m_flSimulationTime() - animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flPlaybackRate * animlayers[ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL].m_flCycle;
				land_in_cycle = land_time >= previous_record->simulation_time;
			}

			auto duck_amount_per_tick = (e->m_flDuckAmount() - previous_record->duck_amount) / ticks_chocked;

			for (auto i = 0; i < ticks_chocked; i++)
			{
				/*
				auto lby_delta = fabs(math::normalize_yaw(e->m_flLowerBodyYawTarget() - previous_record->lby));

				if (lby_delta > 0.0f && e->m_vecVelocity().Length() < 5.0f)
				{
					auto delta = ticks_chocked - i;
					auto use_new_lby = true;

					if (lby_delta < 1.0f)
						use_new_lby = !delta;
					else
						use_new_lby = delta < 2;

					e->m_flLowerBodyYawTarget() = use_new_lby ? backup_lower_body_yaw_target : previous_record->lby;
				}
				*/ // THIS NOT NEED LMAO

				auto simulated_time = previous_record->simulation_time + TICKS_TO_TIME(i);

				if (duck_amount_per_tick)
					e->m_flDuckAmount() = previous_record->duck_amount + duck_amount_per_tick * static_cast<float>(i);

				on_ground = e->m_fFlags() & FL_ONGROUND;

				if (land_in_cycle && !is_landed)
				{
					if (land_time <= simulated_time)
					{
						is_landed = true;
						on_ground = true;
					}
					else
					{
						on_ground = previous_record->flags & FL_ONGROUND;
					}
				}

				if (on_ground)
					e->m_fFlags() |= FL_ONGROUND;
				else
					e->m_fFlags() &= ~FL_ONGROUND;

				auto simulated_ticks = TIME_TO_TICKS(simulated_time);

				m_globals()->m_realtime = simulated_time;
				m_globals()->m_curtime = simulated_time;
				m_globals()->m_framecount = simulated_ticks;
				m_globals()->m_tickcount = simulated_ticks;
				m_globals()->m_interpolation_amount = 0.0f;

				cheat.globals.updating_animation = true;
				e->update_clientside_animation();
				cheat.globals.updating_animation = false;

				m_globals()->m_realtime = backup_realtime;
				m_globals()->m_curtime = backup_curtime;
				m_globals()->m_framecount = backup_framecount;
				m_globals()->m_tickcount = backup_tickcount;
				m_globals()->m_interpolation_amount = backup_interpolation_amount;

				updated_animations = true;
			}
		}
	}

	if (!updated_animations)
	{
		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;
	}

	memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

	auto setup_matrix = [&](player_t* e, AnimationLayer* layers, const int& matrix) -> void
	{
		//e->invalidate_physics_recursive(PHYSIC_ANGLES | PHYSIC_SEQUENCE | PHYSIC_ANIMATION);
		e->invalidate_physics_recursive(PHYSIC_ANIMATION);

		AnimationLayer backup_layers[15];
		memcpy(backup_layers, e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		memcpy(e->get_animlayers(), layers, e->animlayer_count() * sizeof(AnimationLayer));

		switch (matrix)
		{
		case MAIN:
			e->setup_bones_fixed(record->matrixes_data.main, BONE_USED_BY_ANYTHING);
			break;
		case NONE:
			e->setup_bones_fixed(record->matrixes_data.zero, BONE_USED_BY_HITBOX);
			break;
		case FIRST:
			e->setup_bones_fixed(record->matrixes_data.first, BONE_USED_BY_HITBOX);
			break;
		case SECOND:
			e->setup_bones_fixed(record->matrixes_data.second, BONE_USED_BY_HITBOX);
			break;
		}

		memcpy(e->get_animlayers(), backup_layers, e->animlayer_count() * sizeof(AnimationLayer));
	};

	PlayerInfo player_info;
	m_engine()->GetPlayerInfo(e->EntIndex(), &player_info);

	if (!player_info.fakeplayer && cheat.local()->is_alive() && e->is_alive() && e->m_iTeamNum() != cheat.local()->m_iTeamNum() && !configs.legitbot.enabled)
	{
		float backup_yaw = animstate->m_flGoalFeetYaw;
		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()];

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		previous_goal_feet_yaw[e->EntIndex()] = backup_yaw;
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, NONE);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].zero_goal_feet_yaw = animstate->m_flGoalFeetYaw;

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + e->get_max_desync_delta());

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, FIRST);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].first_goal_feet_yaw = animstate->m_flGoalFeetYaw;

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y - e->get_max_desync_delta());

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, SECOND);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].second_goal_feet_yaw = animstate->m_flGoalFeetYaw;

		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()];

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		player_resolver[e->EntIndex()].initialize(e, record, previous_goal_feet_yaw[e->EntIndex()]);
		player_resolver[e->EntIndex()].resolve_yaw();
		player_resolver[e->EntIndex()].resolve_pitch();
		/*
		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()];

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		previous_goal_feet_yaw[e->EntIndex()] = animstate->m_flGoalFeetYaw;
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y);

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, NONE);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].zero_goal_feet_yaw = animstate->m_flGoalFeetYaw;
		memcpy(player_resolver[e->EntIndex()].resolver_layers[0], e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + e->get_max_desync_delta());

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, FIRST);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].first_goal_feet_yaw = animstate->m_flGoalFeetYaw;
		memcpy(player_resolver[e->EntIndex()].resolver_layers[1], e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		animstate->m_flGoalFeetYaw = math::normalize_yaw(e->m_angEyeAngles().y + e->get_max_desync_delta());

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		setup_matrix(e, animlayers, SECOND);
		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));
		player_resolver[e->EntIndex()].second_goal_feet_yaw = animstate->m_flGoalFeetYaw;
		memcpy(player_resolver[e->EntIndex()].resolver_layers[2], e->get_animlayers(), e->animlayer_count() * sizeof(AnimationLayer));

		animstate->m_flGoalFeetYaw = previous_goal_feet_yaw[e->EntIndex()];

		cheat.globals.updating_animation = true;
		e->update_clientside_animation();
		cheat.globals.updating_animation = false;

		memcpy(animstate, &state, sizeof(c_baseplayeranimationstate));

		player_resolver[e->EntIndex()].initialize(e, record, previous_goal_feet_yaw[e->EntIndex()]);
		player_resolver[e->EntIndex()].resolve_yaw();
		player_resolver[e->EntIndex()].resolve_pitch();
		*/
	}

	cheat.globals.updating_animation = true;
	e->update_clientside_animation();
	cheat.globals.updating_animation = false;

	setup_matrix(e, animlayers, MAIN);
	memcpy(e->m_CachedBoneData().Base(), record->matrixes_data.main, e->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

	m_globals()->m_curtime = backup_curtime;
	m_globals()->m_frametime = backup_frametime;

	//e->m_flLowerBodyYawTarget() = backup_lower_body_yaw_target;
	e->m_flDuckAmount() = backup_duck_amount;
	e->m_fFlags() = backup_flags;
	e->m_iEFlags() = backup_eflags;

	memcpy(e->get_animlayers(), animlayers, e->animlayer_count() * sizeof(AnimationLayer));
	memcpy(player_resolver[e->EntIndex()].previous_layers, animlayers, e->animlayer_count() * sizeof(AnimationLayer));

	record->store_data(e, false);

	if (e->m_flSimulationTime() < e->m_flOldSimulationTime())
		record->invalid = true;
}