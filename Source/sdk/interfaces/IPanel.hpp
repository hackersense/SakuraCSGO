#pragma once

#include "../sdk/VirtualMethod.h"

class IPanel
{
public:
	VIRTUAL_METHOD(const char*, GetName, 36, (unsigned int vguiPanel), (this, vguiPanel))
};