#pragma once

#include "../math/QAngle.hpp"
#include "../misc/CUserCmd.hpp"
#include "IMoveHelper.hpp"

class IClientEntity;

class IMoveHelper
{
	virtual void UnknownVirtual() = 0;
public:
	virtual void SetHost(entity_t* host) = 0;
private:
	virtual void pad00() = 0;
	virtual void pad01() = 0;
public:
	virtual void ProcessImpacts() = 0;
};