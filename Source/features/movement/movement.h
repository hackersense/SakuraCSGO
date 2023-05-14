#pragma once

#include "..\..\includes.hpp"

class movement : public singleton<movement>
{
public:
	void longjump(CUserCmd* m_pcmd);
	void edgejump(CUserCmd* m_pcmd);
	void jumpbug(CUserCmd* m_pcmd);
};