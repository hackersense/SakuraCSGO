#include "slowwalk.h"

void slowwalk::create_move(CUserCmd* m_pcmd, float custom_speed)
{
	if (!(cheat.local()->m_fFlags() & FL_ONGROUND && engineprediction::get().backup_data.flags & FL_ONGROUND))
		return;

	auto weapon_info = cheat.globals.weapon->get_weapon_info();

	if (!weapon_info)
		return;

	if (custom_speed == -1.0f)
		cheat.globals.slowwalking = true;

	auto modifier = custom_speed == -1.0f ? 0.3f : custom_speed;
	auto max_speed = modifier * (cheat.globals.scoped ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed);

	if (!cheat.globals.weapon->is_non_aim())
	{
		auto move_length = sqrt(m_pcmd->m_sidemove * m_pcmd->m_sidemove + m_pcmd->m_forwardmove * m_pcmd->m_forwardmove);

		auto forwardmove = m_pcmd->m_forwardmove / move_length;
		auto sidemove = m_pcmd->m_sidemove / move_length;

		if (move_length > max_speed)
		{
			if (max_speed + 1.0f < cheat.local()->m_vecVelocity().Length2D())
			{
				m_pcmd->m_forwardmove = 0.0f;
				m_pcmd->m_sidemove = 0.0f;
			}
			else
			{
				m_pcmd->m_sidemove = max_speed * sidemove;
				m_pcmd->m_forwardmove = max_speed * forwardmove;
			}
		}
	}
	else
	{
		auto forwardmove = m_pcmd->m_forwardmove;
		auto sidemove = m_pcmd->m_sidemove;

		auto move_length = sqrt(sidemove * sidemove + forwardmove * forwardmove);
		auto move_length_backup = move_length;

		if (move_length > 110.0f)
		{
			m_pcmd->m_forwardmove = forwardmove / move_length_backup * 110.0f;
			move_length = sidemove / move_length_backup * 110.0f;
			m_pcmd->m_sidemove = sidemove / move_length_backup * 110.0f;
		}
	}
}
