#include "util.hpp"
#include "..\features\visuals\player_esp.h"
#include "..\features\serverside\animation_system.h"
#include "..\features\misc\misc.h"
#include <thread>

#define INRANGE(x, a, b) (x >= a && x <= b)
#define GETBITS(x) (INRANGE((x & (~0x20)),'A','F') ? ((x & (~0x20)) - 'A' + 0xA) : (INRANGE(x, '0', '9') ? x - '0' : 0))
#define GETBYTE(x) (GETBITS(x[0]) << 4 | GETBITS(x[1]))

namespace util
{
	int epoch_time()
	{
		return std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
	}

	uint64_t FindSignature(const char* szModule, const char* szSignature)
	{
		MODULEINFO modInfo;
		GetModuleInformation(GetCurrentProcess(), GetModuleHandleA(szModule), &modInfo, sizeof(MODULEINFO));

		uintptr_t startAddress = (DWORD)modInfo.lpBaseOfDll;
		uintptr_t endAddress = startAddress + modInfo.SizeOfImage;

		const char* pat = szSignature;
		uintptr_t firstMatch = 0;

		for (auto pCur = startAddress; pCur < endAddress; pCur++)
		{
			if (!*pat)
				return firstMatch;

			if (*(PBYTE)pat == '\?' || *(BYTE*)pCur == GETBYTE(pat))
			{
				if (!firstMatch)
					firstMatch = pCur;

				if (!pat[2])
					return firstMatch;

				if (*(PWORD)pat == '\?\?' || *(PBYTE)pat != '\?')
					pat += 3;
				else
					pat += 2;
			}
			else
			{
				pat = szSignature;
				firstMatch = 0;
			}
		}
		if (cheat.isDebugMode)
			MessageBoxA(nullptr, ("Failed to find pattern:\n" + std::string(szSignature)).c_str(), ("Sakura - " + std::string(szModule)).c_str(), MB_OK | MB_ICONWARNING);
		return 0;
	}

	bool visible(const Vector& start, const Vector& end, entity_t* entity, player_t* from)
	{
		CGameTrace trace;

		Ray_t ray;
		ray.Init(start, end);

		CTraceFilter filter;
		filter.pSkip = from;

		m_trace()->TraceRay(ray, MASK_SHOT | CONTENTS_HITBOX, &filter, &trace);

		return trace.entity == entity || trace.fraction > 0.97f;
	}

	bool is_button_down(int code)
	{
		if (code <= KEY_NONE || code >= KEY_MAX || isChatOpen() || !m_engine()->IsActiveApp() || m_engine()->Con_IsVisible())
			return false;

		return GetAsyncKeyState(code) & 0x8000;
	}

	void movement_fix(Vector& origin_angle, CUserCmd* m_pcmd)
	{
		if (m_pcmd->m_viewangles.z == 0.0f)
		{
			Vector view_fwd, view_right, view_up, cmd_fwd, cmd_right, cmd_up;
			auto viewangles = m_pcmd->m_viewangles;
			viewangles.Normalized();

			math::angle_vectors(origin_angle, &view_fwd, &view_right, &view_up);
			math::angle_vectors(viewangles, &cmd_fwd, &cmd_right, &cmd_up);

			float forward_origin = sqrtf((view_fwd.x * view_fwd.x) + (view_fwd.y * view_fwd.y));
			float side_origin = sqrtf((view_right.x * view_right.x) + (view_right.y * view_right.y));
			float up_origin = sqrtf(view_up.z * view_up.z);

			Vector norm_view_fwd((1.f / forward_origin) * view_fwd.x, (1.f / forward_origin) * view_fwd.y, 0.f);
			Vector norm_view_right((1.f / side_origin) * view_right.x, (1.f / side_origin) * view_right.y, 0.f);
			Vector norm_view_up(0.f, 0.f, (1.f / up_origin) * view_up.z);

			float forward_cmd = sqrtf((cmd_fwd.x * cmd_fwd.x) + (cmd_fwd.y * cmd_fwd.y));
			float side_cmd = sqrtf((cmd_right.x * cmd_right.x) + (cmd_right.y * cmd_right.y));
			float up_cmd = sqrtf(cmd_up.z * cmd_up.z);

			Vector norm_cmd_fwd((1.f / forward_cmd) * cmd_fwd.x, (1.f / forward_cmd) * cmd_fwd.y, 0.f);
			Vector norm_cmd_right((1.f / side_cmd) * cmd_right.x, (1.f / side_cmd) * cmd_right.y, 0.f);
			Vector norm_cmd_up(0.f, 0.f, (1.f / up_cmd) * cmd_up.z);

			float forward_x = norm_view_fwd.x * m_pcmd->m_forwardmove;
			float forward_y = norm_view_fwd.y * m_pcmd->m_forwardmove;
			float forward_z = norm_view_fwd.z * m_pcmd->m_forwardmove;
			float side_x = norm_view_right.x * m_pcmd->m_sidemove;
			float side_y = norm_view_right.y * m_pcmd->m_sidemove;
			float side_z = norm_view_right.z * m_pcmd->m_sidemove;
			float up_x = norm_view_up.x * m_pcmd->m_upmove;
			float up_y = norm_view_up.z * m_pcmd->m_upmove;
			float up_z = norm_view_up.y * m_pcmd->m_upmove;

			static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
			static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
			static auto cl_upspeed = m_cvar()->FindVar(crypt_str("cl_upspeed"));

			m_pcmd->m_forwardmove = math::clamp(((((norm_cmd_fwd.x * side_x) + (norm_cmd_fwd.y * side_y)) + (norm_cmd_fwd.z * side_z))
				+ (((norm_cmd_fwd.x * forward_x) + (norm_cmd_fwd.y * forward_y)) + (norm_cmd_fwd.z * forward_z)))
				+ (((norm_cmd_fwd.y * up_x) + (norm_cmd_fwd.x * up_z)) + (norm_cmd_fwd.z * up_y)), -cl_forwardspeed->GetFloat(), cl_forwardspeed->GetFloat());
			m_pcmd->m_sidemove = math::clamp(((((norm_cmd_right.x * side_x) + (norm_cmd_right.y * side_y)) + (norm_cmd_right.z * side_z))
				+ (((norm_cmd_right.x * forward_x) + (norm_cmd_right.y * forward_y)) + (norm_cmd_right.z * forward_z)))
				+ (((norm_cmd_right.x * up_z) + (norm_cmd_right.y * up_x)) + (norm_cmd_right.z * up_y)), -cl_sidespeed->GetFloat(), cl_sidespeed->GetFloat());
			m_pcmd->m_upmove = math::clamp(((((norm_cmd_up.x * side_y) + (norm_cmd_up.y * side_x)) + (norm_cmd_up.z * side_z))
				+ (((norm_cmd_up.x * forward_y) + (norm_cmd_up.y * forward_x)) + (norm_cmd_up.z * forward_z)))
				+ (((norm_cmd_up.x * up_x) + (norm_cmd_up.y * up_z)) + (norm_cmd_up.z * up_y)), -cl_upspeed->GetFloat(), cl_upspeed->GetFloat());
		}
		else
		{
			Vector move, dir;
			float delta, len;
			Vector move_angle;

			if (!(cheat.local()->m_fFlags() & FL_ONGROUND) && m_pcmd->m_viewangles.z != 0 && m_pcmd->m_buttons & IN_ATTACK)
				m_pcmd->m_sidemove = 0;

			move = Vector(m_pcmd->m_forwardmove, m_pcmd->m_sidemove, 0);

			len = move.Normalize();

			if (!len)
				return;

			math::vector_angles(move, move_angle);

			delta = (m_pcmd->m_viewangles.y - origin_angle.y);

			move_angle.y += delta;

			math::angle_vectors(move_angle, dir);

			dir *= len;

			if (cheat.local()->get_move_type() == MOVETYPE_LADDER)
			{
				if (m_pcmd->m_viewangles.x >= 45 && origin_angle.x < 45 && std::abs(delta) <= 65)
					dir.x = -dir.x;

				m_pcmd->m_forwardmove = dir.x;
				m_pcmd->m_sidemove = dir.y;

				if (m_pcmd->m_forwardmove > 200)
					m_pcmd->m_buttons |= IN_FORWARD;
				else if (m_pcmd->m_forwardmove < -200)
					m_pcmd->m_buttons |= IN_BACK;
				if (m_pcmd->m_sidemove > 200)
					m_pcmd->m_buttons |= IN_MOVERIGHT;
				else if (m_pcmd->m_sidemove < -200)
					m_pcmd->m_buttons |= IN_MOVELEFT;
			}
			else
			{
				if (m_pcmd->m_viewangles.x < -90 || m_pcmd->m_viewangles.x > 90)
					dir.x = -dir.x;

				m_pcmd->m_forwardmove = dir.x;
				m_pcmd->m_sidemove = dir.y;
			}

			m_pcmd->m_forwardmove = math::clamp(m_pcmd->m_forwardmove, -450.f, 450.f);
			m_pcmd->m_sidemove = math::clamp(m_pcmd->m_sidemove, -450.f, 450.f);
			m_pcmd->m_upmove = math::clamp(m_pcmd->m_upmove, -320.f, 320.f);
		}
	}

	unsigned int find_in_datamap(datamap_t* map, const char *name)
	{
		while (map)
		{
			for (auto i = 0; i < map->dataNumFields; i++)
			{
				if (!map->dataDesc[i].fieldName)
					continue;

				if (!strcmp(name, map->dataDesc[i].fieldName))
					return map->dataDesc[i].fieldOffset[TD_OFFSET_NORMAL];

				if (map->dataDesc[i].fieldType == FIELD_EMBEDDED)
				{
					if (map->dataDesc[i].td)
					{
						unsigned int offset;

						if (offset = find_in_datamap(map->dataDesc[i].td, name))
							return offset;
					}
				}
			}

			map = map->baseMap;
		}

		return 0;
	}

	bool get_bbox(entity_t* entity, Box& box)
	{
		auto collideable = entity->GetCollideable();
		if (!collideable)
			return false;

		auto& m_CoordinateFrame = entity->m_CoordinateFrame();

		auto& min = collideable->OBBMins();
		auto& max = collideable->OBBMaxs();

		Vector points[8] =
		{
			Vector(min.x, min.y, min.z),
			Vector(min.x, max.y, min.z),
			Vector(max.x, max.y, min.z),
			Vector(max.x, min.y, min.z),
			Vector(max.x, max.y, max.z),
			Vector(min.x, max.y, max.z),
			Vector(min.x, min.y, max.z),
			Vector(max.x, min.y, max.z)
		};

		Vector pointsTransformed[8];

		for (auto i = 0; i < 8; i++)
			math::vector_transform(points[i], m_CoordinateFrame, pointsTransformed[i]);

		Vector flb;
		Vector brt;
		Vector blb;
		Vector frt;
		Vector frb;
		Vector brb;
		Vector blt;
		Vector flt;

		auto bFlb = math::world_to_screen(pointsTransformed[3], flb);
		auto bBrt = math::world_to_screen(pointsTransformed[5], brt);
		auto bBlb = math::world_to_screen(pointsTransformed[0], blb);
		auto bFrt = math::world_to_screen(pointsTransformed[4], frt);
		auto bFrb = math::world_to_screen(pointsTransformed[2], frb);
		auto bBrb = math::world_to_screen(pointsTransformed[1], brb);
		auto bBlt = math::world_to_screen(pointsTransformed[6], blt);
		auto bFlt = math::world_to_screen(pointsTransformed[7], flt);

		if (!bFlb && !bBrt && !bBlb && !bFrt && !bFrb && !bBrb && !bBlt && !bFlt)
			return false;

		Vector arr[8] =
		{
			flb,
			brt,
			blb,
			frt,
			frb,
			brb,
			blt,
			flt
		};

		auto left = flb.x;
		auto top = flb.y;
		auto right = flb.x;
		auto bottom = flb.y;

		for (int i = 1; i < 8; i++)
		{
			if (left > arr[i].x)
				left = arr[i].x;
			if (top < arr[i].y)
				top = arr[i].y;
			if (right < arr[i].x)
				right = arr[i].x;
			if (bottom > arr[i].y)
				bottom = arr[i].y;
		}

		box.x = (int)left;
		box.y = (int)bottom;
		box.w = (int)(right - left);
		box.h = (int)(top - bottom);

		return true;
	}

	void trace_line(Vector& start, Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* trace)
	{
		Ray_t ray;
		ray.Init(start, end);

		m_trace()->TraceRay(ray, mask, filter, trace);
	}

	void trace_line_entity(Vector& start, Vector& end, unsigned int mask, IClientEntity* entity, CGameTrace* trace)
	{
		Ray_t ray;
		ray.Init(start, end);

		CTraceFilter filter;
		filter.pSkip = entity;

		m_trace()->TraceRay(ray, mask, &filter, trace);
	}

	void clip_trace_to_players(IClientEntity* e, const Vector& start, const Vector& end, unsigned int mask, CTraceFilter* filter, CGameTrace* tr)
	{
		Vector mins = e->GetCollideable()->OBBMins(), maxs = e->GetCollideable()->OBBMaxs();

		Vector dir(end - start);
		dir.Normalize();

		Vector center = (maxs + mins) / 2, pos = (center + e->GetAbsOrigin());

		Vector to = pos - start;
		float range_along = dir.Dot(to);

		float range;
		if (range_along < 0.f)
			range = -to.Length();
		else if (range_along > dir.Length())
			range = -(pos - end).Length();
		else {
			auto ray = pos - (dir * range_along + start);
			range = ray.Length();
		}

		if (range <= 60.f)
		{
			CGameTrace trace;

			Ray_t ray;
			ray.Init(start, end);

			m_trace()->ClipRayToEntity(ray, mask, e, &trace);

			if (tr->fraction > trace.fraction)
				*tr = trace;
		}
	}

	void RotateMovement(CUserCmd* cmd, float yaw)
	{
		Vector viewangles;
		m_engine()->GetViewAngles(viewangles);

		float rotation = DEG2RAD(viewangles.y - yaw);

		float cos_rot = cos(rotation);
		float sin_rot = sin(rotation);

		float new_forwardmove = cos_rot * cmd->m_forwardmove - sin_rot * cmd->m_sidemove;
		float new_sidemove = sin_rot * cmd->m_forwardmove + cos_rot * cmd->m_sidemove;

		cmd->m_forwardmove = new_forwardmove;
		cmd->m_sidemove = new_sidemove;
	}

	void color_modulate(float color[3], IMaterial* material)
	{
		auto found = false;
		auto var = material->FindVar(crypt_str("$envmaptint"), &found);

		if (found)
			var->set_vec_value(color[0], color[1], color[2]);

		m_renderview()->SetColorModulation(color[0], color[1], color[2]);
	}

	bool get_backtrack_matrix(player_t* e, matrix3x4_t* matrix)
	{
		if (!configs.ragebot.enabled && !configs.legitbot.enabled && !configs.backtrack.enabled)
			return false;

		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return false;

		auto i = e->EntIndex();

		if (i < 1 || i > 64)
			return false;

		auto records = &player_records[i];

		if (records->size() < 2)
			return false;

		for (auto record = records->rbegin(); record != records->rend(); ++record)
		{
			if (!record->valid())
				continue;

			if (record->origin.DistTo(e->GetAbsOrigin()) < 1.0f)
				return false;

			auto curtime = m_globals()->m_curtime;
			auto range = 0.2f;

			if (cheat.local()->is_alive())
				curtime = TICKS_TO_TIME(cheat.globals.fixed_tickbase);
			
			auto next_record = record + 1;
			auto end = next_record == records->rend();

			auto next = end ? e->GetAbsOrigin() : next_record->origin;
			auto time_next = end ? e->m_flSimulationTime() : next_record->simulation_time;

			auto correct = nci->GetLatency(FLOW_OUTGOING) + nci->GetLatency(FLOW_INCOMING) + util::get_interpolation();
			auto time_delta = time_next - record->simulation_time;

			auto add = end ? range : time_delta;
			auto deadtime = record->simulation_time + correct + add;
			auto delta = deadtime - curtime;

			auto mul = 1.0f / add;
			auto lerp = math::lerp(next, record->origin, math::clamp(delta * mul, 0.0f, 1.0f));

			matrix3x4_t result[MAXSTUDIOBONES];
			memcpy(result, record->matrixes_data.main, MAXSTUDIOBONES * sizeof(matrix3x4_t));

			for (auto j = 0; j < MAXSTUDIOBONES; j++)
			{
				auto matrix_delta = math::matrix_get_origin(record->matrixes_data.main[j]) - record->origin;
				math::matrix_set_origin(matrix_delta + lerp, result[j]);
			}

			memcpy(matrix, result, MAXSTUDIOBONES * sizeof(matrix3x4_t));
			return true;
		}

		return false;
	}

	void create_state(c_baseplayeranimationstate* state, player_t* e)
	{
		if (!state || !e)
			return;

		static auto createAnimState = reinterpret_cast<void(__thiscall*)(c_baseplayeranimationstate*, player_t*)>(memory->createState);

		if (!createAnimState)
			return;

		createAnimState(state, e);
	}

	void update_state(c_baseplayeranimationstate* state, const Vector& angles)
	{
		if (!state)
			return;

		static auto updateAnimState = reinterpret_cast<void(__vectorcall*)(void*, void*, float, float, float, void*)>(memory->updateState);

		if (!updateAnimState)
			return;

		updateAnimState(state, nullptr, angles.z, angles.y, angles.x, nullptr);
	}

	void reset_state(c_baseplayeranimationstate* state)
	{
		if (!state)
			return;

		static auto resetAnimState = reinterpret_cast<void(__thiscall*)(c_baseplayeranimationstate*)>(memory->resetState);

		if (!resetAnimState)
			return;

		resetAnimState(state);
	}

	void fix_mouse_delta(CUserCmd* cmd)
	{
		if (!cmd)
			return;

		static Vector delta_viewangles{ };
		Vector delta = cmd->m_viewangles - delta_viewangles;

		delta.x = std::clamp(delta.x, -89.0f, 89.0f);
		delta.y = std::clamp(delta.y, -180.0f, 180.0f);
		delta.z = 0.0f;
		static auto sensitivity = m_cvar()->FindVar("sensitivity");

		if (delta.x != 0.f) {
			static auto pitch = m_cvar()->FindVar("m_pitch");

			int final_dy = static_cast<int>((delta.x / pitch->GetFloat()) / sensitivity->GetFloat());
			if (final_dy <= 32767) {
				if (final_dy >= -32768) {
					if (final_dy >= 1 || final_dy < 0) {
						if (final_dy <= -1 || final_dy > 0)
							final_dy = final_dy;
						else
							final_dy = -1;
					}
					else {
						final_dy = 1;
					}
				}
				else {
					final_dy = 32768;
				}
			}
			else {
				final_dy = 32767;
			}

			cmd->m_mousedy = static_cast<short>(final_dy);
		}

		if (delta.y != 0.f) {
			static auto yaw = m_cvar()->FindVar("m_yaw");

			int final_dx = static_cast<int>((delta.y / yaw->GetFloat()) / sensitivity->GetFloat());
			if (final_dx <= 32767) {
				if (final_dx >= -32768) {
					if (final_dx >= 1 || final_dx < 0) {
						if (final_dx <= -1 || final_dx > 0)
							final_dx = final_dx;
						else
							final_dx = -1;
					}
					else {
						final_dx = 1;
					}
				}
				else {
					final_dx = 32768;
				}
			}
			else {
				final_dx = 32767;
			}

			cmd->m_mousedx = static_cast<short>(final_dx);
		}

		delta_viewangles = cmd->m_viewangles;
	}

	void thirdperson(bool fakeducking)
	{
		if (configs.player.enabled && (configs.misc.thirdperson_toggle.key > KEY_NONE && configs.misc.thirdperson_toggle.key < KEY_MAX || configs.misc.thirdperson_when_spectating))
		{
			static auto current_fraction = 0.0f;
			static auto in_thirdperson = false;

			if (!in_thirdperson && cheat.globals.in_thirdperson)
			{
				current_fraction = 0.0f;
				in_thirdperson = true;
			}
			else if (in_thirdperson && !cheat.globals.in_thirdperson)
				in_thirdperson = false;

			if (cheat.local()->is_alive() && in_thirdperson && m_clientstate()->m_iDeltaTick > 0)
			{
				auto distance = static_cast<float>(configs.misc.thirdperson_distance);

				Vector angles;
				m_engine()->GetViewAngles(angles);

				Vector inverse_angles;
				m_engine()->GetViewAngles(inverse_angles);

				inverse_angles.z = distance;

				Vector forward, right, up;
				math::angle_vectors(inverse_angles, &forward, &right, &up);

				Ray_t ray;
				CTraceFilterWorldAndPropsOnly filter;
				CGameTrace trace;

				auto& origin = cheat.local()->GetAbsOrigin();
				auto eye_pos = fakeducking ? origin + m_gamemovement()->GetPlayerViewOffset(false) : origin + cheat.local()->m_vecViewOffset();
				auto offset = eye_pos + forward * -distance + right + up;

				ray.Init(eye_pos, offset, Vector(-16.0f, -16.0f, -16.0f), Vector(16.0f, 16.0f, 16.0f));
				m_trace()->TraceRay(ray, MASK_SHOT_HULL, &filter, &trace);

				trace.fraction = std::clamp(trace.fraction, 0.f, 1.f);

				current_fraction = math::interpolate(current_fraction, trace.fraction, m_globals()->m_frametime * 10.0f);
				angles.z = distance * (configs.misc.smooth_thirdperson ? current_fraction : trace.fraction);

				m_input()->m_bCameraInThirdPerson = current_fraction > 0.1f || configs.misc.keep_thirdperson;
				m_input()->m_vecCameraOffset = angles;
			}
			else if (m_input()->m_bCameraInThirdPerson)
			{
				cheat.globals.in_thirdperson = false;
				m_input()->m_bCameraInThirdPerson = false;
				m_input()->m_vecCameraOffset.z = 0.f;
			}

			static auto require_reset = false;

			if (cheat.local()->is_alive())
			{
				require_reset = false;
				return;
			}

			if (configs.misc.thirdperson_when_spectating)
			{
				if (require_reset)
					cheat.local()->m_iObserverMode() = OBS_MODE_CHASE;

				if (cheat.local()->m_iObserverMode() == OBS_MODE_IN_EYE)
					require_reset = true;
			}
		}
		else if (cheat.globals.in_thirdperson)
		{
			cheat.globals.in_thirdperson = false;
			m_input()->m_bCameraInThirdPerson = false;
			m_input()->m_vecCameraOffset.z = 0.f;
		}
	}

	Vector2D get_speed_move(CUserCmd* cmd)
	{
		auto move = Vector(cmd->m_forwardmove, cmd->m_sidemove, cmd->m_upmove);
		float speed = sqrt((move.x * move.x) + (move.y * move.y));
		Vector move2;
		math::vector_angles(move, move2);
		move2.Normalize();
		float yaw = DEG2RAD(cmd->m_viewangles.y - cheat.globals.original.y + move2.y);
		return { cos(yaw) * speed , sin(yaw) * speed };
	}

	Vector2D get_speed_move(CUserCmd* cmd, float prevYaw)
	{
		auto move = Vector(cmd->m_forwardmove, cmd->m_sidemove, cmd->m_upmove);
		float speed = sqrt((move.x * move.x) + (move.y * move.y));
		Vector move2;
		math::vector_angles(move, move2);
		move2.Normalize();
		float yaw = DEG2RAD(cmd->m_viewangles.y - prevYaw + move2.y);
		return { cos(yaw) * speed , sin(yaw) * speed };
	}

	Vector2D get_max_move()
	{
		static auto cl_forwardspeed = m_cvar()->FindVar(crypt_str("cl_forwardspeed"));
		static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
		return { cl_forwardspeed->GetFloat(), cl_sidespeed->GetFloat() };
	}

	void copy_command(CUserCmd* cmd, int tickbase_shift, bool teleport, bool autopeek)
	{
		const auto chocked_ticks = (cmd->m_command_number % 2) != 1 ? (14 - m_clientstate()->m_iChokedCommands) : m_clientstate()->m_iChokedCommands;
		
		if (configs.ragebot.disable_teleport || !teleport)
		{
			cmd->m_forwardmove = 0;
			cmd->m_sidemove = 0;
		}
		else if (!cheat.globals.start_position.IsZero() && autopeek)
		{
			auto& current_position = cheat.local()->GetAbsOrigin();
			auto difference = current_position - cheat.globals.start_position;

			if (difference.Length2D() > 5.0f)
			{
				auto velocity = Vector(difference.x * cos(cmd->m_viewangles.y / 180.0f * M_PI) + difference.y * sin(cmd->m_viewangles.y / 180.0f * M_PI), difference.y * cos(cmd->m_viewangles.y / 180.0f * M_PI) - difference.x * sin(cmd->m_viewangles.y / 180.0f * M_PI), difference.z);

				cmd->m_forwardmove = -velocity.x * 20.0f;
				cmd->m_sidemove = velocity.y * 20.0f;
			}
			else
			{
				//cheat.globals.fired_shot = false;
				cheat.globals.start_position.Zero();
			}
		}
		else
		{
			/*
			if (cmd->m_sidemove > 5.0f)
				cmd->m_sidemove = cl_sidespeed->GetFloat();
			else if (cmd->m_sidemove < -5.0f)
				cmd->m_sidemove = -cl_sidespeed->GetFloat();

			if (cmd->m_forwardmove > 5.0f)
				cmd->m_forwardmove = cl_forwardspeed->GetFloat();
			else if (cmd->m_forwardmove < -5.0f)
				cmd->m_forwardmove = -cl_forwardspeed->GetFloat();

				const auto max_move = get_max_move();
				if (cheat.globals.original_forwardmove > 5.0f)
					cmd->m_forwardmove = max_move.x;
				else if (cheat.globals.original_forwardmove < -5.0f)
					cmd->m_forwardmove = -max_move.x;

				if (cheat.globals.original_sidemove > 5.0f)
					cmd->m_sidemove = max_move.y;
				else if (cheat.globals.original_sidemove < -5.0f)
					cmd->m_sidemove = -max_move.y;

			const auto max_move = get_max_move();
		if (abs(cmd->m_sidemove) > 10.f)
			cmd->m_sidemove = copysignf(max_move.y, cmd->m_sidemove);
		else
			cmd->m_sidemove = 0.f;

		if (abs(cmd->m_forwardmove) > 10.f)
			cmd->m_forwardmove = copysignf(max_move.x, cmd->m_forwardmove);
		else
			cmd->m_forwardmove = 0.f;
			*/
			//cmd->m_sidemove = abs(cmd->m_sidemove) > 10.f ? copysignf(450.f, cmd->m_sidemove) : 0.f;
			if (misc::get().double_tap_key && !misc::get().double_tap_disable)
			{
				const auto max_move = get_max_move();
				if (cheat.globals.original_forwardmove > 5.0f)
					cmd->m_forwardmove = max_move.x;
				else if (cheat.globals.original_forwardmove < -5.0f)
					cmd->m_forwardmove = -max_move.x;

				if (cheat.globals.original_sidemove > 5.0f)
					cmd->m_sidemove = max_move.y;
				else if (cheat.globals.original_sidemove < -5.0f)
					cmd->m_sidemove = -max_move.y;
			}
			else
			{
				const auto speed = get_speed_move(cmd);
				cmd->m_forwardmove = speed.x;
				cmd->m_sidemove = speed.y;
			}
		}

		auto commands_to_add = 0;

		const auto curtime = m_globals()->m_curtime;

		while (commands_to_add != tickbase_shift)
		{
			auto sequence_number = commands_to_add + cmd->m_command_number;

			auto command = m_input()->GetUserCmd(sequence_number);
			auto verified_command = m_input()->GetVerifiedUserCmd(sequence_number);

			memcpy(command, cmd, sizeof(CUserCmd));

			if (command->m_tickcount != INT_MAX && m_clientstate()->m_iDeltaTick > 0)
				m_prediction()->Update(m_clientstate()->m_iDeltaTick, true, m_clientstate()->m_nLastCommandAck, m_clientstate()->m_nLastOutgoingCommand + m_clientstate()->m_iChokedCommands);

			command->m_command_number = sequence_number;
			command->m_predicted = command->m_tickcount != INT_MAX;

			//++m_clientstate()->m_iChokedCommands;
			//if (m_clientstate()->m_pNetChannel)
			//{
			//	++m_clientstate()->m_pNetChannel->m_nChokedPackets;
			//	++m_clientstate()->m_pNetChannel->m_nOutSequenceNr;
			//}

			if (m_clientstate()->m_pNetChannel && (commands_to_add + 1) < tickbase_shift) {
				++m_clientstate()->m_iChokedCommands;
				++m_clientstate()->m_pNetChannel->m_nChokedPackets;
				++m_clientstate()->m_pNetChannel->m_nOutSequenceNr;
			}

			math::normalize_angles(command->m_viewangles);

			memcpy(&verified_command->m_cmd, command, sizeof(CUserCmd));
			verified_command->m_crc = command->GetChecksum();

			++commands_to_add;
			//--cheat.globals.ticks_allowed;
		};

		m_globals()->m_curtime = curtime;

		m_prediction()->split->m_bPreviousAckHadErrors = true;
		m_prediction()->split->m_iCommandsPredicted = 0;
	}

	float get_interpolation()
	{
		static auto cl_interp = m_cvar()->FindVar(crypt_str("cl_interp"));
		static auto cl_interp_ratio = m_cvar()->FindVar(crypt_str("cl_interp_ratio"));
		static auto sv_client_min_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_min_interp_ratio"));
		static auto sv_client_max_interp_ratio = m_cvar()->FindVar(crypt_str("sv_client_max_interp_ratio"));
		static auto cl_updaterate = m_cvar()->FindVar(crypt_str("cl_updaterate"));
		static auto sv_minupdaterate = m_cvar()->FindVar(crypt_str("sv_minupdaterate"));
		static auto sv_maxupdaterate = m_cvar()->FindVar(crypt_str("sv_maxupdaterate"));

		auto ratio = std::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
		return max(cl_interp->GetFloat(), (ratio / ((sv_maxupdaterate) ? sv_maxupdaterate->GetFloat() : cl_updaterate->GetFloat())));

		//auto updaterate = math::clamp(cl_updaterate->GetFloat(), sv_minupdaterate->GetFloat(), sv_maxupdaterate->GetFloat());
		//auto lerp_ratio = math::clamp(cl_interp_ratio->GetFloat(), sv_client_min_interp_ratio->GetFloat(), sv_client_max_interp_ratio->GetFloat());
		//return math::clamp(lerp_ratio / updaterate, cl_interp->GetFloat(), 1.0f);
	}

	float server_time(CUserCmd* cmd) noexcept
	{
		static int tick;
		static CUserCmd* lastCmd;

		if (cmd) {
			if (cheat.local() && (!lastCmd || lastCmd->m_predicted))
				tick = cheat.globals.fixed_tickbase;
			else
				tick++;
			lastCmd = cmd;
		}
		return tick * m_globals()->m_intervalpertick;
	}

	bool isChatOpen()
	{
		if (!m_engine()->IsInGame())
			return false;

		const auto hudChat = memory->findHudElement(memory->hud, "CCSGO_HudChat");
		if (!hudChat)
			return false;

		return *reinterpret_cast<bool*>(uintptr_t(hudChat) + 0xD);
	}

	GameMode getGameMode()
	{
		static auto gameType{ m_cvar()->FindVar("game_type")};
		static auto gameMode{ m_cvar()->FindVar("game_mode") };
		switch (gameType->GetInt())
		{
		case 0:
			switch (gameMode->GetInt())
			{
			case 0:
				return GameMode::Casual;
			case 1:
				return GameMode::Competitive;
			case 2:
				return GameMode::Wingman;
			case 3:
				return GameMode::WeaponsExpert;
			default:
				break;
			}
			break;
		case 1:
			switch (gameMode->GetInt())
			{
			case 0:
				return GameMode::ArmsRace;
			case 1:
				return GameMode::Demolition;
			case 2:
				return GameMode::Deathmatch;
			default:
				break;
			}
			break;
		case 2:
			return GameMode::Training;
		case 3:
			return GameMode::Custom;
		case 4:
			switch (gameMode->GetInt())
			{
			case 0:
				return GameMode::Guardian;
			case 1:
				return GameMode::CoopStrike;
			default:
				break;
			}
			break;
		case 5:
			return GameMode::WarGames;
		case 6:
			return GameMode::DangerZone;
		default:
			break;
		}
		return GameMode::None;
	}
}