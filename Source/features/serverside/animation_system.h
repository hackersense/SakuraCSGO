#pragma once

#include "..\..\sdk\structs.hpp"

enum
{
	MAIN,
	NONE,
	FIRST,
	SECOND
};

enum class ResolverType
{
	ORIGINAL,
	BRUTEFORCE,
	LBY,
	LAYERS,
	TRACE,
	DIRECTIONAL
};

enum ResolverSide
{
	RESOLVER_ORIGINAL,
	RESOLVER_ZERO,
	RESOLVER_FIRST,
	RESOLVER_SECOND,
	RESOLVER_LOW_FIRST,
	RESOLVER_LOW_SECOND,
	RESOLVER_DEFAULT
};

struct matrixes
{
	matrix3x4_t main[MAXSTUDIOBONES];
	matrix3x4_t zero[MAXSTUDIOBONES];
	matrix3x4_t first[MAXSTUDIOBONES];
	matrix3x4_t second[MAXSTUDIOBONES];
};

class adjust_data
{
public:
	player_t* player;
	int index;

	AnimationLayer layers[15];
	matrixes matrixes_data;

	ResolverType type;
	ResolverSide side;

	std::string username;

	bool invalid;
	bool immune;
	bool dormant;
	bool robot;
	bool shot;
	int flags;
	int bone_count;
	float last_shot_time;
	float simulation_time;
	float duck_amount;
	float lby;

	Vector angles;
	Vector abs_angles;
	Vector eye_position;
	Vector velocity;
	Vector origin;
	Vector mins;
	Vector maxs;

	adjust_data()
	{
		reset();
	}

	void reset()
	{
		player = nullptr;
		index = -1;

		type = ResolverType::ORIGINAL;
		side = ResolverSide::RESOLVER_ORIGINAL;

		invalid = false;
		immune = false;
		dormant = false;
		robot = false;
		shot = false;
		flags = 0;
		bone_count = 0;
		last_shot_time = 0.0f;
		simulation_time = 0.0f;
		duck_amount = 0.0f;
		lby = 0.0f;

		angles.Zero();
		abs_angles.Zero();
		eye_position.Zero();
		velocity.Zero();
		origin.Zero();
		mins.Zero();
		maxs.Zero();
	}

	adjust_data(player_t* e, bool store = true)
	{
		type = ResolverType::ORIGINAL;
		side = ResolverSide::RESOLVER_ORIGINAL;

		invalid = false;
		store_data(e, store);
	}

	void store_data(player_t* e, bool store = true)
	{
		if (!e->is_alive())
			return;

		player = e;
		index = player->EntIndex();

		if (store)
		{
			memcpy(layers, e->getAnimationLayers(), e->getAnimationLayersCount() * sizeof(AnimationLayer));
			memcpy(matrixes_data.main, player->m_CachedBoneData().base(), player->m_CachedBoneData().count() * sizeof(matrix3x4_t));
		}

		immune = player->m_bGunGameImmunity() || player->m_fFlags() & FL_FROZEN;
		dormant = player->IsDormant();

		PlayerInfo player_info;
		m_engine()->GetPlayerInfo(index, &player_info);

		username = player_info.getUsername();
		robot = player_info.fakeplayer;
		flags = player->m_fFlags();
		bone_count = player->m_CachedBoneData().count();

		last_shot_time = player->getActiveWeapon() ? player->getActiveWeapon()->m_fLastShotTime() : 0.f;
		simulation_time = player->m_flSimulationTime();
		duck_amount = player->m_flDuckAmount();
		lby = player->m_flLowerBodyYawTarget();

		angles = player->m_angEyeAngles();
		abs_angles = player->GetAbsAngles();
		eye_position = player->m_angEyeAngles();
		velocity = player->m_vecVelocity();
		origin = player->m_vecOrigin();
		mins = player->GetCollideable()->OBBMins();
		maxs = player->GetCollideable()->OBBMaxs();
	}

	void adjust_player()
	{
		if (!valid(false))
			return;

		memcpy(player->getAnimationLayers(), layers, player->getAnimationLayersCount() * sizeof(AnimationLayer));
		memcpy(player->m_CachedBoneData().base(), matrixes_data.main, player->m_CachedBoneData().count() * sizeof(matrix3x4_t));

		player->m_fFlags() = flags;
		player->m_CachedBoneData().size = bone_count;

		player->m_flSimulationTime() = simulation_time;
		player->m_flDuckAmount() = duck_amount;
		player->m_flLowerBodyYawTarget() = lby;

		player->m_angEyeAngles() = angles;
		player->set_abs_angles(abs_angles);
		player->m_angEyeAngles() = eye_position;
		player->m_vecVelocity() = velocity;
		player->m_vecOrigin() = origin;
		player->set_abs_origin(origin);
		player->GetCollideable()->OBBMins() = mins;
		player->GetCollideable()->OBBMaxs() = maxs;
	}

	bool valid(bool extra_checks = true)
	{
		if (!this)
			return false;

		if (index > 0)
			player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(index));

		if (!player)
			return false;

		if (player->m_lifeState() != LIFE_ALIVE)
			return false;

		if (immune)
			return false;

		if (dormant)
			return false;

		if (!extra_checks)
			return true;

		if (invalid)
			return false;

		auto net_channel_info = m_engine()->GetNetChannelInfo();

		if (!net_channel_info)
			return false;

		static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

		auto outgoing = net_channel_info->GetLatency(FLOW_OUTGOING);
		auto incoming = net_channel_info->GetLatency(FLOW_INCOMING);

		auto correct = math::clamp(outgoing + incoming + util::get_interpolation(), 0.0f, sv_maxunlag->GetFloat());

		auto curtime = localPlayer->is_alive() ? TICKS_TO_TIME(cheat.globals.fixed_tickbase) : m_globals()->m_curtime;
		auto delta_time = correct - (curtime - simulation_time);

		if (fabs(delta_time) > 0.2f)
			return false;

		auto extra_choke = 0;

		if (cheat.globals.fakeducking)
			extra_choke = 14 - m_clientstate()->m_iChokedCommands;

		auto server_tickcount = extra_choke + m_globals()->m_tickcount + TIME_TO_TICKS(outgoing + incoming);
		auto dead_time = static_cast<int>(TICKS_TO_TIME(server_tickcount) - sv_maxunlag->GetFloat());

		return simulation_time >= static_cast<float>(dead_time);
	}
};

class optimized_adjust_data
{
public:
	int index;
	player_t* player;

	float simulation_time;
	float duck_amount;

	Vector angles;
	Vector origin;

	bool shot;

	optimized_adjust_data()
	{
		reset();
	}

	void reset()
	{
		index = 0;
		player = nullptr;

		simulation_time = 0.0f;
		duck_amount = 0.0f;

		angles.Zero();
		origin.Zero();

		shot = false;
	}
};

class resolver
{
	player_t* player = nullptr;
	adjust_data* player_record = nullptr;
	float original_goal_feet_yaw = 0.0f;

	bool was_first_bruteforce = false;
	bool was_second_bruteforce = false;
	bool was_first_low_bruteforce = false;
	bool was_second_low_bruteforce = false;
	bool side = false;

	float lock_side = 0.0f;

public:
	void initialize(player_t* e, adjust_data* record, float goal_feet_yaw);
	void reset();
	bool is_slow_walking(player_t* entity);
	bool isRoll(player_t* entity);
	void test(player_t* e);
	void resolve_yaw();
	void resolve_pitch();

	AnimationLayer previous_layers[15] = { };
	float second_goal_feet_yaw = 0.0f;
	float zero_goal_feet_yaw = 0.0f;
	float first_goal_feet_yaw = 0.0f;

	ResolverSide last_side = ResolverSide::RESOLVER_ORIGINAL;
	ResolverSide backup_side = ResolverSide::RESOLVER_ORIGINAL;
	int random_number = 0;
};

extern std::deque<adjust_data> player_records[65];

class animation_system : public singleton<animation_system>
{
public:
	void init();
	void apply_interpolation_flags(player_t* e, int flag);
	void run(ClientFrameStage_t stage);
	bool valid(int i, player_t* e);
	void update_player_animations(player_t* e);

	resolver player_resolver[65] = { };

	bool is_dormant[65] = { };
	float previous_goal_feet_yaw[65] = { };
};