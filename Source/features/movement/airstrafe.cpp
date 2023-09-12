#include "airstrafe.h"
#include "..\serverside\engine_prediction.h"

#define CheckIfNonValidNumber(x) (fpclassify(x) == FP_INFINITE || fpclassify(x) == FP_NAN || fpclassify(x) == FP_SUBNORMAL)

void airstrafe::create_move(CUserCmd* m_pcmd, Vector& currentViewAngles)
{
	if (localPlayer->m_fFlags() & FL_ONGROUND || engineprediction::get().backup_data.flags & FL_ONGROUND || localPlayer->get_move_type() == MOVETYPE_LADDER || localPlayer->get_move_type() == MOVETYPE_NOCLIP)
		return;

	static auto cl_sidespeed = m_cvar()->FindVar(crypt_str("cl_sidespeed"));
	auto side_speed = cl_sidespeed->GetFloat();

	if (configs.misc.airstrafe == 1)
	{
		if (m_pcmd->m_mousedx < 0)
			m_pcmd->m_sidemove = -450.0f;
		else if (m_pcmd->m_mousedx > 0)
			m_pcmd->m_sidemove = 450.0f;
	}
	else if (configs.misc.airstrafe == 2)
	{
		auto getVelocityDegree = [](float velocity)
		{
			auto degree = RAD2DEG(atan(30.f / velocity));

			if (CheckIfNonValidNumber(degree) || degree > 90.f)
				return 90.f;
			else if (degree < .0f)
				return .0f;
			else
				return degree;
		};
		auto& velocity = localPlayer->m_vecVelocity();
		velocity.z = .0f;
		static auto oldYaw = .0f;
		auto forwardMove = m_pcmd->m_forwardmove;
		auto sideMove = m_pcmd->m_sidemove;

		if (velocity.Length2D() < 5.f && !forwardMove && !sideMove)
			return;

		static auto flip = false;
		flip = !flip;
		auto turnDirectionModifier = flip ? 1.f : -1.f;
		auto viewAngles = m_pcmd->m_viewangles;

		if (forwardMove || sideMove)
		{
			m_pcmd->m_forwardmove = .0f;
			m_pcmd->m_sidemove = .0f;
			auto turnAngle = atan2(-sideMove, forwardMove);
			viewAngles.y += turnAngle * 57.295779513082f;
		}
		else if (forwardMove)
			m_pcmd->m_forwardmove = .0f;

		auto strafeAngle = RAD2DEG(atan(15.f / velocity.Length2D()));

		if (strafeAngle > 90.f)
			strafeAngle = 90.f;
		else if (strafeAngle < .0f)
			strafeAngle = .0f;

		auto orientation = Vector(.0f, viewAngles.y - oldYaw, .0f);
		orientation.y = math::normalize_yaw(orientation.y);
		auto yawDelta = orientation.y;
		oldYaw = viewAngles.y;
		auto FABSYawDelta = fabs(yawDelta);

		if (FABSYawDelta <= strafeAngle || FABSYawDelta >= 30.f)
		{
			Vector VelocityAngles;

			math::vector_angles(velocity, VelocityAngles);

			orientation = Vector(.0F, viewAngles.y - VelocityAngles.y, .0f);
			orientation.y = math::normalize_yaw(orientation.y);
			auto velocityAngleYawDelta = orientation.y;
			auto velocityDegree = getVelocityDegree(velocity.Length2D());

			if (velocityAngleYawDelta <= velocityDegree || velocity.Length2D() <= 15.f)
			{
				if (-velocityDegree <= velocityAngleYawDelta || velocity.Length2D() <= 15.f)
				{
					viewAngles.y += strafeAngle * turnDirectionModifier;
					m_pcmd->m_sidemove = cl_sidespeed->GetFloat() * turnDirectionModifier;
				}
				else
				{
					viewAngles.y = VelocityAngles.y - velocityDegree;
					m_pcmd->m_sidemove = cl_sidespeed->GetFloat();
				}
			}
			else
			{
				viewAngles.y = VelocityAngles.y + velocityDegree;
				m_pcmd->m_sidemove = -cl_sidespeed->GetFloat();
			}
		}
		else if (yawDelta > .0f)
			m_pcmd->m_sidemove = -cl_sidespeed->GetFloat();
		else if (yawDelta < .0f)
			m_pcmd->m_sidemove = cl_sidespeed->GetFloat();

		auto move = Vector(m_pcmd->m_forwardmove, m_pcmd->m_sidemove, .0f);
		auto speed = move.Length();
		Vector moveAngles;

		math::vector_angles(move, moveAngles);

		auto normalizedX = fmod(m_pcmd->m_viewangles.x + 180.f, 360.f) - 180.f;
		auto normalizedY = fmod(m_pcmd->m_viewangles.y + 180.f, 360.f) - 180.f;
		auto yaw = DEG2RAD(normalizedY - viewAngles.y + moveAngles.y);

		if (normalizedX >= 90.f || normalizedX <= -90.f || m_pcmd->m_viewangles.x >= 90.f && m_pcmd->m_viewangles.x <= 200.f || m_pcmd->m_viewangles.x <= -90.f && m_pcmd->m_viewangles.x <= 200.f)
			m_pcmd->m_forwardmove = -cos(yaw) * speed;
		else
			m_pcmd->m_forwardmove = cos(yaw) * speed;

		m_pcmd->m_sidemove = sin(yaw) * speed;
	}
	else if (configs.misc.airstrafe == 3)
	{
	/*
		static float yaw_add = 0.f;

		bool back = m_pcmd->m_buttons & IN_BACK;
		bool forward = m_pcmd->m_buttons & IN_FORWARD;
		bool right = m_pcmd->m_buttons & IN_MOVELEFT;
		bool left = m_pcmd->m_buttons & IN_MOVERIGHT;

		if (back) {
			yaw_add = -180.f;
			if (right)
				yaw_add -= 45.f;
			else if (left)
				yaw_add += 45.f;
		}
		else if (right) {
			yaw_add = 90.f;
			if (back)
				yaw_add += 45.f;
			else if (forward)
			yaw_add -= 45.f;
		}
		else if (left) {
			yaw_add = -90.f;
			if (back)
				yaw_add -= 45.f;
			else if (forward)
				yaw_add += 45.f;
		}
		else {
			yaw_add = 0.f;
		}

		auto cva = cheat.globals.original;

		cva.y += yaw_add;
		m_pcmd->m_forwardmove = 0.f;
		m_pcmd->m_sidemove = 0.f;

		const auto delta = math::normalize_yaw(cva.y - RAD2DEG(atan2(localPlayer->m_vecVelocity().y, localPlayer->m_vecVelocity().x)));

		m_pcmd->m_sidemove = delta > 0.f ? -cl_sidespeed->GetFloat() : cl_sidespeed->GetFloat();

		//cva.y = math::normalize_yaw(cva.y - delta);
		*/
		static auto old_yaw = 0.0f;

		auto get_velocity_degree = [](float velocity)
		{
			auto tmp = RAD2DEG(atan(30.0f / velocity));

			if (CheckIfNonValidNumber(tmp) || tmp > 90.0f)
				return 90.0f;

			else if (tmp < 0.0f)
				return 0.0f;
			else
				return tmp;
		};

		if (localPlayer->get_move_type() != MOVETYPE_WALK)
			return;

		auto velocity = localPlayer->m_vecVelocity();
		velocity.z = 0.0f;

		auto forwardmove = m_pcmd->m_forwardmove;
		auto sidemove = m_pcmd->m_sidemove;

		if (velocity.Length2D() < 5.0f && !forwardmove && !sidemove)
			return;

		static auto flip = false;
		flip = !flip;

		auto turn_direction_modifier = flip ? 1.0f : -1.0f;
		auto viewangles = m_pcmd->m_viewangles;

		if (forwardmove || sidemove)
		{
			m_pcmd->m_forwardmove = 0.0f;
			m_pcmd->m_sidemove = 0.0f;

			auto turn_angle = atan2(-sidemove, forwardmove);
			viewangles.y += turn_angle * 57.295779513082f;
		}
		else if (forwardmove)
			m_pcmd->m_forwardmove = 0.0f;

		auto strafe_angle = RAD2DEG(atan(15.0f / velocity.Length2D()));

		if (strafe_angle > 90.0f)
			strafe_angle = 90.0f;
		else if (strafe_angle < 0.0f)
			strafe_angle = 0.0f;

		auto temp = Vector(0.0f, viewangles.y - old_yaw, 0.0f);
		temp.y = math::normalize_yaw(temp.y);

		auto yaw_delta = temp.y;
		old_yaw = viewangles.y;

		auto abs_yaw_delta = fabs(yaw_delta);

		if (abs_yaw_delta <= strafe_angle || abs_yaw_delta >= 15.0f) // whatsapp turbo ativado
		{
			Vector velocity_angles;
			math::vector_angles(velocity, velocity_angles);

			temp = Vector(0.0f, viewangles.y - velocity_angles.y, 0.0f);
			temp.y = math::normalize_yaw(temp.y);

			auto velocityangle_yawdelta = temp.y;
			auto velocity_degree = get_velocity_degree(velocity.Length2D());

			if (velocityangle_yawdelta <= velocity_degree || velocity.Length2D() <= 15.0f)
			{
				if (-velocity_degree <= velocityangle_yawdelta || velocity.Length2D() <= 15.0f)
				{
					viewangles.y += strafe_angle * turn_direction_modifier;
					m_pcmd->m_sidemove = side_speed * turn_direction_modifier;
				}
				else
				{
					viewangles.y = velocity_angles.y - velocity_degree;
					m_pcmd->m_sidemove = side_speed;
				}
			}
			else
			{
				viewangles.y = velocity_angles.y + velocity_degree;
				m_pcmd->m_sidemove = -side_speed;
			}
		}
		else if (yaw_delta > 0.0f)
			m_pcmd->m_sidemove = -side_speed;
		else if (yaw_delta < 0.0f)
			m_pcmd->m_sidemove = side_speed;

		auto move = Vector(m_pcmd->m_forwardmove, m_pcmd->m_sidemove, 0.0f);
		auto speed = move.Length();

		Vector angles_move;
		math::vector_angles(move, angles_move);

		auto normalized_x = fmod(m_pcmd->m_viewangles.x + 180.0f, 360.0f) - 180.0f;
		auto normalized_y = fmod(m_pcmd->m_viewangles.y + 180.0f, 360.0f) - 180.0f;

		auto yaw = DEG2RAD(normalized_y - viewangles.y + angles_move.y);

		if (normalized_x >= 90.0f || normalized_x <= -90.0f || m_pcmd->m_viewangles.x >= 90.0f && m_pcmd->m_viewangles.x <= 200.0f || m_pcmd->m_viewangles.x <= -90.0f && m_pcmd->m_viewangles.x <= 200.0f)
			m_pcmd->m_forwardmove = -cos(yaw) * speed;
		else
			m_pcmd->m_forwardmove = cos(yaw) * speed;

		m_pcmd->m_sidemove = sin(yaw) * speed;
	}
}