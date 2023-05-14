#pragma once
#include "CUserCmd.hpp"

class CommandContext
{
public:
	bool m_bNeedsProcessing;
	CUserCmd m_pCmd;
	int m_iCommandNumber;
};