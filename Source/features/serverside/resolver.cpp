#include "animation_system.h"
#include "..\ragebot\ragebot.h"

void resolver::initialize(player_t* e, adjust_data* record, float goal_feet_yaw)
{
	player = e;
	player_record = record;
	original_goal_feet_yaw = goal_feet_yaw;
}

void resolver::reset()
{
	player = nullptr;
	player_record = nullptr;

	was_first_bruteforce = false;
	was_second_bruteforce = false;
	was_first_low_bruteforce = false;
	was_second_low_bruteforce = false;
	side = false;
	original_goal_feet_yaw = 0.0f;
}

bool resolver::is_slow_walking(player_t* entity)
{
	float velocity_2D[64]{};
	float old_velocity_2D[64]{};

	if (entity->m_vecVelocity().Length2D() != velocity_2D[entity->EntIndex()] && entity->m_vecVelocity().Length2D() != 0) {
		old_velocity_2D[entity->EntIndex()] = velocity_2D[entity->EntIndex()];
		velocity_2D[entity->EntIndex()] = entity->m_vecVelocity().Length2D();
	}

	if (velocity_2D[entity->EntIndex()] > 0.1) {
		int tick_counter[64]{};

		if (velocity_2D[entity->EntIndex()] == old_velocity_2D[entity->EntIndex()])
			++tick_counter[entity->EntIndex()];
		else
			tick_counter[entity->EntIndex()] = 0;

		while (tick_counter[entity->EntIndex()] > (1 / m_globals()->m_intervalpertick) * 0.1f) // should give use 100ms in ticks if their speed stays the same for that long they are definetely up to something..
			return true;

	}
	return false;
}

bool resolver::isRoll(player_t* entity)
{
	float angle_diff = math::normalize_yaw(0);
	Vector first = ZERO, second = ZERO, third = ZERO;
	first = Vector(entity->hitbox_position(HITBOX_HEAD).x, entity->hitbox_position(HITBOX_HEAD).y + min(angle_diff, 5), entity->hitbox_position(HITBOX_HEAD).z);
	second = Vector(entity->hitbox_position(HITBOX_HEAD).x, entity->hitbox_position(HITBOX_HEAD).y, entity->hitbox_position(HITBOX_HEAD).z);
	third = Vector(entity->hitbox_position(HITBOX_HEAD).x, entity->hitbox_position(HITBOX_HEAD).y - min(angle_diff, 5), entity->hitbox_position(HITBOX_HEAD).z);
	Ray_t one, two, three;
	CGameTrace tone, ttwo, ttree;
	CTraceFilter fl;
	fl.pSkip = entity;
	one.Init(localPlayer->get_eye_position(), first);
	two.Init(localPlayer->get_eye_position(), second);
	three.Init(localPlayer->get_eye_position(), third);
	m_trace()->TraceRay(one, MASK_PLAYERSOLID, &fl, &tone);
	m_trace()->TraceRay(two, MASK_PLAYERSOLID, &fl, &ttwo);
	m_trace()->TraceRay(three, MASK_PLAYERSOLID, &fl, &ttree);

	float lby = fabs(math::normalize_yaw(entity->m_flLowerBodyYawTarget()));
	if (lby < 5 && lby > -5)
		return true;
	return false;
}

float ClampYaw(float a)
{
	auto ret = 0;

	if (a > 180)
		ret = a - 360;

	if (180 > a)
		ret = a + 360;

	return ret;
}

/*
void resolver::test(player_t* e)
{
	// lets be real this is the most p thing we ever seen
	AnimationLayer layers[15];
	AnimationLayer moveLayers[3][15];
	int m_flSide = 0;
	bool m_bAnimsUpdated;
	bool m_bResolve;
	bool m_flPreviousDelta;

	if (player->is_alive() && !player->is_player())
	{
		memcpy(layers, player->get_animlayers(), sizeof(AnimationLayer) * 15);
		memcpy(moveLayers, player->get_animlayers(), sizeof(AnimationLayer) * 15);

		auto standing = layers[ANIMATION_LAYER_ADJUST].m_flWeight == 0.0f && layers[ANIMATION_LAYER_ADJUST].m_flCycle == 0.0f;
		auto animating = layers[ANIMATION_LAYER_LEAN].m_flWeight * 1000.f;
		auto moving = !animating && (layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f) == (layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight * 1000.f);

		player->get_animation_state()->m_flFeetCycle = e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flCycle;
		player->get_animation_state()->m_flFeetYawRate = e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flWeight;
		player->get_animation_state()->m_flWeight() = e->get_animlayer(ANIMATION_LAYER_LEAN)->m_flWeight;

		float m_flSpeed = player->m_vecVelocity().Length2D();
		if (m_flSpeed > 1.1f)
		{
			if (moving)
			{
				float EyeYaw = fabs(layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - moveLayers[ANIMATION_LAYER_AIMMATRIX][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);
				float Negative = fabs(layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - moveLayers[ANIMATION_LAYER_WEAPON_ACTION_RECROUCH][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);
				float Positive = fabs(layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate - moveLayers[ANIMATION_LAYER_WEAPON_ACTION][ANIMATION_LAYER_MOVEMENT_MOVE].m_flPlaybackRate);
				if (Positive > EyeYaw || Positive >= Negative || (Positive * 1000.0))
				{
					if (EyeYaw >= Negative && Positive > Negative && !(Negative * 1000.0))
					{
						m_bAnimsUpdated = true;
						m_bResolve = true;
						m_flSide = 1;
					}
				}
				else
				{
					m_bAnimsUpdated = true;
					m_bResolve = true;
					m_flSide = -1;
				}
			}
		}
		else if (standing)
		{
			auto m_flEyeDelta = std::remainderf(math::normalize_yaw(player->get_animation_state()->m_flEyeYaw - player->m_flLowerBodyYawTarget()), 360.f) <= 0.f;
			if (2 * m_flEyeDelta)
			{
				if (2 * m_flEyeDelta == 2)
				{
					m_flSide = -1;
				}
			}
			else
			{
				m_flSide = 1;
			}
			m_bResolve = true;
			m_flPreviousDelta = m_flEyeDelta;
		}
	}
	player->get_animation_state()->m_flGoalFeetYaw = math::normalize_yaw(player->get_animation_state()->m_flEyeYaw + (player->get_max_desync_delta_for_auto() * m_flSide));
}
*/

void resolver::test(player_t* e)
{
	auto state = e->get_animation_state();
	float velocity = std::floor(std::sqrt(std::pow(state->m_vecVelocity.x, 2) + std::pow(state->m_vecVelocity.y, 2)) + 0.5f);
	bool act_move = velocity >= 3 && (e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flWeight * 1000);
	bool act_stand = velocity < 3 && (e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flWeight < 1);

	state->m_flFeetCycle = e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flCycle;
	state->m_flFeetYawRate = e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE)->m_flWeight;
	state->m_flAccelerationWeight = e->get_animlayer(ANIMATION_LAYER_LEAN)->m_flWeight;
	float delta = 0.0f;
	int m_side = 0;

	if (act_stand)
	{
		auto layer = e->get_animlayer(ANIMATION_LAYER_ADJUST);
		auto layer_move = e->get_animlayer(ANIMATION_LAYER_MOVEMENT_MOVE);

		if (layer->m_flWeight < 1 && layer->m_flCycle < 1 && layer_move->m_flWeight < 1)
		{
			auto m_flEyeDelta = math::angle_difference(state->m_flEyeYaw, state->m_flGoalFeetYaw) <= 0.f;
			//auto m_flEyeDelta = std::remainderf(math::normalize_yaw(state->m_flEyeYaw - player->m_flLowerBodyYawTarget()), 360.f) <= 0.f;
			m_side = (2 * m_flEyeDelta == 2) ? -1 : 1;
		}
	}
	else if (act_move)
	{
		if (ClampYaw(state->m_flEyeYaw - state->m_flGoalFeetYaw) > 15)
			m_side = 1;
		else if (ClampYaw(state->m_flEyeYaw - state->m_flGoalFeetYaw) < -15)
			m_side = -1;
	}

	if (m_side == 1)
		delta = e->get_max_desync_delta();
	else if (m_side == -1)
		delta = e->get_min_desync_delta();
	else
		delta = state->m_flEyeYaw;

	e->get_animation_state()->m_flGoalFeetYaw = math::normalize_yaw(state->m_flGoalFeetYaw + delta);
}

void resolver::resolve_yaw()
{
	if (!configs.ragebot.resolver[RESOLVER_YAW])
		return;

	if (configs.ragebot.resolver_mode == 1 && math::random_int(0, 2))
		return;

	PlayerInfo player_info;
	m_engine()->GetPlayerInfo(player->EntIndex(), &player_info);
	auto animstate = player->get_animation_state();

	if (!animstate || player->get_move_type() == MOVETYPE_LADDER || player->get_move_type() == MOVETYPE_NOCLIP || player->m_bGunGameImmunity() || configs.player_list.disable_resolver[player_record->index])
		return;

	const auto beforeAnimYaw = animstate->m_flGoalFeetYaw;

	bool extending = player_record->layers[ANIMATION_LAYER_ADJUST].m_flCycle == 0.f && player_record->layers[ANIMATION_LAYER_ADJUST].m_flWeight == 0.f;

	if (player->m_flFlashDuration() > 75.0f && player->m_angEyeAngles().x > -55.f && player->m_angEyeAngles().x < 55.f && extending) // flashed or shooting not fakeangle (hideshots exception, so check pitch)
		return;

	if (player_record->shot && fabs(player->m_angEyeAngles().x) < 30.f) {
		if (math::angle_difference(player->m_angEyeAngles().y, animstate->m_flGoalFeetYaw) > 35.0f)
		{
			player->get_animation_state()->m_flGoalFeetYaw = player_record->eye_position.y + player->get_max_desync_delta();
		}
		else if (math::angle_difference(player->m_angEyeAngles().y, animstate->m_flGoalFeetYaw) < -35.0f)
		{
			player->get_animation_state()->m_flGoalFeetYaw = player_record->eye_position.y + player->get_min_desync_delta();
		}
		else {
			player->get_animation_state()->m_flGoalFeetYaw = player_record->eye_position.y;
		}

		return;
	}

	auto type = ResolverType::ORIGINAL;

	auto delta = math::normalize_yaw(player->m_angEyeAngles().y - zero_goal_feet_yaw);
	auto valid_lby = true;

	if (animstate->m_flVelocityLengthXY > 0.1f || fabsf(animstate->m_flVelocityLengthZ) > 100.f)
		valid_lby = animstate->m_flTimeSinceStartedMoving < 0.22f;

	if (player_record->layers[ANIMATION_LAYER_ADJUST].m_flWeight == 0.f && player_record->layers[ANIMATION_LAYER_ADJUST].m_flCycle == 0.f && player_record->layers[ANIMATION_LAYER_MOVEMENT_MOVE].m_flWeight == 0.f)
	{
		auto delta = math::angle_difference(player->m_angEyeAngles().y, zero_goal_feet_yaw);
		auto positive_resolver = (2 * (delta <= 0.0f) - 1) > 0;
		player_record->side = positive_resolver ? ResolverSide::RESOLVER_SECOND : ResolverSide::RESOLVER_FIRST;
		side = positive_resolver;
		type = ResolverType::LBY;
	}
	else if (fabsf(delta) > 28.0f && valid_lby)
	{
		if (cheat.globals.missed_shots[player->EntIndex()])
			delta = -delta;

		player_record->type = ResolverType::LBY;

		if (delta > 28.0f)
			player_record->side = ResolverSide::RESOLVER_FIRST;
		else if (delta < -28.0f)
			player_record->side = ResolverSide::RESOLVER_SECOND;
	}
	else if (m_globals()->m_curtime - lock_side > 2.0f)
	{
		auto fire_data_first = autowall::get().wall_penetration(cheat.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.first), player);
		auto fire_data_second = autowall::get().wall_penetration(cheat.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.second), player);
		auto fire_data_center = autowall::get().wall_penetration(cheat.globals.eye_pos, player->hitbox_position_matrix(HITBOX_HEAD, player_record->matrixes_data.zero), player);

		if (fire_data_first.visible != fire_data_second.visible && fire_data_center.visible != fire_data_first.visible)
		{
			side = fire_data_second.visible;
			lock_side = m_globals()->m_curtime;
		}
		else
		{
			if (fabsf(fire_data_first.damage - fire_data_second.damage) > 10.f) //&& (fire_data_center.visible - fabs(fire_data_second.damage) > 100); // distance.
				side = fire_data_first.damage > fire_data_second.damage && fabs(fire_data_center.damage > fire_data_first.damage) > 1; //&& fire_data_first.visible > fire_data_center.visible;
		}
	}
	else
	{
		type = ResolverType::TRACE;
	}

	if (cheat.globals.missed_shots[player->EntIndex()])
	{
		switch (last_side)
		{
		case ResolverSide::RESOLVER_ORIGINAL:
			cheat.globals.missed_shots[player->EntIndex()] = 0;
			break;
		case ResolverSide::RESOLVER_ZERO:
			player_record->type = ResolverType::BRUTEFORCE;

			player_record->side = side ? ResolverSide::RESOLVER_FIRST : ResolverSide::RESOLVER_SECOND;

			was_first_bruteforce = false;
			was_second_bruteforce = false;
			break;
		case ResolverSide::RESOLVER_FIRST:
			player_record->type = type;

			player_record->side = was_second_bruteforce ? ResolverSide::RESOLVER_ZERO : ResolverSide::RESOLVER_SECOND;

			was_first_bruteforce = true;

			was_first_low_bruteforce = false;
			was_second_low_bruteforce = false;
			break;
		case ResolverSide::RESOLVER_SECOND:
			player_record->type = type;

			player_record->side = was_first_bruteforce ? ResolverSide::RESOLVER_ZERO : ResolverSide::RESOLVER_FIRST;

			was_second_bruteforce = true;

			was_first_low_bruteforce = false;
			was_second_low_bruteforce = false;
			break;
		case ResolverSide::RESOLVER_LOW_FIRST:
			player_record->type = type;

			player_record->side = was_second_low_bruteforce && player_record->side == ResolverSide::RESOLVER_FIRST ? ResolverSide::RESOLVER_LOW_FIRST : ResolverSide::RESOLVER_FIRST;

			was_first_low_bruteforce = true;
			break;
		case ResolverSide::RESOLVER_LOW_SECOND:
			player_record->type = type;

			player_record->side = was_second_low_bruteforce && player_record->side == ResolverSide::RESOLVER_SECOND ? ResolverSide::RESOLVER_LOW_SECOND : ResolverSide::RESOLVER_SECOND;

			was_second_low_bruteforce = true;
			break;
		}
	}

	if (backup_side != ResolverSide::RESOLVER_ORIGINAL)
	{
		if (configs.ragebot.adaptive_resolver)
			player_record->side = backup_side;
		else
			backup_side = ResolverSide::RESOLVER_ORIGINAL;
	}

	/*
	const auto is_overriding = key_binds::get().get_key_bind_state(key_bind_features::RESOLVER_OVERRIDE);
	const auto player_desync_delta = player->get_max_desync_delta();
	const auto low_player_desync_delta = player_desync_delta / 2;
	const auto player_desync_delta_min = player->get_min_desync_delta();
	const auto low_player_desync_delta_min = player_desync_delta_min / 2;
	const auto max_delta = is_overriding ? player_desync_delta_min : player_desync_delta;
	const auto low_delta = is_overriding ? low_player_desync_delta_min : low_player_desync_delta;
	const auto currentYaw = 0;//(animstate->m_flCurrentFeetYaw - animstate->m_flGoalFeetYaw);
	const auto currentEyeYaw = player->m_angEyeAngles().y;

	switch (player_record->side)
	{
	case ResolverSide::RESOLVER_ZERO:
		animstate->m_flGoalFeetYaw = currentEyeYaw;
		break;
	case ResolverSide::RESOLVER_FIRST:
		animstate->m_flGoalFeetYaw = currentEyeYaw + (max_delta - currentYaw);
		break;
	case ResolverSide::RESOLVER_SECOND:
		animstate->m_flGoalFeetYaw = currentEyeYaw - (max_delta + currentYaw);
		break;
	case ResolverSide::RESOLVER_LOW_FIRST:
		animstate->m_flGoalFeetYaw = currentEyeYaw + (low_delta - currentYaw);
		break;
	case ResolverSide::RESOLVER_LOW_SECOND:
		animstate->m_flGoalFeetYaw = currentEyeYaw - (low_delta + currentYaw);
		break;
	}*/

	test(player);

	auto& info = resolver_info{ player->EntIndex(), true, beforeAnimYaw, player->get_animation_state()->m_flGoalFeetYaw, cheat.globals.missed_shots[player->EntIndex()], player_record->shot };

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_resolver")))
			current.func(info);

	player->get_animation_state()->m_flGoalFeetYaw = info.current_animation;
}

void resolver::resolve_pitch()
{
	if (!configs.ragebot.resolver[RESOLVER_PITCH])
		return;

	PlayerInfo player_info;
	m_engine()->GetPlayerInfo(player->EntIndex(), &player_info);
	auto animstate = player->get_animation_state();

	if (!animstate || player->get_move_type() == MOVETYPE_LADDER || player->get_move_type() == MOVETYPE_NOCLIP || configs.player_list.disable_resolver[player_record->index])
		return;

	auto beforeAnimPitch = player->m_angEyeAngles().x;

	float pitch = 0;
	if (beforeAnimPitch < -179)
		pitch = (beforeAnimPitch + 360);
	else if (beforeAnimPitch > 90 || beforeAnimPitch < -90)
		pitch = 89;
	else if (beforeAnimPitch > 89 && beforeAnimPitch < 91)
		pitch = (beforeAnimPitch - 90);
	else if (beforeAnimPitch > 179 && beforeAnimPitch < 181)
		pitch = (beforeAnimPitch - 180);
	else if (beforeAnimPitch > -179 && beforeAnimPitch < -181)
		pitch = (beforeAnimPitch + 180);
	else if (fabs(beforeAnimPitch) == 0)
		pitch = copysign(89, beforeAnimPitch);

	auto& info = resolver_info{ player->EntIndex(), false, beforeAnimPitch, pitch, cheat.globals.missed_shots[player->EntIndex()], player_record->shot };

	if (cheat.globals.loaded_script)
		for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_resolver")))
			current.func(info);

	player->m_angEyeAngles().x = info.current_animation;
}