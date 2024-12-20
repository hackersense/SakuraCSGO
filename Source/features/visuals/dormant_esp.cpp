#include "dormant_esp.h"
#include "player_esp.h"

void c_dormant_esp::start()
{
	m_utlCurSoundList.removeAll();
	m_enginesound()->GetActiveSounds(m_utlCurSoundList);

	if (!m_utlCurSoundList.count())
		return;

	for (auto i = 0; i < m_utlCurSoundList.count(); i++)
	{
		auto& sound = m_utlCurSoundList[i];

		if (sound.m_nSoundSource < 1 || sound.m_nSoundSource > 64)
			continue;

		if (sound.m_pOrigin->IsZero())
			continue;

		if (!valid_sound(sound))
			continue;

		auto player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(sound.m_nSoundSource));

		if (!player->valid(true, false))
			continue;

		setup_adjust(player, sound);
		m_cSoundPlayers[sound.m_nSoundSource].Override(sound);
	}

	m_utlvecSoundBuffer = m_utlCurSoundList;
}

void c_dormant_esp::setup_adjust(player_t* player, ActiveSoundInfo_t& sound)
{
	Vector src3D, dst3D;
	CGameTrace tr;
	Ray_t ray;
	CTraceFilter filter;

	src3D = *sound.m_pOrigin + Vector(0.0f, 0.0f, 1.0f);
	dst3D = src3D - Vector(0.0f, 0.0f, 100.0f);

	filter.pSkip = player;
	ray.Init(src3D, dst3D);

	m_trace()->TraceRay(ray, MASK_PLAYERSOLID, &filter, &tr);

	if (tr.allsolid)
		m_cSoundPlayers[sound.m_nSoundSource].m_iReceiveTime = -1;

	*sound.m_pOrigin = tr.fraction <= 0.97f ? tr.endpos : *sound.m_pOrigin;

	m_cSoundPlayers[sound.m_nSoundSource].m_nFlags = player->m_fFlags();
	m_cSoundPlayers[sound.m_nSoundSource].m_nFlags |= (tr.fraction < 0.50f ? FL_DUCKING : 0) | (tr.fraction < 1.0f ? FL_ONGROUND : 0);
	m_cSoundPlayers[sound.m_nSoundSource].m_nFlags &= (tr.fraction >= 0.50f ? ~FL_DUCKING : 0) | (tr.fraction >= 1.0f ? ~FL_ONGROUND : 0);
}

bool c_dormant_esp::adjust_sound(player_t* entity)
{
	auto i = entity->EntIndex();
	auto sound_player = m_cSoundPlayers[i];

	auto expired = false;

	if (fabs(m_globals()->m_realtime - sound_player.m_iReceiveTime) > 10.0f)
		expired = true;

	entity->m_bSpotted() = true;
	entity->m_fFlags() = sound_player.m_nFlags;
	entity->set_abs_origin(sound_player.m_vecOrigin);

	cheat.globals.dormant_origin[i] = sound_player.m_vecOrigin;
	return !expired;
}

bool c_dormant_esp::valid_sound(ActiveSoundInfo_t& sound)
{
	for (auto i = 0; i < m_utlvecSoundBuffer.count(); i++)
		if (m_utlvecSoundBuffer[i].m_nGuid == sound.m_nGuid)
			return false;

	return true;
}