#pragma once
#include "..\..\includes.hpp"

class c_dormant_esp : public singleton<c_dormant_esp>
{
public:
	void start();

	bool adjust_sound(player_t* player);
	void setup_adjust(player_t* player, ActiveSoundInfo_t& sound);
	bool valid_sound(ActiveSoundInfo_t& sound);

	struct SoundPlayer
	{
		void reset(bool store_data = false, const Vector& origin = ZERO, int flags = 0)
		{
			if (store_data)
			{
				m_iReceiveTime = m_globals()->m_realtime;
				m_vecOrigin = origin;
				m_nFlags = flags;
			}
			else
			{
				m_iReceiveTime = 0.0f;
				m_vecOrigin.Zero();
				m_nFlags = 0;
			}
		}

		void Override(ActiveSoundInfo_t& sound)
		{
			m_iReceiveTime = m_globals()->m_realtime;
			m_vecOrigin = *sound.m_pOrigin;
		}

		float m_iReceiveTime = 0.0f;
		Vector m_vecOrigin = ZERO;
		int m_nFlags = 0;
	} m_cSoundPlayers[65];

	SafeCUtlVector<ActiveSoundInfo_t> m_utlvecSoundBuffer;
	SafeCUtlVector<ActiveSoundInfo_t> m_utlCurSoundList;
};