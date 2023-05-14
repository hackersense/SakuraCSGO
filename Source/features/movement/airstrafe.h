#pragma once

#include "..\..\includes.hpp"

class airstrafe : public singleton <airstrafe>
{
public:
	void create_move(CUserCmd* m_pcmd, Vector& currentViewAngles);
};