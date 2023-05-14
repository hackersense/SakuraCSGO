#include "cheat.hpp"
cheat_t cheat;

bool cheat_t::available()
{
	return m_engine()->IsInGame() && local();
}

player_t* cheat_t::local(player_t* e, bool init) // Why add init? Prevent incorrect addition
{
	static player_t* local = nullptr;

	if (init)
		local = e;

	return local;
}

