#include <mutex>

#include "grenade_prediction.h"

std::vector<std::pair<Vector, Vector>> screenPoints;
std::vector<std::pair<Vector, Vector>> endPoints;
std::vector<std::pair<Vector, Vector>> savedPoints;

std::mutex renderMutex;

int grenade_act{ 1 };

void setup(Vector& vecSrc, Vector& vecThrow, Vector viewangles)
{
	auto AngleVectors = [](const Vector& angles, Vector* forward, Vector* right, Vector* up)
	{
		float sr, sp, sy, cr, cp, cy;

		sp = static_cast<float>(sin(double(angles.x) * 0.01745329251f));
		cp = static_cast<float>(cos(double(angles.x) * 0.01745329251f));
		sy = static_cast<float>(sin(double(angles.y) * 0.01745329251f));
		cy = static_cast<float>(cos(double(angles.y) * 0.01745329251f));
		sr = static_cast<float>(sin(double(angles.z) * 0.01745329251f));
		cr = static_cast<float>(cos(double(angles.z) * 0.01745329251f));

		if (forward)
		{
			forward->x = cp * cy;
			forward->y = cp * sy;
			forward->z = -sp;
		}

		if (right)
		{
			right->x = (-1 * sr * sp * cy + -1 * cr * -sy);
			right->y = (-1 * sr * sp * sy + -1 * cr * cy);
			right->z = -1 * sr * cp;
		}

		if (up)
		{
			up->x = (cr * sp * cy + -sr * -sy);
			up->y = (cr * sp * sy + -sr * cy);
			up->z = cr * cp;
		}
	};
	Vector angThrow = viewangles;
	float pitch = angThrow.x;

	if (pitch <= 90.0f)
	{
		if (pitch < -90.0f)
		{
			pitch += 360.0f;
		}
	}
	else
	{
		pitch -= 360.0f;
	}

	float a = pitch - (90.0f - fabs(pitch)) * 10.0f / 90.0f;
	angThrow.x = a;

	float flVel = 750.0f * 0.9f;

	static const float power[] = { 1.0f, 1.0f, 0.5f, 0.0f };
	float b = power[grenade_act];
	b = b * 0.7f;
	b = b + 0.3f;
	flVel *= b;

	Vector vForward, vRight, vUp;
	AngleVectors(angThrow, &vForward, &vRight, &vUp);

	vecSrc = cheat.local()->get_eye_position();
	float off = (power[grenade_act] * 12.0f) - 12.0f;
	vecSrc.z += off;

	CGameTrace tr;
	Vector vecDest = vecSrc;
	vecDest += vForward * 22.0f;

	m_trace()->trace_hull(vecSrc, vecDest, 0x200400B, tr);

	Vector vecBack = vForward; vecBack *= 6.0f;
	vecSrc = tr.endpos;
	vecSrc -= vecBack;

	vecThrow = cheat.local()->m_vecVelocity(); vecThrow *= 1.25f;
	vecThrow += vForward * flVel;
}

int physicsClipVelocity(const Vector& in, const Vector& normal, Vector& out, float overbounce)
{
	static const float STOP_EPSILON = 0.1f;

	float backoff;
	float change;
	float angle;
	int i, blocked;

	blocked = 0;

	angle = normal.z;

	if (angle > 0)
	{
		blocked |= 1; // floor
	}
	if (!angle)
	{
		blocked |= 2; // step
	}

	backoff = in.Dot(normal) * overbounce;

	for (i = 0; i < 3; i++)
	{
		switch (i)
		{
		case 0:
			change = normal.x * backoff;
			out.x = in.x - change;
			if (out.x > -STOP_EPSILON && out.x < STOP_EPSILON)
			{
				out.x = 0;
			}
			break;
		case 1:
			change = normal.y * backoff;
			out.y = in.y - change;
			if (out.y > -STOP_EPSILON && out.y < STOP_EPSILON)
			{
				out.y = 0;
			}
			break;
		case 2:
			change = normal.z * backoff;
			out.z = in.z - change;
			if (out.z > -STOP_EPSILON && out.z < STOP_EPSILON)
			{
				out.z = 0;
			}
			break;
		}
	}

	return blocked;
}

void pushEntity(Vector& src, const Vector& move, CGameTrace& tr)
{
	Vector vecAbsEnd = src;
	vecAbsEnd += move;
	m_trace()->trace_hull(src, vecAbsEnd, 0x200400B, tr);
}

void resolveFlyCollisionCustom(CGameTrace& tr, Vector& vecVelocity, float interval)
{
	// Calculate elasticity
	float flSurfaceElasticity = 1.0;
	float flGrenadeElasticity = 0.45f;
	float flTotalElasticity = flGrenadeElasticity * flSurfaceElasticity;
	if (flTotalElasticity > 0.9f) flTotalElasticity = 0.9f;
	if (flTotalElasticity < 0.0f) flTotalElasticity = 0.0f;

	// Calculate bounce
	Vector vecAbsVelocity;
	physicsClipVelocity(vecVelocity, tr.plane.normal, vecAbsVelocity, 2.0f);
	vecAbsVelocity *= flTotalElasticity;

	float flSpeedSqr = vecAbsVelocity.LengthSqr();
	static const float flMinSpeedSqr = 20.0f * 20.0f;

	if (flSpeedSqr < flMinSpeedSqr)
	{
		vecAbsVelocity.x = 0.0f;
		vecAbsVelocity.y = 0.0f;
		vecAbsVelocity.z = 0.0f;
	}

	if (tr.plane.normal.z > 0.7f)
	{
		vecVelocity = vecAbsVelocity;
		vecAbsVelocity *= ((1.0f - tr.fraction) * interval);
		pushEntity(tr.endpos, vecAbsVelocity, tr);
	}
	else
	{
		vecVelocity = vecAbsVelocity;
	}
}

void addGravityMove(Vector& move, Vector& vel, float frametime, bool onground)
{
	Vector basevel{ 0.0f, 0.0f, 0.0f };

	move.x = (vel.x + basevel.x) * frametime;
	move.y = (vel.y + basevel.y) * frametime;

	if (onground)
	{
		move.z = (vel.z + basevel.z) * frametime;
	}
	else
	{
		float gravity = 800.0f * 0.4f;
		float newZ = vel.z - (gravity * frametime);
		move.z = ((vel.z + newZ) / 2.0f + basevel.z) * frametime;
		vel.z = newZ;
	}
}

enum ACT
{
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP
};

void tick(int buttons)
{
	bool in_attack = (buttons & IN_ATTACK || !configs.visuals.grenade_prediction_click_only);
	bool in_attack2 = buttons & IN_ATTACK2;

	grenade_act = (in_attack && in_attack2) ? ACT_LOB :
		(in_attack2) ? ACT_DROP :
		(in_attack) ? ACT_THROW :
		ACT_NONE;
}

bool checkDetonate(const Vector& vecThrow, const CGameTrace& tr, int tick, float interval, weapon_t* activeWeapon)
{
	switch (activeWeapon->m_iItemDefinitionIndex())
	{
	case WEAPON_SMOKEGRENADE:
	case WEAPON_DECOY:
		if (vecThrow.Length2D() < 0.1f)
		{
			int det_tick_mod = (int)(0.2f / interval);
			return !(tick % det_tick_mod);
		}
		return false;
	case WEAPON_MOLOTOV:
	case WEAPON_INCGRENADE:
		if (tr.fraction != 1.0f && tr.plane.normal.z > 0.7f)
			return true;
		return false;
	case WEAPON_FLASHBANG:
	case WEAPON_HEGRENADE:
		return (float)tick * interval > 1.5f && !(tick % (int)(0.2f / interval));
	default:
		return false;
	}
}

void drawCircle(Vector position, float points, float radius)
{
	float step = 3.141592654f * 2.0f / points;
	Vector end2d{}, start2d{};
	Vector lastPos{};
	for (float a = -step; a < 3.141592654f * 2.0f; a += step) {
		Vector start{ radius * cosf(a) + position.x, radius * sinf(a) + position.y, position.z };

		CGameTrace tr;
		m_trace()->trace_hull(position, start, 0x200400B, tr);
		if (tr.endpos.IsZero())
			continue;

		if (math::world_to_screen_pixel_aligned(tr.endpos, start2d) && math::world_to_screen_pixel_aligned(lastPos, end2d) && lastPos != ZERO)
		{
			if (start2d.x != 0.f && start2d.y != 0.f && end2d.x != 0.f && end2d.y != 0.f)
				endPoints.emplace_back(std::pair<Vector, Vector>{ end2d, start2d });
		}
		lastPos = tr.endpos;
	}
}

void grenade_prediction::run(CUserCmd* cmd)
{
	renderMutex.lock();
	screenPoints.clear();
	endPoints.clear();

	if (!configs.visuals.grenade_prediction || !cheat.local() || !cheat.local()->is_alive() || !m_engine()->IsInGame())
	{
		renderMutex.unlock();
		return;
	}

	tick(cmd->m_buttons);

	auto activeWeapon = cheat.globals.weapon;
	if (!activeWeapon || !activeWeapon->is_normal_grenade() || (!activeWeapon->m_bPinPulled() && !configs.visuals.grenade_prediction_click_only))
	{
		renderMutex.unlock();
		return;
	}

	Vector vecSrc, vecThrow;
	setup(vecSrc, vecThrow, cmd->m_viewangles);

	float interval = m_globals()->m_intervalpertick;
	int logstep = static_cast<int>(0.05f / interval);
	int logtimer = 0;

	std::vector<Vector> path;

	for (int i = 0; i < path.max_size() - 1; ++i)
	{
		if (!logtimer)
			path.emplace_back(vecSrc);

		Vector move;
		addGravityMove(move, vecThrow, interval, false);

		// Push entity
		CGameTrace tr;
		pushEntity(vecSrc, move, tr);

		int result = 0;
		if (checkDetonate(vecThrow, tr, i, interval, activeWeapon))
			result |= 1;

		if (tr.fraction != 1.0f)
		{
			result |= 2; // Collision!
			resolveFlyCollisionCustom(tr, vecThrow, interval);
		}

		vecSrc = tr.endpos;

		if (result & 1)
			break;

		if ((result & 2) || logtimer >= logstep)
			logtimer = 0;
		else
			++logtimer;
	}

	path.emplace_back(vecSrc);

	Vector prev = path[0];
	Vector nadeStart, nadeEnd;
	Vector lastPos{ };
	for (auto& nade : path)
	{
		if (math::world_to_screen_pixel_aligned(prev, nadeStart) && math::world_to_screen_pixel_aligned(nade, nadeEnd))
		{
			screenPoints.emplace_back(std::pair<Vector, Vector>{ nadeStart, nadeEnd });
			prev = nade;
			lastPos = nade;
		}
	}

	if (!lastPos.IsZero())
		drawCircle(lastPos, 120, 150);

	renderMutex.unlock();
}

void grenade_prediction::draw()
{
	if (!configs.visuals.grenade_prediction || !cheat.local() || !cheat.local()->is_alive() || !m_engine()->IsInGame())
		return;

	if (renderMutex.try_lock())
	{
		savedPoints = screenPoints;
		renderMutex.unlock();
	}

	// draw end nade path
	if (!endPoints.empty())
		for (auto& point : endPoints)
		{
			imRender->DrawLine(point.first.x, point.first.y, point.second.x, point.second.y, configs.visuals.grenade_prediction_color);
			imRender->DrawLine(point.first.x - 0.5, point.first.y - 0.5, point.second.x - 0.5, point.second.y - 0.5, configs.visuals.grenade_prediction_color);
			imRender->DrawLine(point.first.x - 1, point.first.y - 1, point.second.x - 1, point.second.y - 1, configs.visuals.grenade_prediction_color);
			imRender->DrawLine(point.first.x - 1.5, point.first.y - 1.5, point.second.x - 1.5, point.second.y - 1.5, configs.visuals.grenade_prediction_color);
			imRender->DrawLine(point.first.x - 2, point.first.y - 2, point.second.x - 2, point.second.y - 2, configs.visuals.grenade_prediction_color);
		}

	// draw nade path
	if (!savedPoints.empty())
		for (auto& point : savedPoints)
		{
			imRender->DrawLine(point.first.x, point.first.y, point.second.x, point.second.y, configs.visuals.grenade_prediction_tracer_color);
			imRender->DrawLine(point.first.x - 0.5, point.first.y - 0.5, point.second.x - 0.5, point.second.y - 0.5, configs.visuals.grenade_prediction_tracer_color);
			imRender->DrawLine(point.first.x - 1, point.first.y - 1, point.second.x - 1, point.second.y - 1, configs.visuals.grenade_prediction_tracer_color);
		}
}
