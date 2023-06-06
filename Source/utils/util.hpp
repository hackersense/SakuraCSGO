#pragma once

#include "..\includes.hpp"
#include "..\sdk\math\Vector.hpp"
#include "..\sdk\math\Vector2D.hpp"
#include "..\sdk\misc\CUserCmd.hpp"
#include "..\utils\crypt_str.h"

class entity_t;
class player_t;
class matrix3x4_t;
class IClientEntity;
class CGameTrace;
class NEWMaterial;
class CTraceFilter;
class c_baseplayeranimationstate;

struct datamap_t;

struct Box
{
	int x = 0;
	int y = 0;
	int w = 0;
	int h = 0;
};

struct HPInfo
{
	int hp = -1;
	int hp_difference = 0;
	float hp_difference_time = 0.0f;
};

struct SoundInfo
{
	float last_time = FLT_MIN;
	Vector origin = ZERO;
};

enum class GameMode
{
	None,
	Casual,
	Competitive,
	Wingman,
	WeaponsExpert,
	ArmsRace,
	Demolition,
	Deathmatch,
	Training,
	Custom,
	Guardian,
	CoopStrike,
	WarGames,
	DangerZone
};

namespace util
{
	uint64_t FindSignature(const char* szModule, const char* szSignature);

	bool visible(const Vector& start, const Vector& end, entity_t* entity, player_t* from);
	bool is_button_down(int code);

	int epoch_time();
	void RotateMovement(CUserCmd* cmd, float yaw);
	bool get_bbox(entity_t* entity, Box& box);
	void trace_line(Vector& start, Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* trace);
	void trace_line_entity(Vector& start, Vector& end, unsigned int mask, IClientEntity* entity, CGameTrace* trace);
	void clip_trace_to_players(IClientEntity* e, const Vector& start, const Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* tr);
	void movement_fix(Vector& wish_angle, CUserCmd* m_pcmd);
	unsigned int find_in_datamap(datamap_t* map, const char* name);
	void color_modulate(float color[3], NEWMaterial* material);
	bool get_backtrack_matrix(player_t* e, matrix3x4_t* matrix);
	void create_state(c_baseplayeranimationstate* state, player_t* e);
	void update_state(c_baseplayeranimationstate* state, const Vector& angles);
	void reset_state(c_baseplayeranimationstate* state);
	void fix_mouse_delta(CUserCmd* cmd);
	void thirdperson(bool fakeducking);
	Vector2D get_speed_move(CUserCmd* cmd);
	Vector2D get_speed_move(CUserCmd* cmd, float prevYaw);
	Vector2D get_max_move();
	void copy_command(CUserCmd* cmd, int tickbase_shift, bool teleport, bool autopeek);
	float get_interpolation();
	float server_time(CUserCmd* cmd = nullptr) noexcept;
	bool isChatOpen();
	GameMode getGameMode();
}