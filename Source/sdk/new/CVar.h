#pragma once

#include "..\VirtualMethod.h"

struct NEWConVar;

class NEWCvar {
public:
	VIRTUAL_METHOD(NEWConVar*, FindVar, 15, (const char* name), (this, name))
};

class NEWConCommandBase
{
public:
	void* vmt;
	NEWConCommandBase* next;
	bool registered;
	const char* name;
	const char* helpString;
	int flags;
	NEWConCommandBase* conCommandBases;
	void* accessor;
};