#include "ragebot.h"
#include "autowall.h"
#include "..\misc\misc.h"
#include "..\misc\logs.h"
#include "..\movement\slowwalk.h"
#include "..\legitbot\backtrack.h"
#include "..\serverside\local_animations.h"

void ragebot::run(CUserCmd* cmd)
{
	backup.clear();
	targets.clear();
	scanned_targets.clear();
	final_target.reset();
	should_stop = false;

	if (!configs.ragebot.enabled)
		return;

	automatic_revolver(cmd);
	prepare_targets();

	if (cheat.globals.weapon->is_non_aim() || cheat.globals.current_weapon < 0)
		return;

	currentWeapon = configs.ragebot.weapon[cheat.globals.current_weapon].enabled ? configs.ragebot.weapon[cheat.globals.current_weapon] : configs.ragebot.weapon[0];

	scan_targets();

	if (!should_stop && currentWeapon.autostop_modifiers[AUTOSTOP_PREDICTIVE])
	{
		auto max_speed = 260.0f;
		auto weapon_info = cheat.globals.weapon->get_weapon_info();

		if (weapon_info)
			max_speed = cheat.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;

		auto ticks_to_stop = math::clamp(engineprediction::get().backup_data.velocity.Length2D() / max_speed * 3.0f, 0.0f, 4.0f);
		auto predicted_eye_pos = cheat.globals.eye_pos + engineprediction::get().backup_data.velocity * m_globals()->m_intervalpertick * ticks_to_stop;

		for (auto& target : targets)
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data, predicted_eye_pos, true);

			if (!last_data.valid())
				continue;

			should_stop = true;
			break;
		}
	}

	if (!automatic_stop(cmd))
		return;

	if (scanned_targets.empty())
		return;

	find_best_target();

	if (!final_target.data.valid())
		return;

	fire(cmd);
}

void ragebot::automatic_revolver(CUserCmd* cmd)
{
	if (cmd->m_buttons & IN_ATTACK || cmd->m_buttons & IN_ATTACK2 || cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
		return;

	static auto r8cock_time = 0.0f;
	auto server_time = TICKS_TO_TIME(cheat.globals.backup_tickbase);

	if (cheat.globals.weapon->can_fire(false))
	{
		if (r8cock_time <= server_time)
		{
			if (cheat.globals.weapon->m_flNextSecondaryAttack() <= server_time)
				r8cock_time = server_time + 0.234375f;
			else
				cmd->m_buttons |= IN_ATTACK2;
		}
		else
			cmd->m_buttons |= IN_ATTACK;
	}
	else
	{
		r8cock_time = server_time + 0.234375f;
		cmd->m_buttons &= ~IN_ATTACK;
	}

	cheat.globals.revolver_working = true;
}

void ragebot::prepare_targets()
{
	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		if (configs.player_list.white_list[i])
			continue;

		auto e = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		if (!e->valid(true, false))
			continue;

		if (e->m_flSimulationTime() < e->m_flOldSimulationTime())
			continue;

		auto records = &player_records[i];

		if (records->empty())
			continue;

		targets.emplace_back(target(e, get_record(records, false), get_record(records, true)));
	}

	if (targets.size() >= 4)
	{
		auto first = rand() % targets.size();
		auto second = rand() % targets.size();
		auto third = rand() % targets.size();

		for (auto i = 0; i < targets.size(); ++i)
		{
			if (i == first || i == second || i == third)
				continue;

			targets.erase(targets.begin() + i);

			if (i > 0)
				--i;
		}
	}

	for (auto& target : targets)
		backup.emplace_back(adjust_data(target.e));
}

static bool compare_records(const optimized_adjust_data& first, const optimized_adjust_data& second)
{
	auto first_pitch = math::normalize_pitch(first.angles.x);
	auto second_pitch = math::normalize_pitch(second.angles.x);

	if (fabs(first_pitch - second_pitch) > 15.0f)
		return fabs(first_pitch) < fabs(second_pitch);
	else if (first.shot != second.shot)
		return first.shot;
	else if (first.duck_amount != second.duck_amount)
		return first.duck_amount < second.duck_amount;
	else if (first.origin != second.origin)
		return first.origin.DistTo(localPlayer->GetAbsOrigin()) < second.origin.DistTo(localPlayer->GetAbsOrigin());

	return first.simulation_time > second.simulation_time;
}

adjust_data* ragebot::get_record(std::deque<adjust_data>* records, bool history)
{
	if (history)
	{
		std::deque<optimized_adjust_data> optimized_records;

		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);
			optimized_adjust_data optimized_record;

			optimized_record.index = i;
			optimized_record.player = record->player;
			optimized_record.simulation_time = record->simulation_time;
			optimized_record.duck_amount = record->duck_amount;
			optimized_record.angles = record->angles;
			optimized_record.origin = record->origin;
			optimized_record.shot = record->shot;

			optimized_records.emplace_back(optimized_record);
		}

		if (optimized_records.size() < 2)
			return nullptr;

		std::sort(optimized_records.begin(), optimized_records.end(), compare_records);

		for (auto& optimized_record : optimized_records)
		{
			auto record = &records->at(optimized_record.index);

			if (!record->valid())
				continue;

			return record;
		}
	}
	else
	{
		for (auto i = 0; i < records->size(); ++i)
		{
			auto record = &records->at(i);

			if (!record->valid())
				continue;

			return record;
		}
	}

	return nullptr;
}

int ragebot::get_minimum_damage(bool visible, int health)
{
	auto weaponData = cheat.globals.weapon->get_weapon_info();
	auto minimum_damage = 0;

	if (visible)
	{
		if (weaponData && currentWeapon.minimum_visible_damage < 1)
			minimum_damage = min(weaponData->iDamage / 3, health / 4);
		else if (currentWeapon.minimum_visible_damage > 100)
			minimum_damage = (currentWeapon.minimum_visible_damage - 100) + health;
		else
			minimum_damage = std::clamp(currentWeapon.minimum_visible_damage, 1, health);
	}
	else
	{
		if (weaponData && currentWeapon.minimum_damage < 1)
			minimum_damage = min(weaponData->iDamage / 3, health / 4);
		else if (currentWeapon.minimum_damage > 100)
			minimum_damage = (currentWeapon.minimum_damage - 100) + health;
		else
			minimum_damage = std::clamp(currentWeapon.minimum_damage, 1, health);
	}

	if (key_binds::get().get_key_bind_state(key_bind_features::DAMAGE_OVERRIDE))
	{
		if (currentWeapon.minimum_override_damage > 100)
			minimum_damage = (currentWeapon.minimum_override_damage - 100) + health;
		else
			minimum_damage = std::clamp(currentWeapon.minimum_override_damage, 1, health);
	}

	return minimum_damage;
}

void ragebot::scan_targets()
{
	if (targets.empty())
		return;

	for (auto& target : targets)
	{
		if (target.history_record->valid())
		{
			scan_data last_data;

			if (target.last_record->valid())
			{
				target.last_record->adjust_player();
				scan(target.last_record, last_data);
			}

			scan_data history_data;

			target.history_record->adjust_player();
			scan(target.history_record, history_data);

			if (last_data.valid() && last_data.damage > history_data.damage)
				scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
			else if (history_data.valid())
				scanned_targets.emplace_back(scanned_target(target.history_record, history_data));
		}
		else
		{
			if (!target.last_record->valid())
				continue;

			scan_data last_data;

			target.last_record->adjust_player();
			scan(target.last_record, last_data);

			if (!last_data.valid())
				continue;

			scanned_targets.emplace_back(scanned_target(target.last_record, last_data));
		}
	}
}

bool ragebot::automatic_stop(CUserCmd* cmd)
{
	if (!should_stop)
		return true;

	if (!currentWeapon.autostop)
		return true;

	if (cheat.globals.slowwalking)
		return true;

	if (!(localPlayer->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return true;

	if (cheat.globals.weapon->is_empty())
		return true;

	if (!currentWeapon.autostop_modifiers[AUTOSTOP_BETWEEN_SHOTS] && !cheat.globals.weapon->can_fire(false))
		return true;

	auto animlayer = localPlayer->get_animlayers()[ANIMATION_LAYER_WEAPON_ACTION];

	if (animlayer.m_iSequence)
	{
		auto activity = localPlayer->sequence_activity(animlayer.m_iSequence);

		if (activity == ACT_CSGO_RELOAD && animlayer.m_flWeight > 0.0f)
			return true;
	}

	auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (!weapon_info)
		return true;

	auto get_standing_accuracy = [&]() -> const float
		{
			const auto max_speed = cheat.globals.weapon->m_zoomLevel() > 0 ? weapon_info->flMaxPlayerSpeed : weapon_info->flMaxPlayerSpeedAlt;
			return max_speed / 3.f;
		};

	auto& velocity = localPlayer->m_vecVelocity();
	float speed = velocity.Length2D();
	float max_speed = (cheat.globals.weapon->m_zoomLevel() == 0 ? weapon_info->flMaxPlayerSpeed : weapon_info->flMaxPlayerSpeedAlt) * 0.1f;

	if ((speed <= get_standing_accuracy() / 2) && currentWeapon.autostop_modifiers[AUTOSTOP_FORCE_ACCURACY])
	{
		slowwalk::get().create_move(cmd);
	}
	else
	{
		Vector direction;

		math::vector_angles(velocity, direction);

		direction.y = cmd->m_viewangles.y - direction.y;

		Vector forward;
		math::angle_vectors(direction, forward);

		const auto negatedDirection = forward * -speed;
		cmd->m_forwardmove = negatedDirection.x;
		cmd->m_sidemove = negatedDirection.y;

		if (currentWeapon.autostop_modifiers[AUTOSTOP_FORCE_ACCURACY])
			return false;
	}

	return true;
}

bool ragebot::bounding_box_check(adjust_data* record)
{
	const auto collideable = record->player->GetCollideable();
	if (!collideable)
		return false;

	const auto bbmin = record->mins + record->origin;
	const auto bbmax = record->maxs + record->origin;

	Vector points[7];
	points[0] = record->player->hitbox_position_matrix(HITBOX_HEAD, record->matrixes_data.main);
	points[1] = (bbmin + bbmax) * 0.5f;
	points[2] = Vector((bbmax.x + bbmin.x) * 0.5f, (bbmax.y + bbmin.y) * 0.5f, bbmin.z);
	points[3] = bbmax;
	points[4] = Vector(bbmax.x, bbmin.y, bbmax.z);
	points[5] = Vector(bbmin.x, bbmin.y, bbmax.z);
	points[6] = Vector(bbmin.x, bbmax.y, bbmax.z);

	for (auto& point : points)
		return autowall::get().wall_penetration(cheat.globals.eye_pos, point, record->player).valid;

	return false;
};

static bool compare_points(const scan_point& first, const scan_point& second)
{
	return !first.center && first.hitbox == second.hitbox;
}

bool ragebot::baimHitboxesCheck(int hitbox)
{
	switch (hitbox)
	{
	case HITBOX_PELVIS:
		return !currentWeapon.body_aim_hitboxes.at(4);
		break;
	case HITBOX_STOMACH:
		return !currentWeapon.body_aim_hitboxes.at(3);
		break;
	case HITBOX_LOWER_CHEST:
		return !currentWeapon.body_aim_hitboxes.at(2);
		break;
	case HITBOX_CHEST:
		return !currentWeapon.body_aim_hitboxes.at(1);
		break;
	case HITBOX_UPPER_CHEST:
		return !currentWeapon.body_aim_hitboxes.at(0);
		break;
	case HITBOX_RIGHT_THIGH:
	case HITBOX_LEFT_THIGH:
	case HITBOX_RIGHT_CALF:
	case HITBOX_LEFT_CALF:
		return !currentWeapon.body_aim_hitboxes.at(6);
		break;
	case HITBOX_RIGHT_FOOT:
	case HITBOX_LEFT_FOOT:
		return !currentWeapon.body_aim_hitboxes.at(7);
		break;
	case HITBOX_RIGHT_UPPER_ARM:
	case HITBOX_RIGHT_LOWER_ARM:
	case HITBOX_LEFT_UPPER_ARM:
	case HITBOX_LEFT_LOWER_ARM:
		return !currentWeapon.body_aim_hitboxes.at(5);
		break;
	default:
		return true;
		break;
	}
}

void ragebot::scan(adjust_data* record, scan_data& data, const Vector& shoot_position, bool optimized)
{
	auto weapon = optimized ? localPlayer->getActiveWeapon() : cheat.globals.weapon;

	if (!weapon)
		return;

	auto weapon_info = weapon->get_weapon_info();

	if (!weapon_info)
		return;

	auto hitboxes = get_hitboxes(record, optimized);

	if (hitboxes.empty())
		return;

	auto force_safe_points = ((record->player->m_iHealth() <= weapon_info->iDamage) && currentWeapon.safe_points_conditions[SAFEPOINTS_LOWDAMAGE]) || key_binds::get().get_key_bind_state(key_bind_features::SAFE_POINT) || configs.player_list.force_safe_points[record->index] || (currentWeapon.max_misses && cheat.globals.missed_shots[record->index] >= currentWeapon.max_misses_amount);
	auto best_damage = 0;

	//auto best_body_damage = 0.f;
	//auto prev_hitbox = -1;
	//scan_data best_data;

	auto minimum_damage = get_minimum_damage(false, record->player->m_iHealth());
	auto minimum_visible_damage = get_minimum_damage(true, record->player->m_iHealth());

	auto get_hitgroup = [](const int& hitbox)
		{
			if (hitbox == HITBOX_HEAD)
				return 0;
			else if (hitbox == HITBOX_PELVIS)
				return 1;
			else if (hitbox == HITBOX_STOMACH)
				return 2;
			else if (hitbox >= HITBOX_LOWER_CHEST && hitbox <= HITBOX_UPPER_CHEST)
				return 3;
			else if (hitbox >= HITBOX_RIGHT_THIGH && hitbox <= HITBOX_LEFT_FOOT)
				return 4;
			else if (hitbox >= HITBOX_RIGHT_HAND && hitbox <= HITBOX_LEFT_LOWER_ARM)
				return 5;

			return -1;
		};

	std::vector<scan_point> points;

	for (auto& hitbox : hitboxes)
	{
		if (optimized)
		{
			points.emplace_back(scan_point(record->player->hitbox_position_matrix(hitbox, record->matrixes_data.main), hitbox, true));
			continue;
		}

		auto current_points = get_points(record, hitbox);

		for (auto& point : current_points)
		{
			//point.safe = record->robot || (hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point) && hitbox_intersection(record->player, record->matrixes_data.first, hitbox, shoot_position, point.point) && hitbox_intersection(record->player, record->matrixes_data.second, hitbox, shoot_position, point.point));

			if (!record->robot)
			{
				auto safe = 0.0f;

				if (record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.first[0].GetOrigin() || record->matrixes_data.zero[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin() || record->matrixes_data.first[0].GetOrigin() == record->matrixes_data.second[0].GetOrigin())
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.zero, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.first, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;
				else if (!hitbox_intersection(record->player, record->matrixes_data.second, hitbox, shoot_position, point.point, &safe))
					safe = 0.0f;

				point.safe = safe;
			}
			else
				point.safe = 1.0f;

			if (!force_safe_points || point.safe)
				points.emplace_back(point);
		}
	}

	/*
	if (!optimized)
	{
		for (auto& point : points)
		{
			if (points.empty())
				return;

			if (point.hitbox == HITBOX_HEAD)
				continue;

			for (auto it = points.begin(); it != points.end(); ++it)
			{
				if (point.point == it->point)
					continue;

				auto first_angle = math::calculate_angle(shoot_position, point.point);
				auto second_angle = math::calculate_angle(shoot_position, it->point);

				auto distance = shoot_position.DistTo(point.point);
				auto fov = math::fast_sin(DEG2RAD(math::get_fov(first_angle, second_angle))) * distance;

				if (fov < 5.0f)
				{
					points.erase(it);
					break;
				}
			}
		}
	}
	*/

	if (points.empty())
		return;

	if (!optimized)
	{
		std::sort(points.begin(), points.end(), compare_points);

		if (points.size() > 7 && !bounding_box_check(record))
			return;
	}

	auto body_hitboxes = true;

	for (auto& point : points)
	{
		if (!optimized && !point.safe)
		{
			if (currentWeapon.safe_points_conditions[SAFEPOINTS_VISIBLE] && util::visible(cheat.globals.eye_pos, point.point, record->player, localPlayer.get()))
				continue;

			if (currentWeapon.safe_points_conditions[SAFEPOINTS_INAIR] && !(record->flags & FL_ONGROUND))
				continue;

			if (currentWeapon.safe_points_conditions[SAFEPOINTS_INCROUCH] && (record->flags & FL_ONGROUND || currentWeapon.safe_points_conditions[SAFEPOINTS_INAIR]) && record->flags & FL_DUCKING)
				continue;

			if (currentWeapon.safe_points_conditions[SAFEPOINTS_ONLIMBS] && (point.hitbox >= HITBOX_RIGHT_THIGH && point.hitbox < HITBOX_MAX))
				continue;
		}

		if (!optimized && (currentWeapon.safe_points_conditions[SAFEPOINTS_PREFER] || force_safe_points) && data.point.safe && data.point.safe != point.safe)
			continue;

		auto fire_data = autowall::get().wall_penetration(shoot_position, point.point, record->player);

		if (!fire_data.valid)
			continue;

		if (fire_data.damage < 1)
			continue;

		if (!fire_data.visible && configs.ragebot.autowall == 0)
			continue;

		if (!optimized && get_hitgroup(fire_data.hitbox) != get_hitgroup(point.hitbox))
			continue;

		auto current_minimum_damage = fire_data.visible ? minimum_visible_damage : minimum_damage;

		/*
		if (currentWeapon.safe_points_conditions[SAFEPOINTS_PREFER]) {
			if (point.safe && point.hitbox >= HITBOX_PELVIS && point.hitbox <= HITBOX_UPPER_CHEST && currentWeapon.body_aim_conditions[BODYAIMCOND_PREFER]) {
				if ((fire_data.damage > best_body_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_body_damage + 20.f) {
					if (fire_data.damage >= current_minimum_damage)
					{
						best_body_damage = best_damage = fire_data.damage;
						best_data.point = point;
						best_data.visible = fire_data.visible;
						best_data.damage = fire_data.damage;
						best_data.hitbox = fire_data.hitbox;
						prev_hitbox = point.hitbox;
					}
				}
			}
			else if (point.safe && point.hitbox >= HITBOX_PELVIS && point.hitbox <= HITBOX_UPPER_CHEST && fire_data.damage >= record->player->m_iHealth() + 5) {
				if (fire_data.damage >= current_minimum_damage && ((fire_data.damage > best_body_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_body_damage + 20.f)) {
					best_body_damage = best_damage = fire_data.damage;
					best_data.point = point;
					best_data.visible = fire_data.visible;
					best_data.damage = fire_data.damage;
					best_data.hitbox = fire_data.hitbox;
					prev_hitbox = point.hitbox;
				}
			}
			else if (point.safe && (fire_data.damage > best_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_damage + 20.f) {
				if (fire_data.damage >= current_minimum_damage)
				{
					best_damage = fire_data.damage;
					best_data.point = point;
					best_data.visible = fire_data.visible;
					best_data.damage = fire_data.damage;
					best_data.hitbox = fire_data.hitbox;
					prev_hitbox = point.hitbox;
				}
			}
			else if (fire_data.damage >= current_minimum_damage && ((fire_data.damage > best_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_damage + 20.f)) {
				best_damage = fire_data.damage;
				best_data.point = point;
				best_data.visible = fire_data.visible;
				best_data.damage = fire_data.damage;
				best_data.hitbox = fire_data.hitbox;
				prev_hitbox = point.hitbox;
			}
		}
		else if (point.hitbox >= HITBOX_PELVIS && point.hitbox <= HITBOX_UPPER_CHEST && currentWeapon.body_aim_conditions[BODYAIMCOND_PREFER]) {
			if ((fire_data.damage > best_body_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_body_damage + 20.f) {
				if (fire_data.damage >= current_minimum_damage)
				{
					best_body_damage = best_damage = fire_data.damage;
					best_data.point = point;
					best_data.visible = fire_data.visible;
					best_data.damage = fire_data.damage;
					best_data.hitbox = fire_data.hitbox;
					prev_hitbox = point.hitbox;
				}
			}
		}
		else if (point.hitbox >= HITBOX_PELVIS && point.hitbox <= HITBOX_UPPER_CHEST && fire_data.damage >= record->player->m_iHealth() + 5) {
			if (fire_data.damage >= current_minimum_damage && ((fire_data.damage > best_body_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_body_damage + 20.f)) {
				best_body_damage = best_damage = fire_data.damage;
				best_data.point = point;
				best_data.visible = fire_data.visible;
				best_data.damage = fire_data.damage;
				best_data.hitbox = fire_data.hitbox;
				prev_hitbox = point.hitbox;
			}
		}
		else if (fire_data.damage >= current_minimum_damage && ((fire_data.damage > best_damage && prev_hitbox != point.hitbox) || fire_data.damage > best_damage + 20.f)) {
			best_damage = fire_data.damage;
			best_data.point = point;
			best_data.visible = fire_data.visible;
			best_data.damage = fire_data.damage;
			best_data.hitbox = fire_data.hitbox;
			prev_hitbox = point.hitbox;
		}
		*/

		//body_hitboxes && 
		if (!optimized && baimHitboxesCheck(point.hitbox))
		{
			body_hitboxes = false;

			if (configs.player_list.force_body_aim[record->index])
				continue;

			if (key_binds::get().get_key_bind_state(key_bind_features::BODY_AIM))
				continue;

			autowall::returninfo_t baim_data = autowall::returninfo_t();

			for (auto& maybe_point : points)
				if (!baimHitboxesCheck(maybe_point.hitbox))
				{
					baim_data = autowall::get().wall_penetration(shoot_position, maybe_point.point, record->player);
					break;
				}

			if (currentWeapon.body_aim_conditions[BODYAIMCOND_DOUBLETAP] && (baim_data.valid || !currentWeapon.dynamic_body_aim_conditions) && misc::get().double_tap_key)
				continue;

			if (currentWeapon.body_aim_conditions[BODYAIMCOND_PREFER] && baim_data.valid)
			{
				if (currentWeapon.dynamic_body_aim_conditions)
				{
					if (baim_data.damage >= current_minimum_damage)
						continue;
				}
				else
					continue;
			}

			if (currentWeapon.body_aim_conditions[BODYAIMCOND_INCROUCH] && (baim_data.valid || !currentWeapon.dynamic_body_aim_conditions) && record->flags & FL_DUCKING)
				continue;

			if (currentWeapon.body_aim_conditions[BODYAIMCOND_INAIR] && (baim_data.valid || !currentWeapon.dynamic_body_aim_conditions) && !(record->flags & FL_ONGROUND))
				continue;
		}

		if (fire_data.damage >= current_minimum_damage && fire_data.damage >= best_damage)
		{
			if (!optimized && !should_stop)
			{
				should_stop = true;

				if (currentWeapon.autostop_modifiers[AUTOSTOP_LETHAL] && fire_data.damage < record->player->m_iHealth())
					should_stop = false;
				else if (currentWeapon.autostop_modifiers[AUTOSTOP_VISIBLE] && !fire_data.visible)
					should_stop = false;
				else if (currentWeapon.autostop_modifiers[AUTOSTOP_CENTER] && !point.center)
					should_stop = false;
			}

			if (!optimized && force_safe_points && !point.safe)
				continue;

			best_damage = fire_data.damage;

			data.point = point;
			data.visible = fire_data.visible;
			data.damage = best_damage;
			data.hitbox = fire_data.hitbox;

			//data.point = best_data.point;
			//data.visible = best_data.visible;
			//data.damage = best_data.damage;
			//data.hitbox = best_data.hitbox;

			//if (force_safe_points && point.safe)
			//	break;

			if (optimized)
				break;
			//return;
		}
	}
}

std::vector<int> ragebot::get_hitboxes(adjust_data* record, bool optimized)
{
	std::vector<int> hitboxes;

	if (optimized)
	{
		hitboxes.emplace_back(HITBOX_HEAD);
		hitboxes.emplace_back(HITBOX_CHEST);
		hitboxes.emplace_back(HITBOX_STOMACH);
		hitboxes.emplace_back(HITBOX_PELVIS);

		return hitboxes;
	}

	// prefer, high speed.
	if (currentWeapon.head_aim_conditions[HEADIMCOND_AIR] && !(record->player->m_fFlags() & FL_ONGROUND) && static_cast<std::vector<int>::iterator>(std::find(hitboxes.begin(), hitboxes.end(), HITBOX_HEAD)) == hitboxes.end())
		hitboxes.emplace_back(HITBOX_HEAD);

	// prefer, on shot.
	if (currentWeapon.head_aim_conditions[HEADIMCOND_ONSHOT] && record->shot && static_cast<std::vector<int>::iterator>(std::find(hitboxes.begin(), hitboxes.end(), HITBOX_HEAD)) == hitboxes.end())
		hitboxes.emplace_back(HITBOX_HEAD);

	// only, high speed.
	if (currentWeapon.head_aim_only_conditions[HEADIMCOND_ONLY_AIR] && !(record->player->m_fFlags() & FL_ONGROUND))
	{
		if (static_cast<std::vector<int>::iterator>(std::find(hitboxes.begin(), hitboxes.end(), HITBOX_HEAD)) == hitboxes.end())
			hitboxes.emplace_back(HITBOX_HEAD);
		return hitboxes;
	}

	// only, on shot.
	if (currentWeapon.head_aim_only_conditions[HEADIMCOND_ONLY_ONSHOT] && record->shot)
	{
		if (static_cast<std::vector<int>::iterator>(std::find(hitboxes.begin(), hitboxes.end(), HITBOX_HEAD)) == hitboxes.end())
			hitboxes.emplace_back(HITBOX_HEAD);
		return hitboxes;
	}

	float desired_speed = .1f;

	// get weapon.
	weapon_t* weapon = record->player->getActiveWeapon();
	if (weapon)
	{
		// get weapon info.
		weapon_info_t* weapon_info = weapon->get_weapon_info();
		if (weapon_info)
		{
			// get the max possible speed whilest we are still accurate.
			desired_speed = ((record->player->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed) * 0.33000001f);
		}
	}

	bool ignore_limbs = record->velocity.Length2D() > desired_speed && currentWeapon.ignore_limbs;

	if (currentWeapon.hitboxes.at(0) && static_cast<std::vector<int>::iterator>(std::find(hitboxes.begin(), hitboxes.end(), HITBOX_HEAD)) == hitboxes.end())
		hitboxes.emplace_back(HITBOX_HEAD);

	if (currentWeapon.hitboxes.at(1))
		hitboxes.emplace_back(HITBOX_UPPER_CHEST);

	if (currentWeapon.hitboxes.at(2))
		hitboxes.emplace_back(HITBOX_CHEST);

	if (currentWeapon.hitboxes.at(3))
		hitboxes.emplace_back(HITBOX_LOWER_CHEST);

	if (currentWeapon.hitboxes.at(4))
		hitboxes.emplace_back(HITBOX_STOMACH);

	if (currentWeapon.hitboxes.at(5))
		hitboxes.emplace_back(HITBOX_PELVIS);

	if (currentWeapon.hitboxes.at(6) && !ignore_limbs)
	{
		hitboxes.emplace_back(HITBOX_RIGHT_UPPER_ARM);
		hitboxes.emplace_back(HITBOX_LEFT_UPPER_ARM);
	}

	if (currentWeapon.hitboxes.at(7))
	{
		hitboxes.emplace_back(HITBOX_RIGHT_THIGH);
		hitboxes.emplace_back(HITBOX_LEFT_THIGH);

		hitboxes.emplace_back(HITBOX_RIGHT_CALF);
		hitboxes.emplace_back(HITBOX_LEFT_CALF);
	}

	if (currentWeapon.hitboxes.at(8) && !ignore_limbs)
	{
		hitboxes.emplace_back(HITBOX_RIGHT_FOOT);
		hitboxes.emplace_back(HITBOX_LEFT_FOOT);
	}

	return hitboxes;
}

float ragebot::GetBodyScale(player_t* player)
{
	if (currentWeapon.scan_point_mode == 1 && currentWeapon.body_scale)
		return currentWeapon.body_scale;

	auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (!weapon_info)
		return 0.f;

	auto factor = [](float x, float min, float max) {
		return 1.f - 1.f / (1.f + pow(2.f, (-([](float x, float min, float max) {
			return ((x - min) * 2.f) / (max - min) - 1.f;
			}(x, min, max)) / 0.115f)));
		}(player->m_vecOrigin().DistTo(cheat.globals.eye_pos), 0.f, weapon_info->flRange / 4.f);

		if (cheat.globals.weapon->is_sniper() && !localPlayer->m_bIsScoped())
			factor = 0.f;

		return std::clamp(factor, 0.f, 0.75f);
}

float ragebot::GetHeadScale(player_t* player)
{
	if (currentWeapon.scan_point_mode == 1 && currentWeapon.head_scale)
		return currentWeapon.head_scale;

	if (player->m_fFlags() & FL_ONGROUND)
		return GetBodyScale(player);
	else
		return 0.75f;
}

std::vector<scan_point> ragebot::get_points(adjust_data* record, int hitbox, bool from_aim)
{
	std::vector<scan_point> points;

	auto model = record->player->GetModel();

	if (!model)
		return points;

	auto hdr = m_modelinfo()->GetStudioModel(model);

	if (!hdr)
		return points;

	auto set = hdr->getHitboxSet(record->player->m_nHitboxSet());

	if (!set)
		return points;

	auto bbox = set->getHitbox(hitbox);

	if (!bbox)
		return points;

	/*
	if (!currentWeapon.scan_point_mode)
	{
		Vector min, max;
		math::vector_transform(bbox->bbmin, record->matrixes_data.main[bbox->bone], min);
		math::vector_transform(bbox->bbmax, record->matrixes_data.main[bbox->bone], max);
		points.emplace_back(scan_point((min + max) * 0.5f, hitbox, true));
		return points;
	}
	*/
	if (currentWeapon.scan_point_mode == 2)
	{
		float scale = currentWeapon.multipoints_scale;

		// reduce scale.
		if (currentWeapon.adaptive_point_scale && (record->player->get_max_desync_delta() < 33.f || record->shot || record->robot))
			scale *= .5f;

		if (bbox->radius <= 0.f)
		{
			matrix3x4_t matrix;
			auto rotation_matrix = math::angle_matrix(bbox->rotation);

			math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);

			Vector origin = matrix.GetOrigin();

			Vector center = (bbox->bbmin + bbox->bbmax) * 0.875f;

			if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
			{
				float d1 = (bbox->bbmin.z - center.z) * 0.875f;

				if (hitbox == HITBOX_LEFT_FOOT)
					d1 *= -1.f;

				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + d1), hitbox, false));

				if (currentWeapon.multipoints_hitboxes.at(8))
				{
					float d2 = (bbox->bbmin.x - center.x) * scale;
					float d3 = (bbox->bbmax.x - center.x) * scale;

					points.emplace_back(scan_point(Vector(center.x + d2, center.y, center.z), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x + d3, center.y, center.z), hitbox, false));
				}
			}
		}
		else
		{
			float r = bbox->radius * scale;

			Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;

			if (hitbox == HITBOX_HEAD)
			{
				if (currentWeapon.multipoints_hitboxes.at(0))
				{
					constexpr float rotation = 0.70710678f;

					points.emplace_back(scan_point(Vector(bbox->bbmax.x + (rotation * r), bbox->bbmax.y + (-rotation * r), bbox->bbmax.z), hitbox, false));

					Vector right{ bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + r };

					points.emplace_back(scan_point(right, hitbox, false));

					Vector left{ bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - r };

					points.emplace_back(scan_point(left, hitbox, false));

					points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - r, bbox->bbmax.z), hitbox, false));

					auto state = record->player->get_animation_state();

					if (state && record->velocity.Length() <= 0.1f && record->angles.x <= state->m_flEyePitch)
					{
						points.emplace_back(scan_point(Vector(bbox->bbmax.x - r, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
					}
				}
			}
			else if (hitbox == HITBOX_STOMACH)
			{
				if (currentWeapon.multipoints_hitboxes.at(4))
				{
					// center.
					points.emplace_back(scan_point(center, hitbox, true));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z - r), hitbox, false));
					// back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_PELVIS)
			{
				if (currentWeapon.multipoints_hitboxes.at(5))
				{
					points.emplace_back(scan_point(center, hitbox, true));
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_UPPER_CHEST)
			{
				if (currentWeapon.multipoints_hitboxes.at(1))
				{
					points.emplace_back(scan_point(center - Vector(0.0f, 0.0f, 2.0f), hitbox, true));
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_LOWER_CHEST)
			{
				if (currentWeapon.multipoints_hitboxes.at(3))
				{
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
					// add extra point on back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_CHEST)
			{
				if (currentWeapon.multipoints_hitboxes.at(2))
				{
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
					// add extra point on back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF)
			{
				if (currentWeapon.multipoints_hitboxes.at(8))
				{
					// add center.
					points.emplace_back(scan_point(center, hitbox, true));

					// half bottom.
					points.emplace_back(scan_point(Vector(bbox->bbmax.x - (bbox->radius / 2.f), bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH)
			{
				if (currentWeapon.multipoints_hitboxes.at(7))
					points.emplace_back(scan_point(center, hitbox, true));
			}
			else if (hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_UPPER_ARM)
			{
				if (currentWeapon.multipoints_hitboxes.at(6))
					points.emplace_back(scan_point(Vector(bbox->bbmax.x + bbox->radius, center.y, center.z), hitbox, false));
			}
		}
	}
	else if (currentWeapon.scan_point_mode == 1)
	{
		auto center = (bbox->bbmin + bbox->bbmax) * 0.5f;
		if (bbox->radius <= 0.0f)
		{
			auto rotation_matrix = math::angle_matrix(bbox->rotation);

			matrix3x4_t matrix;
			math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);

			auto origin = matrix.GetOrigin();

			if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
			{
				auto side = (bbox->bbmin.z - center.z) * 0.875f;

				if (hitbox == HITBOX_LEFT_FOOT)
					side = -side;

				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), hitbox, true));

				auto min = (bbox->bbmin.x - center.x) * 0.875f;
				auto max = (bbox->bbmax.x - center.x) * 0.875f;

				points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), hitbox, false));
				points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), hitbox, false));
			}
		}
		else
		{
			auto scale = (hitbox == HITBOX_HEAD) ? GetHeadScale(record->player) : GetBodyScale(record->player);

			if (currentWeapon.adaptive_point_scale && (record->player->get_max_desync_delta() < 33.f || record->shot || record->robot))
				scale *= .5f;

			if (scale <= 0.0f)
			{
				math::vector_transform(center, record->matrixes_data.main[bbox->bone], center);
				points.emplace_back(scan_point(center, hitbox, true));

				return points;
			}

			auto final_radius = bbox->radius * scale;

			if (hitbox == HITBOX_HEAD)
			{
				auto pitch_down = math::normalize_pitch(record->angles.x) > 85.0f;
				auto backward = fabs(math::normalize_yaw(record->angles.y - math::calculate_angle(record->player->get_eye_position(), localPlayer->GetAbsOrigin()).y)) > 120.0f;

				points.emplace_back(scan_point(center, hitbox, !pitch_down || !backward));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x + 0.70710678f * final_radius, bbox->bbmax.y - 0.70710678f * final_radius, bbox->bbmax.z), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - final_radius, bbox->bbmax.z), hitbox, false));

				if (pitch_down && backward)
					points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
			}
			else if (hitbox >= HITBOX_PELVIS && hitbox <= HITBOX_UPPER_CHEST)
			{
				points.emplace_back(scan_point(center, hitbox, true));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

				points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - final_radius, center.z), hitbox, true));
			}
			else if (hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF)
			{
				points.emplace_back(scan_point(center, hitbox, true));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
			}
			else if (hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH)
				points.emplace_back(scan_point(center, hitbox, true));
			else if (hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_UPPER_ARM)
			{
				points.emplace_back(scan_point(center, hitbox, true));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x + final_radius, center.y, center.z), hitbox, false));
			}
		}
	}
	else
	{
		float head_scale = GetHeadScale(record->player);
		float body_scale = GetBodyScale(record->player);
		// reduce scale.
		if (currentWeapon.adaptive_point_scale && (record->player->get_max_desync_delta() < 33.f || record->shot || record->robot))
		{
			head_scale *= .5f;
			body_scale *= .5f;
		}
		if (bbox->radius <= 0.f)
		{
			matrix3x4_t matrix;
			auto rotation_matrix = math::angle_matrix(bbox->rotation);
			math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);
			Vector origin = matrix.GetOrigin();
			Vector center = (bbox->bbmin + bbox->bbmax) * 0.875f;
			if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
			{
				float d1 = (bbox->bbmin.z - center.z) * 0.875f;
				if (hitbox == HITBOX_LEFT_FOOT)
					d1 *= -1.f;
				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + d1), hitbox, false));
				if (body_scale)
				{
					float d2 = (bbox->bbmin.x - center.x) * body_scale;
					float d3 = (bbox->bbmax.x - center.x) * body_scale;
					points.emplace_back(scan_point(Vector(center.x + d2, center.y, center.z), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x + d3, center.y, center.z), hitbox, false));
				}
			}
		}
		else
		{
			float r_head = bbox->radius * head_scale;
			float r = bbox->radius * body_scale;
			Vector center = (bbox->bbmin + bbox->bbmax) / 2.f;
			if (hitbox == HITBOX_HEAD)
			{
				if (head_scale)
				{
					constexpr float rotation = 0.70710678f;
					points.emplace_back(scan_point(Vector(bbox->bbmax.x + (rotation * r_head), bbox->bbmax.y + (-rotation * r_head), bbox->bbmax.z), hitbox, false));
					Vector right(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + r_head);
					points.emplace_back(scan_point(right, hitbox, false));
					Vector left(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - r_head);
					points.emplace_back(scan_point(left, hitbox, false));
					points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - r_head, bbox->bbmax.z), hitbox, false));
					auto state = record->player->get_animation_state();
					if (state && record->velocity.Length() <= 0.1f && record->angles.x <= state->m_flEyePitch)
						points.emplace_back(scan_point(Vector(bbox->bbmax.x - r_head, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_STOMACH)
			{
				if (body_scale)
				{
					// center.
					points.emplace_back(scan_point(center, hitbox, true));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z - r), hitbox, false));
					// back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_PELVIS)
			{
				if (body_scale)
				{
					points.emplace_back(scan_point(center, hitbox, true));
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_UPPER_CHEST)
			{
				if (body_scale)
				{
					points.emplace_back(scan_point(center - Vector(0.0f, 0.0f, 2.0f), hitbox, true));
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_LOWER_CHEST)
			{
				if (body_scale)
				{
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
					// add extra point on back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_CHEST)
			{
				if (body_scale)
				{
					// left & right points
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmax.z + r), hitbox, false));
					points.emplace_back(scan_point(Vector(center.x, center.y, bbox->bbmin.z - r), hitbox, false));
					// add extra point on back.
					points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - r, center.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF)
			{
				if (body_scale)
				{
					// add center.
					points.emplace_back(scan_point(center, hitbox, true));
					// half bottom.
					points.emplace_back(scan_point(Vector(bbox->bbmax.x - (bbox->radius / 2.f), bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
				}
			}
			else if (hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH)
			{
				if (body_scale)
					points.emplace_back(scan_point(center, hitbox, true));
			}
			else if (hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_UPPER_ARM)
			{
				if (body_scale)
					points.emplace_back(scan_point(Vector(bbox->bbmax.x + bbox->radius, center.y, center.z), hitbox, false));
			}
		}
	}
	/*
			auto center = (bbox->bbmin + bbox->bbmax) * 0.5f;

		if (bbox->radius <= 0.0f)
		{
			auto rotation_matrix = math::angle_matrix(bbox->rotation);

			matrix3x4_t matrix;
			math::concat_transforms(record->matrixes_data.main[bbox->bone], rotation_matrix, matrix);

			auto origin = matrix.GetOrigin();

			if (hitbox == HITBOX_RIGHT_FOOT || hitbox == HITBOX_LEFT_FOOT)
			{
				auto side = (bbox->bbmin.z - center.z) * 0.875f;

				if (hitbox == HITBOX_LEFT_FOOT)
					side = -side;

				points.emplace_back(scan_point(Vector(center.x, center.y, center.z + side), hitbox, true));

				auto min = (bbox->bbmin.x - center.x) * 0.875f;
				auto max = (bbox->bbmax.x - center.x) * 0.875f;

				points.emplace_back(scan_point(Vector(center.x + min, center.y, center.z), hitbox, false));
				points.emplace_back(scan_point(Vector(center.x + max, center.y, center.z), hitbox, false));
			}
		}
		else
		{
			auto scale = 0.0f;

			if (currentWeapon.scan_point_mode == 1)
			{
				scale = (hitbox == HITBOX_HEAD) ? GetHeadScale(record->player) : GetBodyScale(record->player);

				if (currentWeapon.adaptive_point_scale && (record->player->get_max_desync_delta() < 33.f || record->shot || record->robot))
					scale *= .5f;
			}
			else
			{
				auto transformed_center = center;
				math::vector_transform(transformed_center, record->matrixes_data.main[bbox->bone], transformed_center);

				auto spread = cheat.globals.spread + cheat.globals.inaccuracy;
				auto distance = transformed_center.DistTo(cheat.globals.eye_pos);

				distance /= math::fast_sin(DEG2RAD(90.0f - RAD2DEG(spread)));
				spread = math::fast_sin(spread);

				auto radius = max(bbox->radius - distance * spread, 0.0f);
				scale = math::clamp(radius / bbox->radius, 0.0f, 1.0f);
			}

			if (scale <= 0.0f)
			{
				math::vector_transform(center, record->matrixes_data.main[bbox->bone], center);
				points.emplace_back(scan_point(center, hitbox, true));

				return points;
			}

			auto final_radius = bbox->radius * scale;

			if (hitbox == HITBOX_HEAD)
			{
				auto pitch_down = math::normalize_pitch(record->angles.x) > 85.0f;
				auto backward = fabs(math::normalize_yaw(record->angles.y - math::calculate_angle(record->eye_position, localPlayer->GetAbsOrigin()).y)) > 120.0f;

				points.emplace_back(scan_point(center, hitbox, !pitch_down || !backward));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x + 0.70710678f * final_radius, bbox->bbmax.y - 0.70710678f * final_radius, bbox->bbmax.z), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y - final_radius, bbox->bbmax.z), hitbox, false));

				if (pitch_down && backward)
					points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
			}
			else if (hitbox >= HITBOX_PELVIS && hitbox <= HITBOX_UPPER_CHEST)
			{
				points.emplace_back(scan_point(center, hitbox, true));

				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z + final_radius), hitbox, false));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x, bbox->bbmax.y, bbox->bbmax.z - final_radius), hitbox, false));

				points.emplace_back(scan_point(Vector(center.x, bbox->bbmax.y - final_radius, center.z), hitbox, true));
			}
			else if (hitbox == HITBOX_RIGHT_CALF || hitbox == HITBOX_LEFT_CALF)
			{
				points.emplace_back(scan_point(center, hitbox, true));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x - final_radius, bbox->bbmax.y, bbox->bbmax.z), hitbox, false));
			}
			else if (hitbox == HITBOX_RIGHT_THIGH || hitbox == HITBOX_LEFT_THIGH)
				points.emplace_back(scan_point(center, hitbox, true));
			else if (hitbox == HITBOX_RIGHT_UPPER_ARM || hitbox == HITBOX_LEFT_UPPER_ARM)
			{
				points.emplace_back(scan_point(center, hitbox, true));
				points.emplace_back(scan_point(Vector(bbox->bbmax.x + final_radius, center.y, center.z), hitbox, false));
			}
		}
		*/

	for (auto& point : points)
		math::vector_transform(point.point, record->matrixes_data.main[bbox->bone], point.point);

	return points;
}

static bool compare_targets(const scanned_target& first, const scanned_target& second)
{
	if (configs.player_list.high_priority[first.record->index] != configs.player_list.high_priority[second.record->index])
		return configs.player_list.high_priority[first.record->index];

	switch (ragebot::get().currentWeapon.selection_type)
	{
	case 1:
		return first.fov < second.fov;
	case 2:
		return first.distance < second.distance;
	case 3:
		return first.health < second.health;
	case 4:
		return first.data.damage > second.data.damage;
	}

	return false;
}

void ragebot::find_best_target()
{
	if (currentWeapon.selection_type)
		std::sort(scanned_targets.begin(), scanned_targets.end(), compare_targets);

	for (auto& target : scanned_targets)
	{
		if (target.fov > configs.ragebot.field_of_view)
			continue;

		final_target = target;
		final_target.record->adjust_player();
		break;
	}
}

void ragebot::fire(CUserCmd* cmd)
{
	if (!cheat.globals.weapon->can_fire(true))
		return;

	auto aim_angle = math::calculate_angle(cheat.globals.eye_pos, final_target.data.point.point).Clamp();

	if (!configs.ragebot.silent_aim)
		m_engine()->SetViewAngles(aim_angle);

	if (!configs.ragebot.autoshoot && !(cmd->m_buttons & IN_ATTACK))
		return;

	auto final_hitchance = 0;
	auto hitchance_amount = 0;

	if (cheat.globals.double_tap_aim && currentWeapon.double_tap_hitchance)
		hitchance_amount = currentWeapon.double_tap_hitchance_amount;
	else //!cheat.globals.double_tap_aim && 
		hitchance_amount = currentWeapon.hitchance;

	if (hitchance_amount)
	{
		final_hitchance = (cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08 && !(localPlayer->m_fFlags() & FL_ONGROUND) && (!(engineprediction::get().backup_data.flags & FL_ONGROUND) && fabs(engineprediction::get().backup_data.velocity.z) < 5.0f && engineprediction::get().backup_data.velocity.Length2D() < 5.0f)) ? 101 : hitchance(aim_angle);

		if (final_hitchance < hitchance_amount)
		{
			bool hit = (cheat.globals.weapon->IsZoomable(false) && (cheat.globals.inaccuracy < 0.009f || hitchance(aim_angle) >= hitchance_amount));

			bool can_scope = cheat.globals.weapon && cheat.globals.weapon->m_zoomLevel() == 0 && cheat.globals.weapon->IsZoomable(true);

			if (can_scope && configs.ragebot.autoscope != 0 && (configs.ragebot.autoscope == 1 || (configs.ragebot.autoscope == 2 && !hit)))
				cmd->m_buttons |= IN_ZOOM;

			//cmd->m_buttons |= IN_ATTACK2;

			return;
		}
	}

	auto backtrack_ticks = 0;
	auto net_channel_info = m_engine()->GetNetChannelInfo();

	if (net_channel_info)
	{
		auto original_tickbase = cheat.globals.backup_tickbase;

		if (misc::get().double_tap_enabled && configs.ragebot.double_tap_key.key > KEY_NONE && configs.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key && !localPlayer->m_bGunGameImmunity() && !(localPlayer->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && !cheat.globals.weapon->is_grenade() && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_TASER && cheat.globals.weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER)
			original_tickbase += cheat.globals.weapon->get_max_tickbase_shift();

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto correct = math::clamp(net_channel_info->GetLatency(FLOW_OUTGOING) + net_channel_info->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());
		auto delta_time = correct - (TICKS_TO_TIME(original_tickbase) - final_target.record->simulation_time);

		backtrack_ticks = TIME_TO_TICKS(fabsf(delta_time));
	}

	static auto get_hitbox_name = [](int hitbox, bool shot_info = false) -> std::string
		{
			switch (hitbox)
			{
			case HITBOX_HEAD:
				return shot_info ? crypt_str("head") : crypt_str("Head");
			case HITBOX_LOWER_CHEST:
				return shot_info ? crypt_str("lower chest") : crypt_str("Lower chest");
			case HITBOX_CHEST:
				return shot_info ? crypt_str("chest") : crypt_str("Chest");
			case HITBOX_UPPER_CHEST:
				return shot_info ? crypt_str("upper chest") : crypt_str("Upper chest");
			case HITBOX_STOMACH:
				return shot_info ? crypt_str("stomach") : crypt_str("Stomach");
			case HITBOX_PELVIS:
				return shot_info ? crypt_str("pelvis") : crypt_str("Pelvis");
			case HITBOX_RIGHT_UPPER_ARM:
			case HITBOX_RIGHT_LOWER_ARM:
			case HITBOX_RIGHT_HAND:
				return shot_info ? crypt_str("left arm") : crypt_str("Left arm");
			case HITBOX_LEFT_UPPER_ARM:
			case HITBOX_LEFT_LOWER_ARM:
			case HITBOX_LEFT_HAND:
				return shot_info ? crypt_str("right arm") : crypt_str("Right arm");
			case HITBOX_RIGHT_THIGH:
			case HITBOX_RIGHT_CALF:
				return shot_info ? crypt_str("left leg") : crypt_str("Left leg");
			case HITBOX_LEFT_THIGH:
			case HITBOX_LEFT_CALF:
				return shot_info ? crypt_str("right leg") : crypt_str("Right leg");
			case HITBOX_RIGHT_FOOT:
				return shot_info ? crypt_str("left foot") : crypt_str("Left foot");
			case HITBOX_LEFT_FOOT:
				return shot_info ? crypt_str("right foot") : crypt_str("Right foot");
			}
		};

	static auto get_resolver_type = [](ResolverType type) -> std::string
		{
			switch (type)
			{
			case ResolverType::ORIGINAL:
				return crypt_str("Orig ");
			case ResolverType::BRUTEFORCE:
				return crypt_str("BF ");
			case ResolverType::LBY:
				return crypt_str("LBY ");
			case ResolverType::LAYERS:
				return crypt_str("Layer ");
			case ResolverType::TRACE:
				return crypt_str("Trace ");
			case ResolverType::DIRECTIONAL:
				return crypt_str("Dir ");
			}
		};

	static auto get_side_type = [](ResolverSide type) -> std::string
		{
			switch (type)
			{
			case ResolverSide::RESOLVER_ORIGINAL:
				return crypt_str("Orig ");
			case ResolverSide::RESOLVER_ZERO:
				return crypt_str("Zero ");
			case ResolverSide::RESOLVER_FIRST:
				return crypt_str("R1 ");
			case ResolverSide::RESOLVER_SECOND:
				return crypt_str("R2 ");
			case ResolverSide::RESOLVER_LOW_FIRST:
				return crypt_str("RL1 ");
			case ResolverSide::RESOLVER_LOW_SECOND:
				return crypt_str("RL2 ");
			case ResolverSide::RESOLVER_DEFAULT:
				return crypt_str("Def ");
			default:
				return crypt_str("Unknown ");
			}
		};

	PlayerInfo player_info;
	m_engine()->GetPlayerInfo(final_target.record->index, &player_info);

	if (configs.player_list.roll_prediction[final_target.record->index] && !player_info.fakeplayer)
	{
		auto& vec = final_target.record->player->m_vecOrigin();

		auto head_vec = final_target.record->player->hitbox_position_matrix(0, final_target.record->matrixes_data.main);

		auto pos = Vector(vec.x, vec.y, head_vec.z);

		auto new_aim_angle = math::calculate_angle(cheat.globals.eye_pos, pos).Clamp();

		float yaw = new_aim_angle.y - localPlayer->m_aimPunchAngle().y * 2;

		float pitch = new_aim_angle.x - localPlayer->m_aimPunchAngle().x * 2;

		aim_angle = Vector(pitch, yaw, aim_angle.z);
	}

	if (backtrack::get().valid(final_target.record->simulation_time))
		cmd->m_tickcount = TIME_TO_TICKS(final_target.record->simulation_time + util::get_interpolation());
	cmd->m_viewangles = aim_angle;
	cmd->m_buttons |= IN_ATTACK;

	last_target_index = final_target.record->index;
	last_shoot_position = cheat.globals.eye_pos;
	last_target[last_target_index] = LastTarget{ *final_target.record, final_target.data, final_target.distance };

	std::stringstream log;

	log << crypt_str("Fired shot at ") + player_info.getUsername() + crypt_str(" | ");
	log << crypt_str("hitbox : ") + get_hitbox_name(final_target.data.hitbox) + crypt_str(" | ");
	log << crypt_str("hitchance : ") + (final_hitchance > 100 ? crypt_str("Prediction") : std::to_string(final_hitchance)) + crypt_str(" | ");
	log << crypt_str("damage : ") + std::to_string(final_target.data.damage) + crypt_str(" | ");
	log << crypt_str("safety : ") + std::string(final_target.data.point.safe ? "true" : "false") + crypt_str(" | ");
	if (!player_info.fakeplayer)
		log << crypt_str("yaw : ") + std::to_string(std::clamp((int)final_target.record->player->get_animation_state()->m_flGoalFeetYaw, -360, 360)) + crypt_str(" | "); // get_side_type(final_target.record->side)
	log << crypt_str("lag_comp : ") + std::to_string(backtrack_ticks);

	if (configs.misc.events_to_log[EVENTLOG_HIT])
		eventlogs::get().add("Fired", log.str());

	auto shot = &cheat.shots.emplace_back();

	shot->last_target = last_target_index;
	shot->side = final_target.record->side;
	shot->fire_tick = m_globals()->m_tickcount;
	shot->shot_info.target_name = player_info.getUsername();
	shot->shot_info.client_hitbox = get_hitbox_name(final_target.data.hitbox, true);
	shot->shot_info.client_damage = final_target.data.damage;
	shot->shot_info.hitchance = final_hitchance;
	shot->shot_info.backtrack_ticks = backtrack_ticks;
	shot->shot_info.aim_point = final_target.data.point.point;

	cheat.globals.aimbot_working = true;
	cheat.globals.revolver_working = false;
	cheat.globals.last_aimbot_shot = m_globals()->m_tickcount;
}

/*
int ragebot::hitchance(const Vector& aim_angle)
{
	if (precomputed_seeds.empty())
		build_seed_table();

	static auto nospread = m_cvar()->FindVar(crypt_str("weapon_accuracy_nospread"));

	if (nospread->getBool())
		return 101;

	//auto final_hitchance = 0;
	auto total_hits = 0;
	//const auto needed_hits = static_cast<int>(static_cast<float>(MAX_SPREAD_SEED) * (static_cast<float>(hitchance) / 100.f));

	auto forward = ZERO;
	auto right = ZERO;
	auto up = ZERO;
	auto direction = ZERO;
	auto spread = ZERO;

	//static auto weapon_recoil_scale = m_cvar()->FindVar(crypt_str("weapon_recoil_scale"));
	//const auto current_aim_angle = math::calculate_angle(cheat.globals.eye_pos, final_target.data.point.point) - (localPlayer->m_aimPunchAngle() * weapon_recoil_scale->getFloat());

	math::angle_vectors(aim_angle, &forward, &right, &up);

	math::fast_vec_normalize(forward);
	math::fast_vec_normalize(right);
	math::fast_vec_normalize(up);

	const auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (!weapon_info || (cheat.globals.eye_pos - final_target.data.point.point).Length() > weapon_info->flRange)
		return 0;

	auto weapon_inaccuracy = cheat.globals.weapon->get_inaccuracy();
	auto weapon_spread = cheat.globals.weapon->get_spread();

	//if (cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08 && !(localPlayer->m_fFlags() & FL_ONGROUND) && weapon_inaccuracy < 0.009f)
	//	return 101;

	for (int i = 0; i < MAX_SPREAD_SEED; i++) {
		spread = calculate_spread(i, weapon_inaccuracy, weapon_spread);

		//math::random_seed(i + 1);
		//const float spreadX = math::random_float(0.f, 2.f * static_cast<float>(M_PI));
		//const float spreadY = math::random_float(0.f, 2.f * static_cast<float>(M_PI));
		//auto inaccuracy = weapon_inaccuracy * math::random_float(0.f, 1.f);

		//Vector2D spreadView{ (cosf(spread.x) * inaccuracy) + (cosf(spread.y) * inaccuracy),
		//				   (sinf(spread.x) * inaccuracy) + (sinf(spread.y) * inaccuracy) };

		direction = cheat.globals.eye_pos + ((forward + (right * spread.x) + (up * spread.y)) * weapon_info->flRange);


		//for (int hitbox = 0; hitbox < Hitboxes::HITBOX_MAX; hitbox++)
		//	if (hitboxIntersection(final_target.record->player, final_target.record->matrixes_data.main, hitbox, cheat.globals.eye_pos, direction)) //if (hitbox_intersection(final_target.record->player, final_target.record->matrixes_data.main, hitbox, cheat.globals.eye_pos, direction))
		//	{
		//		total_hits++;
		//		break;
		//	}

		if (can_hit(final_target.record->player, final_target.record->matrixes_data.main, final_target.data.hitbox, cheat.globals.eye_pos, direction))
			++total_hits;

		//if (total_hits >= needed_hits)
		//	final_hitchance = static_cast<int>(static_cast<float>(total_hits) / (MAX_SPREAD_SEED * 0.01f));

		//if ((MAX_SPREAD_SEED - i + total_hits) < needed_hits)
		//	break;
	}

	//if (static_cast<int>((static_cast<float>(total_hits) / MAX_SPREAD_SEED) * 100.f) >= hitchance)
	//	return final_hitchance;

	return static_cast<int>(std::ceil(static_cast<float>(total_hits) / (MAX_SPREAD_SEED * 0.01f)));
}
*/

int ragebot::hitchance(const Vector& aim_angle)
{
	static auto nospread = m_cvar()->FindVar(crypt_str("weapon_accuracy_nospread"));

	if (nospread->GetBool())
		return 101;

	auto final_hitchance = 0;
	auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (!weapon_info)
		return final_hitchance;

	if ((cheat.globals.eye_pos - final_target.data.point.point).Length() > weapon_info->flRange)
		return final_hitchance;

	auto forward = ZERO;
	auto right = ZERO;
	auto up = ZERO;

	math::angle_vectors(aim_angle, &forward, &right, &up);

	math::fast_vec_normalize(forward);
	math::fast_vec_normalize(right);
	math::fast_vec_normalize(up);

	/*
	auto is_special_weapon = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP || cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_G3SG1 || cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SCAR20 || cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08;

	auto inaccuracy = weapon_info->flInaccuracyStand;

	if (localPlayer->m_fFlags() & FL_DUCKING)
	{
		if (is_special_weapon)
			inaccuracy = weapon_info->flInaccuracyCrouchAlt;
		else
			inaccuracy = weapon_info->flInaccuracyCrouch;
	}
	else if (is_special_weapon)
		inaccuracy = weapon_info->flInaccuracyStandAlt;

	if (cheat.globals.inaccuracy - 0.000001f < inaccuracy)
		final_hitchance = 101;
	else
	{
	*/
	static auto setup_spread_values = true;
	static float spread_values[256][6];

	if (setup_spread_values)
	{
		setup_spread_values = false;

		for (auto i = 0; i < MAX_SPREAD_SEED; ++i)
		{
			math::random_seed(i + 1);

			auto a = math::random_float(0.0f, 1.0f);
			auto b = math::random_float(0.0f, DirectX::XM_2PI);
			auto c = math::random_float(0.0f, 1.0f);
			auto d = math::random_float(0.0f, DirectX::XM_2PI);

			spread_values[i][0] = a;
			spread_values[i][1] = c;

			auto sin_b = 0.0f, cos_b = 0.0f;
			DirectX::XMScalarSinCos(&sin_b, &cos_b, b);

			auto sin_d = 0.0f, cos_d = 0.0f;
			DirectX::XMScalarSinCos(&sin_d, &cos_d, d);

			spread_values[i][2] = sin_b;
			spread_values[i][3] = cos_b;
			spread_values[i][4] = sin_d;
			spread_values[i][5] = cos_d;
		}
	}

	auto hits = 0;

	for (auto i = 0; i < MAX_SPREAD_SEED; ++i)
	{
		auto inaccuracy = spread_values[i][0] * cheat.globals.inaccuracy;
		auto spread = spread_values[i][1] * cheat.globals.spread;

		auto spread_x = spread_values[i][3] * inaccuracy + spread_values[i][5] * spread;
		auto spread_y = spread_values[i][2] * inaccuracy + spread_values[i][4] * spread;

		auto direction = ZERO;

		direction.x = forward.x + right.x * spread_x + up.x * spread_y;
		direction.y = forward.y + right.y * spread_x + up.y * spread_y;
		direction.z = forward.z + right.z * spread_x + up.z * spread_y;

		auto end = cheat.globals.eye_pos + direction * weapon_info->flRange;

		if (hitbox_intersection(final_target.record->player, final_target.record->matrixes_data.main, final_target.data.hitbox, cheat.globals.eye_pos, end))
			++hits;
	}

	final_hitchance = static_cast<int>(static_cast<float>(hits) / (MAX_SPREAD_SEED * 0.01f));
	//}

	if (cheat.globals.double_tap_aim)
		return final_hitchance;

	auto damage = 0;
	auto high_accuracy_weapon = cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_AWP || cheat.globals.weapon->m_iItemDefinitionIndex() == WEAPON_SSG08;

	auto spread = cheat.globals.spread + cheat.globals.inaccuracy;

	for (auto i = 1; i <= 6; ++i)
	{
		for (auto j = 0; j < 8; ++j)
		{
			auto current_spread = spread * (static_cast<float>(i) / 6.0f);

			auto direction_cos = 0.0f;
			auto direction_sin = 0.0f;

			DirectX::XMScalarSinCos(&direction_cos, &direction_sin, static_cast<float>(j) / 8.0f * DirectX::XM_2PI);

			auto spread_x = direction_cos * current_spread;
			auto spread_y = direction_sin * current_spread;

			auto direction = ZERO;

			direction.x = forward.x + spread_x * right.x + spread_y * up.x;
			direction.y = forward.y + spread_x * right.y + spread_y * up.y;
			direction.z = forward.z + spread_x * right.z + spread_y * up.z;

			auto end = cheat.globals.eye_pos + direction * weapon_info->flRange;

			if (hitbox_intersection(final_target.record->player, final_target.record->matrixes_data.main, final_target.data.hitbox, cheat.globals.eye_pos, end))
			{
				auto fire_data = autowall::get().wall_penetration(cheat.globals.eye_pos, end, final_target.record->player);
				auto valid_hitbox = true;

				if (final_target.data.hitbox == HITBOX_HEAD && fire_data.hitbox != HITBOX_HEAD)
					valid_hitbox = false;

				if (fire_data.valid && fire_data.damage >= 1 && valid_hitbox)
					damage += high_accuracy_weapon ? fire_data.damage : 1;
			}
		}
	}

	if (high_accuracy_weapon)
		return static_cast<float>(damage) / 48.0f >= get_minimum_damage(final_target.data.visible, final_target.health) ? final_hitchance : 0;

	return static_cast<float>(damage) / 48.0f >= static_cast<float>(configs.ragebot.weapon[cheat.globals.current_weapon].hitchance) * 0.01f ? final_hitchance : 0;
}

static int clip_ray_to_hitbox(const Ray_t& ray, mstudiobbox_t* hitbox, matrix3x4_t& matrix, CGameTrace& trace)
{
	trace.fraction = 1.0f;
	trace.startsolid = false;

	return memory->clipRayToHixbox(ray, hitbox, matrix, trace);
}

bool ragebot::hitboxIntersection(player_t* entity, matrix3x4_t* matrix, int iHitbox, const Vector& start, const Vector& end)
{
	auto model = entity->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->getHitboxSet(entity->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto hitbox = studio_set->getHitbox(iHitbox);
	if (!hitbox)
		return false;

	Vector mins, maxs;
	if (hitbox->radius != -1.f)
	{
		math::vector_transform(hitbox->bbmin, matrix[hitbox->bone], mins);
		math::vector_transform(hitbox->bbmax, matrix[hitbox->bone], maxs);
		const auto dist = math::segment_to_segment(start, end, mins, maxs);

		if (dist < hitbox->radius)
			return true;
	}
	else
	{
		math::vector_transform(math::vector_rotate(hitbox->bbmin, hitbox->rotation), matrix[hitbox->bone], mins);
		math::vector_transform(math::vector_rotate(hitbox->bbmax, hitbox->rotation), matrix[hitbox->bone], maxs);

		math::vector_i_transform(start, matrix[hitbox->bone], mins);
		math::vector_i_rotate(end, matrix[hitbox->bone], maxs);

		if (math::intersect_line_with_bb(mins, maxs, hitbox->bbmin, hitbox->bbmax))
			return true;
	}

	return false;
}

bool CompareHitboxesWithHitgroup(int hitbox, int hitgroup)
{
	switch (hitgroup)
	{
	case HITGROUP_HEAD: return hitbox == HITBOX_HEAD; break;
	case HITGROUP_CHEST: return hitbox == HITBOX_CHEST
		|| hitbox == HITBOX_UPPER_CHEST
		|| hitbox == HITBOX_LOWER_CHEST;
		break;
	case HITGROUP_STOMACH: return hitbox == HITBOX_STOMACH || hitbox == HITBOX_PELVIS; break;
	case HITGROUP_LEFTARM: return hitbox == HITBOX_LEFT_LOWER_ARM
		|| hitbox == HITBOX_LEFT_UPPER_ARM
		|| hitbox == HITBOX_LEFT_HAND; break;
	case HITGROUP_RIGHTARM: return hitbox == HITBOX_RIGHT_LOWER_ARM
		|| hitbox == HITBOX_RIGHT_UPPER_ARM
		|| hitbox == HITBOX_RIGHT_HAND; break;
	case HITGROUP_LEFTLEG: return hitbox == HITBOX_LEFT_FOOT
		|| hitbox == HITBOX_LEFT_THIGH
		|| hitbox == HITBOX_LEFT_CALF; break;
	case HITGROUP_RIGHTLEG: return hitbox == HITBOX_RIGHT_FOOT
		|| hitbox == HITBOX_RIGHT_THIGH
		|| hitbox == HITBOX_RIGHT_CALF; break;
	}
	return false;
}

bool ragebot::can_hit(player_t* player, matrix3x4_t* bones, int box, const Vector& start, const Vector& end)
{
	auto model = player->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->getHitboxSet(player->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto hitbox = studio_set->getHitbox(box);
	if (!hitbox)
		return false;

	Vector min, max;

	if (hitbox->radius != -1.f)
	{
		math::vector_transform(hitbox->bbmin, bones[hitbox->bone], min);
		math::vector_transform(hitbox->bbmax, bones[hitbox->bone], max);

		const auto dist = math::segment_to_segment(start, end, min, max);

		if (dist < hitbox->radius)
		{
			if (box == HITBOX_HEAD)
			{
				CGameTrace tr;
				Ray_t ray;
				ray.Init(start, end);

				m_trace()->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, player, &tr);

				return tr.entity == player && tr.hitgroup == HITGROUP_HEAD;
			}
			return true;
		}
	}
	else
	{
		CGameTrace tr;
		Ray_t ray;
		ray.Init(start, end);

		m_trace()->ClipRayToEntity(ray, MASK_SHOT | CONTENTS_GRATE, player, &tr);

		//if (tr.entity == player && ((tr.hitgroup >= HITGROUP_HEAD && tr.hitgroup <= HITGROUP_RIGHTLEG) || tr.hitgroup == HITGROUP_GEAR))
		if (tr.entity == player && CompareHitboxesWithHitgroup(box, tr.hitgroup))
			return true;
	}

	return false;
}

bool ragebot::hitbox_intersection(player_t* e, matrix3x4_t* matrix, int hbox, const Vector& start, const Vector& end, float* safe)
{
	auto model = e->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto studio_set = studio_model->getHitboxSet(e->m_nHitboxSet());

	if (!studio_set)
		return false;

	auto studio_hitbox = studio_set->getHitbox(hbox);

	if (!studio_hitbox)
		return false;

	CGameTrace trace;

	Ray_t ray;
	ray.Init(start, end);

	auto intersected = clip_ray_to_hitbox(ray, studio_hitbox, matrix[studio_hitbox->bone], trace) >= 0;

	if (!safe)
		return intersected;

	Vector min, max;

	math::vector_transform(studio_hitbox->bbmin, matrix[studio_hitbox->bone], min);
	math::vector_transform(studio_hitbox->bbmax, matrix[studio_hitbox->bone], max);

	auto center = (min + max) * 0.5f;
	auto distance = center.DistTo(end);

	if (distance > *safe)
		*safe = distance;

	return intersected;
}
