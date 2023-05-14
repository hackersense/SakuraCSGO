#pragma once

#include "..\..\includes.hpp"
#include "antiaim.h"

class fakelag : public singleton <fakelag>
{
public:
	void run(CUserCmd* m_pcmd);
	void create_move(CUserCmd* m_pcmd);
	bool fakelag_condition(CUserCmd* m_pcmd);

	bool condition = true;
	bool started_peeking = false;

	int max_choke = 0;
};