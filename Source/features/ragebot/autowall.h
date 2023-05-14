#pragma once
#include "..\..\includes.hpp"

class weapon_info_t;
class weapon_t;

struct fire_bullet_data
{
	Vector src;
	CGameTrace enter_trace;
	Vector direction;
	CTraceFilter filter;
	float trace_length;
	float trace_length_remaining;
	float current_damage;
	int penetrate_count;
};

class autowall : public singleton <autowall>
{
public:
	struct returninfo_t
	{
		bool valid = false;

		bool visible = false;
		int damage = -1;
		int hitbox = -1;

		returninfo_t()
		{
			valid = false;

			this->visible = false;
			this->damage = -1;
			this->hitbox = -1;
		}

		returninfo_t(bool visible, int damage, int hitbox)
		{
			valid = true;

			this->visible = visible;
			this->damage = damage;
			this->hitbox = hitbox;
		}
	};

	struct FireBulletData
	{
		Vector src;
		CGameTrace enter_trace;
		Vector direction;
		CTraceFilter filter;
		weapon_info_t* weapon_data;
		float trace_length;
		float trace_length_remaining;
		float length_to_end;
		float current_damage;
		int penetrate_count;
	};

	void scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage);
	bool is_breakable_entity(IClientEntity* e);
	bool traceToExit(const CGameTrace& enterTrace, const Vector& start, const Vector& direction, Vector& end, CGameTrace& exitTrace);
	bool trace_to_exit(Vector startPosition, Vector direction, Vector& end, CGameTrace& enterTrace, CGameTrace& exitTrace, float stepSize, float maxDistance);
	bool handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact = false);
	returninfo_t wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e);
	bool fire_bullet(weapon_t* weapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e = nullptr, const Vector& pos = ZERO);
};