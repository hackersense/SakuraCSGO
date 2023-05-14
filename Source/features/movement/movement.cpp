#include "movement.h"
#include "../serverside/engine_prediction.h"

void movement::edgejump(CUserCmd* m_pcmd)
{
	if (key_binds::get().get_key_bind_state(key_bind_features::EDGE_JUMP) && engineprediction::get().backup_data.flags & FL_ONGROUND && !(cheat.local()->m_fFlags() & FL_ONGROUND))
		m_pcmd->m_buttons |= IN_JUMP;
}

void movement::jumpbug(CUserCmd* m_pcmd)
{
    if (!key_binds::get().get_key_bind_state(key_bind_features::JUMP_BUG))
        return;

    if (const auto mt = cheat.local()->get_move_type(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
        return;

    if (!(engineprediction::get().backup_data.flags & FL_ONGROUND) && cheat.local()->m_fFlags() & FL_ONGROUND)
    {
        if (cheat.globals.fakeducking)
            m_pcmd->m_buttons &= ~IN_BULLRUSH;
        m_pcmd->m_buttons |= IN_DUCK;
    }

    if (cheat.local()->m_fFlags() & FL_ONGROUND)
        m_pcmd->m_buttons &= ~IN_JUMP;
}

void movement::longjump(CUserCmd* m_pcmd)
{
	static int saved_tick_count;
	static bool in_lj;
	static bool do_lj;

	if (!key_binds::get().get_key_bind_state(key_bind_features::LONG_JUMP))
		return;

	if (const auto mt = cheat.local()->get_move_type(); mt == MOVETYPE_LADDER || mt == MOVETYPE_NOCLIP)
		return;

	if (cheat.local()->m_fFlags() & FL_ONGROUND)
	{
		saved_tick_count = m_globals()->m_tickcount;
	}

	if ((m_globals()->m_tickcount - saved_tick_count) > 3)
	{
		do_lj = false;
	}
	else
	{
		do_lj = true;
	}

	if (do_lj && !(cheat.local()->m_fFlags() & FL_ONGROUND))
	{
		m_pcmd->m_buttons |= IN_DUCK;
		in_lj = true;
	}
}