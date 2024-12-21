#pragma once
#include "..\..\includes.hpp"
#include "..\serverside\animation_system.h"

#define MAX_SPREAD_SEED 256

class target
{
public:
	player_t* e;

	adjust_data* last_record;
	adjust_data* history_record;

	target()
	{
		e = nullptr;

		last_record = nullptr;
		history_record = nullptr;
	}

	target(player_t* e, adjust_data* last_record, adjust_data* history_record)
	{
		this->e = e;

		this->last_record = last_record;
		this->history_record = history_record;
	}
};

class scan_point
{
public:
	Vector point;
	int hitbox;
	bool center;
	float safe;

	scan_point()
	{
		point.Zero();
		hitbox = -1;
		center = false;
		safe = 0.0f;
	}

	scan_point(const Vector& point, const int& hitbox, const bool& center)
	{
		this->point = point;
		this->hitbox = hitbox;
		this->center = center;
	}

	void reset()
	{
		point.Zero();
		hitbox = -1;
		center = false;
		safe = 0.0f;
	}
};

class scan_data
{
public:
	scan_point point;
	bool visible;
	int damage;
	int hitbox;

	scan_data()
	{
		reset();
	}

	void reset()
	{
		point.reset();
		visible = false;
		damage = -1;
		hitbox = -1;
	}

	bool valid()
	{
		return damage >= 1 && hitbox != -1;
	}
};

struct LastTarget
{
	adjust_data record;
	scan_data data;
	float distance;
};

class scanned_target
{
public:
	adjust_data* record;
	scan_data data;

	float fov;
	float distance;
	int health;

	scanned_target()
	{
		reset();
	}

	scanned_target(const scanned_target& data)
	{
		this->record = data.record;
		this->data = data.data;
		this->fov = data.fov;
		this->distance = data.distance;
		this->health = data.health;
	}

	scanned_target& operator=(const scanned_target& data)
	{
		this->record = data.record;
		this->data = data.data;
		this->fov = data.fov;
		this->distance = data.distance;
		this->health = data.health;

		return *this;
	}

	scanned_target(adjust_data* record, const scan_data& data)
	{
		this->record = record;
		this->data = data;

		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		auto aim_angle = math::calculate_angle(cheat.globals.eye_pos, data.point.point);
		auto fov = math::get_fov(viewangles, aim_angle);

		this->fov = fov;
		this->distance = cheat.globals.eye_pos.DistTo(data.point.point);
		this->health = record->player->m_iHealth();
	}

	void reset()
	{
		record = nullptr;
		data.reset();

		fov = 0.0f;
		distance = 0.0f;
		health = 0;
	}
};

class ragebot : public singleton <ragebot>
{
	void automatic_revolver(CUserCmd* cmd);
	void prepare_targets();
	adjust_data* get_record(std::deque <adjust_data>* records, bool history);
	int get_minimum_damage(bool visible, int health);
	void scan_targets();
	bool bounding_box_check(adjust_data* record);
	bool automatic_stop(CUserCmd* cmd);
	void find_best_target();
	void fire(CUserCmd* cmd);
	int hitchance(const Vector& aim_angle);
	float GetBodyScale(player_t* player) const;
	float GetHeadScale(player_t* player);

	std::vector<scanned_target> scanned_targets;
	scanned_target final_target;

	inline static std::vector<std::pair<float, float>> precomputed_seeds = {};

	void build_seed_table()
	{
		for (auto i = 0; i < MAX_SPREAD_SEED; i++) {
			math::random_seed(i + 1);

			const auto pi_seed = math::random_float(0.f, DirectX::XM_2PI);

			precomputed_seeds.emplace_back(math::random_float(0.f, 1.f), pi_seed);
		}
	}

	Vector calculate_spread(int seed, float inaccuracy, float spread)
	{
		auto wepInfo = cheat.globals.weapon->get_weapon_info();
		int itemDefIndex;
		float recoilIndex, r1, r2, r3, r4, s1, c1, s2, c2;

		if (!wepInfo || !wepInfo->iBullets)
			return ZERO;

		itemDefIndex = cheat.globals.weapon->m_iItemDefinitionIndex();
		recoilIndex = wepInfo->iRecoilSeed;

		r1 = std::get<0>(precomputed_seeds[seed]);
		r2 = std::get<1>(precomputed_seeds[seed]);
		r3 = std::get<0>(precomputed_seeds[seed]);
		r4 = std::get<1>(precomputed_seeds[seed]);

		if (itemDefIndex == WEAPON_NEGEV && recoilIndex < 3.f)
		{
			for (int i = 3; i > recoilIndex; i--)
			{
				r1 *= r1;
				r3 *= r3;
			}

			r1 = 1.f - r1;
			r3 = 1.f - r3;
		}

		c1 = std::cos(r2);
		c2 = std::cos(r4);
		s1 = std::sin(r2);
		s2 = std::sin(r4);

		return Vector(
			(c1 * (r1 * inaccuracy)) + (c2 * (r3 * spread)),
			(s1 * (r1 * inaccuracy)) + (s2 * (r3 * spread)),
			0.f
		);
	}
public:
	void run(CUserCmd* cmd);
	bool baimHitboxesCheck(int hitbox);
	void scan(adjust_data* record, scan_data& data, const Vector& shoot_position = cheat.globals.eye_pos, bool optimized = false);
	std::vector <int> get_hitboxes(adjust_data* record, bool optimized = false);
	std::vector <scan_point> get_points(adjust_data* record, int hitbox, bool from_aim = true);
	bool hitboxIntersection(player_t* entity, matrix3x4_t* matrix, int iHitbox, const Vector& start, const Vector& end);
	bool can_hit(player_t* player, matrix3x4_t* bones, int box, const Vector& start, const Vector& end);
	bool hitbox_intersection(player_t* e, matrix3x4_t* matrix, int hbox, const Vector& start, const Vector& end, float* safe = nullptr);

	std::vector<target> targets;
	std::vector<adjust_data> backup;

	Config::Ragebot_t::Weapon_t currentWeapon;

	int last_target_index = -1;
	LastTarget last_target[65];

	Vector last_shoot_position;
	bool should_stop;
};
