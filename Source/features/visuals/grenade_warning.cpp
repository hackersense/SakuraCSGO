#include "grenade_warning.h"
#include "world_esp.h"
#include "..\menu.h"

const char* index_to_grenade_name(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE:
		return "smoke";
		break;
	case WEAPON_HEGRENADE:
		return "he grenade";
		break;
	case WEAPON_MOLOTOV:
		return "molotov";
		break;
	case WEAPON_INCGRENADE:
		return "molotov";
		break;
	}
}
const char* index_to_grenade_name_icon(int index)
{
	switch (index)
	{
	case WEAPON_SMOKEGRENADE:
		return "k";
		break;
	case WEAPON_HEGRENADE:
		return "j";
		break;
	case WEAPON_MOLOTOV:
		return "l";
		break;
	case WEAPON_INCGRENADE:
		return "n";
		break;
	}
}

bool c_grenade_prediction::data_t::draw()
{
	if (m_path.size() <= 1 || m_globals()->m_curtime >= m_expire_time)
		return false;

	float distance = cheat.local()->m_vecOrigin().DistTo(m_origin) / 12.f;

	if (distance > 200.f)
		return false;

	static int iScreenWidth, iScreenHeight;
	m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

	auto prev_screen = ZERO;
	auto prev_on_screen = math::world_to_screen(std::get< Vector >(m_path.front()), prev_screen);

	for (int i = 1; i < m_path.size(); ++i) {
		auto cur_screen = ZERO, last_cur_screen = ZERO;
		const auto cur_on_screen = math::world_to_screen(std::get< Vector >(m_path.at(i)), cur_screen);

		if (prev_on_screen && cur_on_screen)
		{
			auto color = configs.visuals.grenade_warning_tracer_color;
			if (i % 4)
				imRender->DrawLine((int)prev_screen.x, (int)prev_screen.y, (int)cur_screen.x, (int)cur_screen.y, color, 2);
		}

		prev_screen = cur_screen;
		prev_on_screen = cur_on_screen;
	}

	float percent = ((m_expire_time - m_globals()->m_curtime) / TICKS_TO_TIME(m_tick));
	int alpha_damage = 0;

	if (m_index == WEAPON_HEGRENADE && distance <= 20)
	{
		alpha_damage = 255 - 255 * (distance / 20);
	}

	if ((m_index == WEAPON_MOLOTOV || m_index == WEAPON_INCGRENADE) && distance <= 15)
	{
		alpha_damage = 255 - 255 * (distance / 15);
	}

	auto col = configs.visuals.grenade_warning_color;
	Ray_t ray;
	CTraceFilterWorldAndPropsOnly filter;
	CGameTrace trace;

	ray.Init(m_origin, m_origin - Vector(0.0f, 0.0f, 105.0f));
	m_trace()->TraceRay(ray, MASK_SOLID, &filter, &trace);

	if (trace.fraction < 1.0f && m_index == 46) {
		auto pos = trace.endpos + Vector(0.0f, 0.0f, 2.0f);
		imRender->DrawRing3D(pos.x, pos.y, pos.z, 90, 140, Color(col.r(), col.g(), col.b(), 255), Color(140, 140, 140, 35), 2, percent);
		static auto g_size = Vector2D(35.0f, 5.0f);
		Vector screen;
		if (math::world_to_screen(pos, screen))
			imRender->CircleFilled(screen.x, screen.y - g_size.y * 0.5f - 12, 21, Color(25, 25, 25, col.a()), 60);
		imRender->CircleFilled(screen.x, screen.y - g_size.y * 0.5f - 12, 21, Color(200, 25, 25, alpha_damage), 60);
		imRender->two_sided_arc(screen.x, screen.y - g_size.y * 0.5f - 12, 20, percent, Color(col.r(), col.g(), col.b(), 255), 3);
		imRender->DrawString(screen.x, screen.y - g_size.y * 0.5f - 12, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, c_menu::get().weapon_icon2, index_to_grenade_name_icon(m_index));
	}
	else
	{

		static auto g_size = Vector2D(35.0f, 5.0f);
		imRender->CircleFilled(prev_screen.x, prev_screen.y - g_size.y * 0.5f - 12, 21, Color(25, 25, 25, col.a()), 60);
		imRender->CircleFilled(prev_screen.x, prev_screen.y - g_size.y * 0.5f - 12, 21, Color(200, 25, 25, alpha_damage), 60);
		imRender->two_sided_arc(prev_screen.x, prev_screen.y - g_size.y * 0.5f - 12, 20, percent, Color(col.r(), col.g(), col.b(), 255), 3);
		imRender->DrawString(prev_screen.x, prev_screen.y - g_size.y * 0.5f - 12, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, c_menu::get().weapon_icon2, index_to_grenade_name_icon(m_index));
	}

	auto isOnScreen = [](Vector origin, Vector& screen) -> bool
	{
		if (!math::world_to_screen(origin, screen))
			return false;

		static int iScreenWidth, iScreenHeight;
		m_engine()->GetScreenSize(iScreenWidth, iScreenHeight);

		auto xOk = iScreenWidth > screen.x;
		auto yOk = iScreenHeight > screen.y;

		return xOk && yOk;
	};

	Vector screenPos;
	Vector vEnemyOrigin = m_origin;
	Vector vLocalOrigin = cheat.local()->GetAbsOrigin();

	if (!cheat.local()->is_alive())
		vLocalOrigin = m_input()->m_vecCameraOffset;

	Vector viewAngles;
	m_engine()->GetViewAngles(viewAngles);

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	if (configs.visuals.grenade_warning_offscreen && !isOnScreen(vEnemyOrigin, screenPos))
	{
		auto screenCenter = Vector2D(width * 0.5f, height * 0.5f);
		auto angleYawRad = DEG2RAD(viewAngles.y - math::calculate_angle(cheat.globals.eye_pos, vEnemyOrigin).y - 90.0f);

		auto radius = 80 - 70 * float(alpha_damage / 255.f);
		auto size = 6 + 6 * float(alpha_damage / 255.f);

		auto newPointX = screenCenter.x + ((((width - (size * 3)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
		auto newPointY = screenCenter.y + ((((height - (size * 3)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));
		auto newPointX2 = screenCenter.x + ((((width - (size * 15)) * 0.5f) * (radius / 100.0f)) * cos(angleYawRad)) + (int)(6.0f * (((float)size - 4.0f) / 16.0f));
		auto newPointY2 = screenCenter.y + ((((height - (size * 15)) * 0.5f) * (radius / 100.0f)) * sin(angleYawRad));
		std::array <Vector2D, 3> points
		{
			Vector2D(newPointX - size, newPointY - size),
				Vector2D(newPointX + size, newPointY),
				Vector2D(newPointX - size, newPointY + size)
		};

		auto warn = Vector2D(newPointX2, newPointY2);
		math::rotate_triangle(points, viewAngles.y - math::calculate_angle(cheat.globals.eye_pos, vEnemyOrigin).y - 90.0f);

		imRender->CircleFilled(warn.x, warn.y - (size * 3) * 0.5f, 21, Color(25, 25, 25, col.a()), 60);
		imRender->CircleFilled(warn.x, warn.y - (size * 3) * 0.5f, 21, Color(200, 25, 25, alpha_damage), 60);
		imRender->two_sided_arc(warn.x, warn.y - (size * 3) * 0.5f, 20, percent, Color(col.r(), col.g(), col.b(), 255), 3);
		imRender->DrawString(warn.x, warn.y - (size * 3) * 0.5f, Color::White, HFONT_CENTERED_X | HFONT_CENTERED_Y, c_menu::get().weapon_icon2, index_to_grenade_name_icon(m_index));
		imRender->TriangleFilled(points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, Color(25, 25, 25, 150));
		imRender->Triangle(points.at(0).x, points.at(0).y, points.at(1).x, points.at(1).y, points.at(2).x, points.at(2).y, Color(col.r(), col.g(), col.b(), 255));
	}
	return true;
}

void c_grenade_prediction::run(entity_t* ent)
{
	auto client_class = ent->GetClientClass();

	if (!client_class)
		return;

	auto model = ent->GetModel();

	if (!model)
		return;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return;

	auto name = std::string(studio_model->name);

	if (name.find("thrown") != std::string::npos || client_class->m_ClassID == CBaseCSGrenadeProjectile || client_class->m_ClassID == CDecoyProjectile || client_class->m_ClassID == CMolotovProjectile)
	{
		auto entity = reinterpret_cast<projectile_t*>(ent);
		auto& predicted_nades = c_grenade_prediction::get().get_list();

		static auto last_server_tick = m_globals()->m_curtime;
		if (last_server_tick != m_globals()->m_curtime)
		{
			predicted_nades.clear();

			last_server_tick = m_globals()->m_curtime;
		}

		if (entity->IsDormant() || !configs.visuals.grenade_warning)
			return;

		const auto client_class = entity->GetClientClass();
		if (!client_class || client_class->m_ClassID != CMolotovProjectile && client_class->m_ClassID != CBaseCSGrenadeProjectile)
			return;

		if (client_class->m_ClassID == CBaseCSGrenadeProjectile) {
			const auto model = entity->GetModel();
			if (!model)
				return;

			const auto studio_model = m_modelinfo()->GetStudioModel(model);
			if (!studio_model || std::string_view(studio_model->name).find("fraggrenade") == std::string::npos)
				return;
		}

		const auto handle = entity->GetRefEHandle().ToLong();

		if (entity->m_nExplodeEffectTickBegin() || !entity->m_hThrower().IsValid() || (entity->m_hThrower().Get()->m_iTeamNum() == cheat.local()->m_iTeamNum() && entity->m_hThrower().Get() != cheat.local())) {
			predicted_nades.erase(handle);

			return;
		}

		if (predicted_nades.find(handle) == predicted_nades.end()) {
			predicted_nades.emplace(
				std::piecewise_construct,
				std::forward_as_tuple(handle),
				std::forward_as_tuple(
					entity->m_hThrower().Get(),
					client_class->m_ClassID == CMolotovProjectile ? WEAPON_MOLOTOV : WEAPON_HEGRENADE,
					entity->m_vecOrigin(), reinterpret_cast<player_t*>(entity)->m_vecVelocity(),
					entity->m_flSpawnTime(), TIME_TO_TICKS(reinterpret_cast<player_t*>(entity)->m_flSimulationTime() - entity->m_flSpawnTime())
				)
			);
		}

		if (predicted_nades.at(handle).draw())
			return;

		predicted_nades.erase(handle);
	}
}