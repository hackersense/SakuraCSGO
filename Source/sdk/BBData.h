#pragma once

#include "../includes.hpp"

struct BoxInfo
{
	player_t* player = nullptr;
	Box box = Box{ };
	Vector absOrigin = ZERO;
	Vector renderOrigin = ZERO;
	Vector child[MAXSTUDIOBONES] = { };
	Vector parent[MAXSTUDIOBONES] = { };
	bool valid = false;
	bool skeletonValid = false;

	void reset()
	{
		valid = false;
		player = nullptr;
		box = Box{ };
		absOrigin = renderOrigin = ZERO;
		ResetSkeleton();
	}

	void ResetSkeleton()
	{
		for (int i = 0; i < MAXSTUDIOBONES; i++)
			child[i] = parent[i] = { };
		skeletonValid = false;
	}
};

namespace BBData
{
	void update();
	BoxInfo getBox(int index);
	void setBox(int index, BoxInfo box);
	void reset(int index);

	inline float viewMatrix[4][4] = {};
}