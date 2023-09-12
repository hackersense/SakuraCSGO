#include "autowall.h"

void autowall::scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage)
{
	if (!e->is_player())
		return;

	auto is_armored = [&]()->bool
	{
		if (e->m_ArmorValue() > 0)
		{
			switch (enterTrace.hitgroup)
			{
			case HITGROUP_HEAD:
				return e->m_bHasHelmet() || e->m_bHasHeavyArmor();
			case HITGROUP_GENERIC:
			case HITGROUP_CHEST:
			case HITGROUP_STOMACH:
			case HITGROUP_LEFTARM:
			case HITGROUP_RIGHTARM:
			case HITGROUP_LEFTLEG:
			case HITGROUP_RIGHTLEG:
			case HITGROUP_GEAR:
				return true;
			default:
				return e->m_bHasHeavyArmor();
			}
		}

		return false;
	};

	static auto mp_damage_scale_ct_head = m_cvar2()->FindVar(crypt_str("mp_damage_scale_ct_head"));
	static auto mp_damage_scale_t_head = m_cvar2()->FindVar(crypt_str("mp_damage_scale_t_head"));

	static auto mp_damage_scale_ct_body = m_cvar2()->FindVar(crypt_str("mp_damage_scale_ct_body"));
	static auto mp_damage_scale_t_body = m_cvar2()->FindVar(crypt_str("mp_damage_scale_t_body"));

	auto head_scale = e->m_iTeamNum() == TeamID::CT ? mp_damage_scale_ct_head->GetFloat() : mp_damage_scale_t_head->GetFloat();
	auto body_scale = e->m_iTeamNum() == TeamID::CT ? mp_damage_scale_ct_body->GetFloat() : mp_damage_scale_t_body->GetFloat();

	auto armor_heavy = e->m_bHasHeavyArmor();
	auto armor_value = e->m_ArmorValue();

	if (armor_heavy)
		head_scale *= 0.5f;

	float flHeadShotMultiplier = 4.0f;
	if (weaponData)
		flHeadShotMultiplier = weaponData->flHeadshotmultyplrier;

	switch (enterTrace.hitgroup)
	{
	case HITGROUP_HEAD:
		currentDamage *= flHeadShotMultiplier * head_scale;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f * body_scale;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f * body_scale;
		break;
	default:
		currentDamage *= body_scale;
		break;
	}

	if (is_armored() && weaponData)
	{
		auto armor_scale = 1.0f;
		auto armor_ratio = weaponData->flArmorRatio * 0.5f;
		auto armor_bonus_ratio = 0.5f;
		auto new_damage = currentDamage * armor_ratio;

		if (armor_heavy)
		{
			armor_ratio *= 0.25f; // 0.2f
			//armor_ratio *= 0.5f;
			armor_bonus_ratio = 0.33f;
			//armor_scale = 0.25f;
			armor_scale = 0.33f;
			new_damage = (currentDamage * armor_ratio) * 0.85f;
		}

		auto estiminated_damage = (currentDamage - (currentDamage * armor_ratio)) * (armor_scale * armor_bonus_ratio);

		if (estiminated_damage > armor_value)
			new_damage = currentDamage - armor_value / armor_bonus_ratio;

		currentDamage = new_damage;
	}

	currentDamage = floorf(currentDamage);
}

bool autowall::is_breakable_entity(IClientEntity* e)
{
	if (!e || !e->EntIndex())
		return false;

	auto client_class = e->GetClientClass();

	if (!client_class)
		return false;

	auto class_id = client_class->m_ClassID;

	//if (configs.ragebot.autowall == 1 || configs.legitbot.enabled)
	//return memory->isBreakableEntityFn(e) || (class_id == ClassID::CBaseDoor || class_id == ClassID::CBreakableSurface || class_id == ClassID::CDynamicProp); // CBreakableProp

	auto take_damage = *reinterpret_cast<uintptr_t*>(uintptr_t(memory->isBreakableEntityFn) + 0x26);
	auto backup = *reinterpret_cast<uint8_t*>(uintptr_t(e) + take_damage);

	if (class_id == ClassID::CBreakableSurface)
		*reinterpret_cast<uint8_t*>(uintptr_t(e) + take_damage) = DAMAGE_YES;
	else if (class_id == ClassID::CBaseDoor || class_id == ClassID::CDynamicProp)
		*reinterpret_cast<uint8_t*>(uintptr_t(e) + take_damage) = DAMAGE_NO;

	auto result = memory->isBreakableEntityFn(e);

	*reinterpret_cast<uint8_t*>(uintptr_t(e) + take_damage) = backup;
	return result;
}

bool autowall::traceToExit(const CGameTrace& enterTrace, const Vector& start, const Vector& direction, Vector& end, CGameTrace& exitTrace)
{
	bool result = false;
	const auto traceToExitFn = memory->traceToExit;
	__asm {
		push 0
		push 0
		push 0
		push exitTrace
		mov eax, direction
		push[eax]Vector.z
		push[eax]Vector.y
		push[eax]Vector.x
		mov eax, start
		push[eax]Vector.z
		push[eax]Vector.y
		push[eax]Vector.x
		mov edx, enterTrace
		mov ecx, end
		call traceToExitFn
		add esp, 40
		mov result, al
	}
	return result;
}

bool autowall::trace_to_exit(Vector startPosition, Vector direction, Vector& end, CGameTrace& enterTrace, CGameTrace& exitTrace, float stepSize, float maxDistance)
{
	float distance{ 0.0f };
	int previousContents{ 0 };

	while (distance <= maxDistance)
	{
		float step = (stepSize == -1.0f ? (m_globals()->m_frametime > m_globals()->m_intervalpertick ? 8.0f : 4.0f) : stepSize);

		distance += step;
		end = startPosition + direction * distance;

		if (!previousContents)
			previousContents = (configs.ragebot.autowall == 1 || configs.legitbot.enabled) ? m_trace()->GetPointContents_WorldOnly(end, MASK_SHOT_HULL | CONTENTS_HITBOX) : m_trace()->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX);

		const int currentContents = (configs.ragebot.autowall == 1 || configs.legitbot.enabled) ? m_trace()->GetPointContents_WorldOnly(end, MASK_SHOT_HULL | CONTENTS_HITBOX) : m_trace()->GetPointContents(end, MASK_SHOT_HULL | CONTENTS_HITBOX);

		if (currentContents & MASK_SHOT_HULL && !(currentContents & CONTENTS_HITBOX && currentContents != previousContents))
			continue;

		auto destination = end - (direction * 4.0f);

		if (m_trace()->TraceRay({ end, destination }, MASK_SHOT, nullptr, &exitTrace); exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
		{
			if (m_trace()->trace_entity(end, startPosition, MASK_SHOT & ~CONTENTS_HITBOX, exitTrace.entity, &exitTrace); exitTrace.DidHit() && !exitTrace.startsolid)
			{
				end = exitTrace.endpos;
				return true;
			}

			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid)
		{
			if (is_breakable_entity(enterTrace.entity) && is_breakable_entity(exitTrace.entity))
			{
				exitTrace.surface.surfaceProps = enterTrace.surface.surfaceProps;
				exitTrace.endpos = startPosition + direction;
				return true;
			}

			continue;
		}

		if (exitTrace.surface.flags & SURF_NODRAW)
		{
			if (is_breakable_entity(enterTrace.entity) && is_breakable_entity(exitTrace.entity))
			{
				end = exitTrace.endpos;
				return true;
			}

			if (!(enterTrace.surface.flags & SURF_NODRAW))
				continue;
		}

		if (exitTrace.plane.normal.Dot(direction) <= 1.0f) // exit nodraw is only valid if our entrace is also nodraw
		{
			end -= direction * (exitTrace.fraction * 4.0f);
			return true;
		}

		/*
		if (exitTrace.DidHit() && !exitTrace.startsolid)
		{
			if (is_breakable_entity(enterTrace.entity) && is_breakable_entity(exitTrace.entity))
			{
				end = exitTrace.endpos;
				return true;
			}

			if (enterTrace.surface.flags & SURF_NODRAW || (!(exitTrace.surface.flags & SURF_NODRAW) && exitTrace.plane.normal.Dot(direction) <= 1.0f))
				return true;

			continue;
		}
		else
		{
			if (enterTrace.entity && enterTrace.get_entity_index() != 0 && is_breakable_entity(enterTrace.entity))
				return true;

			continue;
		}
		*/
	}

	return false;
}

bool autowall::handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	if (weaponData->flPenetration <= 0.0f)
		return false;

	if (possibleHitsRemaining <= 0)
		return false;

	auto contents_grate = enterTrace.contents & CONTENTS_GRATE;
	auto surf_nodraw = enterTrace.surface.flags & SURF_NODRAW;

	auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	auto enter_material = enterSurfaceData->game.material;

	auto is_solid_surf = enterTrace.contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enterTrace.surface.flags >> 7 & SURF_LIGHT;

	Vector end;
	CGameTrace exit_trace;

	// traceToExit(enterTrace, enterTrace.endpos, direction, end, exit_trace)) trace_to_exit(enterTrace.endpos, direction, end, enterTrace, exit_trace, 4.0f, 60.0f)
	if (!trace_to_exit(enterTrace.endpos, direction, end, enterTrace, exit_trace, -1.0f, (configs.ragebot.autowall == 1 || configs.legitbot.enabled) ? 40.0f : 60.0f))
	{
		auto enter = (configs.ragebot.autowall == 1 || configs.legitbot.enabled) ? m_trace()->GetPointContents_WorldOnly(enterTrace.endpos, MASK_SHOT_HULL) : m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL);
		if (!(enter & MASK_SHOT_HULL))
			return false;
	}

	auto enter_penetration_modifier = enterSurfaceData->game.m_flPenetrationModifier;
	auto exit_surface_data = m_physsurface()->GetSurfaceData(exit_trace.surface.surfaceProps);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->game.material;
	auto exit_penetration_modifier = exit_surface_data->game.m_flPenetrationModifier;

	auto combined_damage_modifier = 0.16f;
	auto combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) / 2.0f;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_damage_modifier = 0.05f;
		combined_penetration_modifier = 3.0f;
	}
	else if (contents_grate || surf_nodraw)
		combined_penetration_modifier = 1.0f;
	else if (enter_material == CHAR_TEX_FLESH && reinterpret_cast<player_t*>(enterTrace.entity)->m_iTeamNum() == localPlayer->m_iTeamNum() && !ff_damage_reduction_bullets)
	{
		if (!ff_damage_bullet_penetration)
			return false;

		combined_penetration_modifier = ff_damage_bullet_penetration;
		combined_damage_modifier = 0.16f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3.0f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2.0f;
	}

	auto penetration_modifier = std::fmaxf(0.0f, 1.0f / combined_penetration_modifier);
	auto penetration_distance = (exit_trace.endpos - enterTrace.endpos).Length();

	penetration_distance = penetration_distance * penetration_distance * penetration_modifier * 0.041666668f;

	auto damage_modifier = max(0.0f, 3.0f / weaponData->flPenetration * 1.25f) * penetration_modifier * 3.0f + currentDamage * combined_damage_modifier + penetration_distance;
	auto damage_lost = max(0.0f, damage_modifier);

	if (damage_lost > currentDamage)
		return false;

	currentDamage -= damage_lost;

	if (currentDamage < 1.0f)
		return false;

	eyePosition = exit_trace.endpos;
	--possibleHitsRemaining;

	return true;
}

bool autowall::fire_bullet(weapon_t* weapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e, const Vector& pos)
{
	if (!weapon)
		return false;

	auto weaponData = weapon->get_weapon_info();

	if (!weaponData)
		return false;
	
	CGameTrace enterTrace;

	currentDamage = weaponData->iDamage;

	auto eyePosition = pos;
	auto currentDistance = 0.0f;
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = 3000.0f;
	auto penetrationPower = weaponData->flPenetration;
	auto possibleHitsRemaining = 4;

	while (currentDamage >= 1.0f)
	{
		maxRange -= currentDistance;
		auto end = eyePosition + direction * maxRange;

		m_trace()->trace_entity(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, localPlayer.get(), &enterTrace);
		util::clip_trace_to_players(e, eyePosition, end + direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &enterTrace);

		auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
		auto enterSurfPenetrationModifier = enterSurfaceData->game.m_flPenetrationModifier;
		auto enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.0f)
			break;

		currentDistance += enterTrace.fraction * maxRange;
		currentDamage *= std::pow(weaponData->flRangeModifier, currentDistance / 500.0f);

		if (currentDistance > penetrationDistance && weaponData->flPenetration || enterSurfPenetrationModifier < 0.1f)
			break;

		auto canDoDamage = enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC;
		auto isPlayer = reinterpret_cast<player_t*>(enterTrace.entity)->is_player();
		auto isEnemy = reinterpret_cast<player_t*>(enterTrace.entity)->m_iTeamNum() != localPlayer->m_iTeamNum();

		if (canDoDamage && isPlayer && isEnemy)
		{
			scale_damage(reinterpret_cast<player_t*>(enterTrace.entity), enterTrace, weaponData, currentDamage);
			hitbox = enterTrace.hitbox;
			return true;
		}

		if (!possibleHitsRemaining)
			break;

		static auto damageReductionBullets = m_cvar2()->FindVar(crypt_str("ff_damage_reduction_bullets"));
		static auto damageBulletPenetration = m_cvar2()->FindVar(crypt_str("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat(), !e))
			break;

		visible = false;
	}

	return false;
}

autowall::returninfo_t autowall::wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e)
{
	cheat.globals.autowalling = true;
	auto tmp = point - eye_pos;

	auto angles = ZERO;
	math::vector_angles(tmp, angles);

	auto direction = ZERO;
	math::angle_vectors(angles, direction);

	direction.NormalizeInPlace();

	auto visible = true;
	auto damage = -1.0f;
	auto hitbox = -1;
	
	bool autowall = fire_bullet(cheat.globals.weapon, direction, visible, damage, hitbox, e, eye_pos);
	cheat.globals.autowalling = false;
	return autowall ? returninfo_t(visible, static_cast<int>(damage), hitbox) : returninfo_t();
}