#include "bunnyhop.h"

void bunnyhop::create_move(CUserCmd* cmd)
{
    //static auto wasLastTimeOnGround{ cheat.local()->m_fFlags() & IN_JUMP };

   // if (configs.misc.bunnyhop && !(cheat.local()->m_fFlags() & IN_JUMP) && cheat.local()->get_move_type() != MOVETYPE_LADDER && !wasLastTimeOnGround)
    //    cmd->m_buttons &= ~IN_JUMP;

   // wasLastTimeOnGround = cheat.local()->m_fFlags() & IN_JUMP;

	static auto last_jumped = false;
	static auto should_jump = false;

	if (!last_jumped && should_jump)
	{
		should_jump = false;
		cmd->m_buttons |= IN_JUMP;
	}
	else if (cmd->m_buttons & IN_JUMP)
	{
		if (cheat.local()->m_fFlags() & FL_ONGROUND)
		{
			last_jumped = true;
			should_jump = true;
		}
		else 
		{
			cmd->m_buttons &= ~IN_JUMP;
			last_jumped = false;
		}
	}
	else
	{
		last_jumped = false;
		should_jump = false;
	}
}