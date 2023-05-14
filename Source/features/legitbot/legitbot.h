#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

class legitbot : public singleton<legitbot>
{
public:
	static void run(CUserCmd* cmd);
};
