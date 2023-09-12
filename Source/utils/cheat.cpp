#include "cheat.hpp"
cheat_t cheat;

bool cheat_t::available()
{
	return m_engine()->IsInGame() && localPlayer;
}
