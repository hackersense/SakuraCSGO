#pragma once

#include "..\..\includes.hpp"

#include <array>

class grenade_prediction : public singleton<grenade_prediction>
{
public:
	void run(CUserCmd* cmd);
	void draw();
};