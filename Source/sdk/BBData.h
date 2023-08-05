#pragma once

#include "../includes.hpp"

struct BoxInfo
{
	player_t* player = nullptr;
	Box box;
	Vector absOrigin = ZERO;
	Vector renderOrigin = ZERO;
	Vector child[MAXSTUDIOBONES] = { ZERO };
	Vector parent[MAXSTUDIOBONES] = { ZERO };
	bool valid = false;
	bool skeletonValid = false;

	void reset()
	{
		NoValid();
	}

	void NoValid()
	{
		ResetSkeleton();
		player = nullptr;
		valid = false;
		absOrigin = renderOrigin = ZERO;
	}

	void ResetSkeleton()
	{
		for (int i = 0; i < MAXSTUDIOBONES; i++)
			child[i] = parent[i] = ZERO;
		skeletonValid = false;
	}
};

namespace BBData
{
	void update();
	BoxInfo& getBox(int index);
	void reset(int index);

	inline float viewMatrix[4][4] = {};
}