#include "other_esp.h"
#include "..\ragebot\autowall.h"
#include "..\ragebot\antiaim.h"
#include "..\misc\logs.h"
#include "..\misc\misc.h"
#include "..\serverside\local_animations.h"

bool can_penetrate(weapon_t* weapon)
{
	auto weapon_info = weapon->get_weapon_info();

	if (!weapon_info)
		return false;

	Vector view_angles;
	m_engine()->GetViewAngles(view_angles);

	Vector direction;
	math::angle_vectors(view_angles, direction);

	CGameTrace trace;
	m_trace()->trace_entity(cheat.globals.eye_pos, (cheat.globals.eye_pos + direction * weapon_info->flRange), MASK_SHOT_HULL | CONTENTS_HITBOX, cheat.local(), &trace);

	if (trace.fraction == 1.0f)
		return false;

	auto eye_pos = cheat.globals.eye_pos;
	auto hits = 1;
	auto damage = static_cast<float>(weapon_info->iDamage);
	auto penetration_power = weapon_info->flPenetration;

	static auto damageReductionBullets = m_cvar()->FindVar(crypt_str("ff_damage_reduction_bullets"));
	static auto damageBulletPenetration = m_cvar()->FindVar(crypt_str("ff_damage_bullet_penetration"));

	return autowall::get().handle_bullet_penetration(weapon_info, trace, eye_pos, direction, hits, damage, penetration_power, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat());
}

void otheresp::penetration_reticle()
{
	if (!configs.esp.enabled)
		return;

	if (!configs.visuals.penetration_reticle)
		return;

	if (!cheat.local()->is_alive())
		return;

	auto weapon = cheat.globals.weapon;

	if (!weapon)
		return;

	auto color = Color::Red;

	if (!weapon->is_non_aim() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && can_penetrate(weapon))
		color = Color::Green;

	static int width, height;
	m_engine()->GetScreenSize(width, height);
	
	render::get().rect_filled(width / 2, height / 2 - 1, 1, 3, color);
	render::get().rect_filled(width / 2 - 1, height / 2, 3, 1, color);
}

void otheresp::indicators()
{
	if (!cheat.local()->is_alive())
		return;

	auto weapon = cheat.globals.weapon;

	if (!weapon)
		return;

	if (configs.visuals.indicators[INDICATOR_FAKE] && (antiaim::get().type == ANTIAIM_LEGIT || configs.antiaim.type[antiaim::get().type].desync))
	{
		auto color = Color(130, 20, 20);
		auto animstate = cheat.local()->get_animation_state();

		if (animstate && local_animations::get().local_data.animstate)
		{
			auto delta = fabs(math::normalize_yaw(animstate->m_flGoalFeetYaw - local_animations::get().local_data.animstate->m_flGoalFeetYaw));
			auto desync_delta = max(cheat.local()->get_max_desync_delta(), 58.0f);

			color = Color(130, 20 + (int)(min(delta / desync_delta, 1.0f) * 150.0f), 20);
		}

		m_indicators.push_back(m_indicator("FAKE", color));
	}

	if (configs.visuals.indicators[INDICATOR_DESYNC_SIDE] && (antiaim::get().type == ANTIAIM_LEGIT && configs.antiaim.desync == 1 || antiaim::get().type != ANTIAIM_LEGIT && configs.antiaim.type[antiaim::get().type].desync == 1) && !antiaim::get().condition(cheat.get_command()))
	{
		auto side = antiaim::get().desync_angle > 0.0f ? "RIGHT" : "LEFT";

		if (antiaim::get().type == ANTIAIM_LEGIT)
			side = antiaim::get().desync_angle > 0.0f ? "LEFT" : "RIGHT";

		m_indicators.push_back(m_indicator(side, Color(130, 170, 20)));
	}

	auto choke_indicator = false;

	if (configs.visuals.indicators[INDICATOR_CHOKE] && !fakelag::get().condition && !misc::get().double_tap_enabled && !misc::get().hide_shots_enabled)
	{
		m_indicators.push_back(m_indicator("CHOKE: " + std::to_string(fakelag::get().max_choke), Color(130, 170, 20)));
		choke_indicator = true;
	}

	if (configs.visuals.indicators[INDICATOR_LC] && cheat.local() && cheat.local()->is_alive() && cheat.local()->m_vecVelocity().Length2D() > 251) {
		static auto is_breaking_lagcomp = false;
		if (!m_clientstate()->m_iChokedCommands)
		{
			static auto& last_origin = cheat.local()->m_vecOrigin();
			if (cheat.local()->m_vecOrigin() != last_origin)
			{
				is_breaking_lagcomp = (last_origin.Length2DSqr() - cheat.local()->m_vecOrigin().Length2DSqr()) > 4096.f;
				last_origin = cheat.local()->m_vecOrigin();
			}
		}

		m_indicators.push_back(m_indicator("LC", is_breaking_lagcomp ? Color(130, 170, 20) : Color(130, 20, 20)));
	}
	
	static auto cl_lagcompensation = m_cvar()->FindVar(crypt_str("cl_lagcompensation"));

	if (configs.visuals.indicators[INDICATOR_ANTI_EXPLOIT] && !cl_lagcompensation->GetBool())
		m_indicators.push_back(m_indicator("AX", m_engine()->IsInGame() ? Color(130, 170, 20) : Color(130, 20, 20)));

	if (configs.visuals.indicators[INDICATOR_DAMAGE] && cheat.globals.current_weapon > -1 && key_binds::get().get_key_bind_state(key_bind_features::DAMAGE_OVERRIDE) && !weapon->is_non_aim())
	{
		auto& currentWeapon = configs.ragebot.weapon[cheat.globals.current_weapon].enabled ? configs.ragebot.weapon[cheat.globals.current_weapon] : configs.ragebot.weapon[0];
		if (currentWeapon.minimum_override_damage > 100)
			m_indicators.push_back(m_indicator("DAMAGE: HP + " + std::to_string(currentWeapon.minimum_override_damage - 100), Color(130, 170, 20)));
		else
			m_indicators.push_back(m_indicator("DAMAGE: " + std::to_string(currentWeapon.minimum_override_damage), Color(130, 170, 20)));
	}

	if (configs.visuals.indicators[INDICATOR_SAFE_POINTS] && key_binds::get().get_key_bind_state(key_bind_features::SAFE_POINT) && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator("SAFE POINTS", Color(130, 170, 20)));

	if (configs.visuals.indicators[INDICATOR_BODY_AIM] && key_binds::get().get_key_bind_state(key_bind_features::BODY_AIM) && !weapon->is_non_aim())
		m_indicators.push_back(m_indicator("BODY AIM", Color(130, 170, 20)));

	if (configs.visuals.indicators[INDICATOR_RESOLVER_OVERRIDE] && key_binds::get().get_key_bind_state(key_bind_features::RESOLVER_OVERRIDE) && configs.ragebot.resolver[RESOLVER_YAW] && configs.ragebot.resolver_override.key > KEY_NONE && configs.ragebot.resolver_override.key < KEY_MAX)
		m_indicators.push_back(m_indicator("RESOLVER OVERRIDE", Color(130, 170, 20)));

	if (choke_indicator)
		return;

	if (configs.visuals.indicators[INDICATOR_DT] && configs.ragebot.double_tap && configs.ragebot.double_tap_key.key > KEY_NONE && configs.ragebot.double_tap_key.key < KEY_MAX && misc::get().double_tap_key)
		m_indicators.push_back(m_indicator("DT", !cheat.local()->m_bGunGameImmunity() && !(cheat.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().double_tap_enabled && !weapon->is_grenade() && weapon->m_iItemDefinitionIndex() != WEAPON_TASER && weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && weapon->can_fire(false) ? Color::White : Color(130, 20, 20)));

	if (configs.visuals.indicators[INDICATOR_HS] && configs.ragebot.hide_shots && configs.ragebot.hide_shots_key.key > KEY_NONE && configs.ragebot.hide_shots_key.key < KEY_MAX && misc::get().hide_shots_key)
		m_indicators.push_back(m_indicator("HS", !cheat.local()->m_bGunGameImmunity() && !(cheat.local()->m_fFlags() & FL_FROZEN) && !antiaim::get().freeze_check && misc::get().hide_shots_enabled ? Color(130, 170, 20) : Color(130, 20, 20)));
	}

void otheresp::draw_indicators()
{
	if (!cheat.local()->is_alive())
		return;

	static int width, height;
	m_engine()->GetScreenSize(width, height);

	auto h = height - 325;

	for (auto& indicator : m_indicators)
	{
		render::get().text(fonts[INDICATORFONT], 27, h, indicator.m_color, HFONT_CENTERED_Y, indicator.m_text.c_str());
		h -= 25;
	}

	m_indicators.clear();
}

void otheresp::hitmarker_paint()
{
	if (!configs.visuals.hitmarker[0] && !configs.visuals.hitmarker[1])
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (!cheat.local()->is_alive())
	{
		hitmarker.hurt_time = FLT_MIN;
		hitmarker.point = ZERO;
		return;
	}

	if (hitmarker.hurt_time + 0.7f > m_globals()->m_curtime)
	{
		if (configs.visuals.hitmarker[0])
		{
			static int width, height;
			m_engine()->GetScreenSize(width, height);

			auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
			hitmarker.hurt_color.SetAlpha(alpha);

			auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

			render::get().line(width / 2 + 5 + offset, height / 2 - 5 - offset, width / 2 + 12 + offset, height / 2 - 12 - offset, hitmarker.hurt_color);
			render::get().line(width / 2 + 5 + offset, height / 2 + 5 + offset, width / 2 + 12 + offset, height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(width / 2 - 5 - offset, height / 2 + 5 + offset, width / 2 - 12 - offset, height / 2 + 12 + offset, hitmarker.hurt_color);
			render::get().line(width / 2 - 5 - offset, height / 2 - 5 - offset, width / 2 - 12 - offset, height / 2 - 12 - offset, hitmarker.hurt_color);
		}

		if (configs.visuals.hitmarker[1])
		{
			Vector world;

			if (math::world_to_screen(hitmarker.point, world))
			{
				auto alpha = (int)((hitmarker.hurt_time + 0.7f - m_globals()->m_curtime) * 255.0f);
				hitmarker.hurt_color.SetAlpha(alpha);

				auto offset = 7.0f - (float)alpha / 255.0f * 7.0f;

				render::get().line(world.x + 5 + offset, world.y - 5 - offset, world.x + 12 + offset, world.y - 12 - offset, hitmarker.hurt_color);
				render::get().line(world.x + 5 + offset, world.y + 5 + offset, world.x + 12 + offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y + 5 + offset, world.x - 12 - offset, world.y + 12 + offset, hitmarker.hurt_color);
				render::get().line(world.x - 5 - offset, world.y - 5 - offset, world.x - 12 - offset, world.y - 12 - offset, hitmarker.hurt_color);
			}
		}
	}
}

void otheresp::damage_marker_paint()
{
	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		if (damage_marker[i].hurt_time + 2.0f > m_globals()->m_curtime)
		{
			Vector screen;

			if (!math::world_to_screen(damage_marker[i].position, screen))
				continue;

			auto alpha = (int)((damage_marker[i].hurt_time + 2.0f - m_globals()->m_curtime) * 127.5f);
			damage_marker[i].hurt_color.SetAlpha(alpha);

			render::get().text(fonts[DAMAGE_MARKER], screen.x, screen.y, damage_marker[i].hurt_color, HFONT_CENTERED_X | HFONT_CENTERED_Y, "%i", damage_marker[i].damage);
		}
	}
}

void otheresp::spread_crosshair()
{
	if (!configs.esp.enabled)
		return;

	if (!configs.visuals.show_spread)
		return;

	if (!cheat.local()->is_alive())
		return;

	auto weapon = cheat.globals.weapon;

	if (weapon->is_non_aim())
		return;

	int w, h;
	m_engine()->GetScreenSize(w, h);

	imRender->CircleFilled((float)w * 0.5f, (float)h * 0.5f, cheat.globals.inaccuracy * 500.0f, Color(configs.visuals.show_spread_color.r(), configs.visuals.show_spread_color.g(), configs.visuals.show_spread_color.b(), configs.visuals.show_spread_color.a()), 50);
}

void otheresp::automatic_peek_indicator()
{
	auto weapon = cheat.globals.weapon;

	if (!weapon)
		return;

	static auto position = ZERO;

	if (!cheat.globals.start_position.IsZero())
		position = cheat.globals.start_position;

	if (position.IsZero())
		return;

	static auto alpha = 0.0f;

	if ((!weapon->is_non_aim() && key_binds::get().get_key_bind_state(key_bind_features::AUTO_PEEK)) || alpha)
	{
		if (!weapon->is_non_aim() && key_binds::get().get_key_bind_state(key_bind_features::AUTO_PEEK))
			alpha += 3.0f * m_globals()->m_frametime;
		else
			alpha -= 3.0f * m_globals()->m_frametime;

		alpha = math::clamp(alpha, 0.0f, 1.0f);
		render::get().Draw3DFilledCircle(position, 25.0f, cheat.globals.fired_shot ? Color(30, 240, 30, (int)(alpha * 255.0f)) : Color(configs.misc.automatic_peek_color.get_red(), configs.misc.automatic_peek_color.get_green(), configs.misc.automatic_peek_color.get_blue(), (int)(alpha * 255.0f)));

		/*
		Vector screen;

		if (math::world_to_screen(position, screen))
		{
			static auto offset = 30.0f;

			if (!cheat.globals.fired_shot)
			{
				static auto switch_offset = false;

				if (offset <= 30.0f || offset >= 55.0f)
					switch_offset = !switch_offset;

				offset += switch_offset ? 22.0f * m_globals()->m_frametime : -22.0f * m_globals()->m_frametime;
				offset = math::clamp(offset, 30.0f, 55.0f);

				render::get().rect_filled(screen.x - 10.0f, screen.y - offset - 59.0f, 20.0f, 45.0f, Color(configs.misc.automatic_peek_color.get_red(), configs.misc.automatic_peek_color.get_green(), configs.misc.automatic_peek_color.get_blue(), (int)(alpha * 168.0f)));
				render::get().triangle(Vector2D(screen.x, screen.y - offset), Vector2D(screen.x - 10.5f, screen.y - offset - 14.5f), Vector2D(screen.x + 9.5f, screen.y - offset - 14.5f), Color(configs.misc.automatic_peek_color.get_red(), configs.misc.automatic_peek_color.get_green(), configs.misc.automatic_peek_color.get_blue(), (int)(alpha * 168.0f)));
			}
		}
		*/
	}
}