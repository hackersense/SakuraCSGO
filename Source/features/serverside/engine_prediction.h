#pragma once

#include "..\..\includes.hpp"
#include "..\..\sdk\structs.hpp"

#define EPSILON 0.03125f

class engineprediction : public singleton<engineprediction>
{
	struct Netvars_data
	{
		int tickbase = -1;

		Vector aimPunchAngle{ };
		Vector aimPunchAngleVelocity{ };
		Vector baseVelocity{ };
		float duckAmount{ 0.0f };
		float duckSpeed{ 0.0f };
		float fallVelocity{ 0.0f };
		float thirdPersonRecoil{ 0.0f };
		Vector velocity{ };
		float velocityModifier{ 0.0f };
		Vector viewPunchAngle{ };
		Vector viewOffset{ };
	};

	struct Backup_data
	{
		int flags = 0;
		Vector velocity = ZERO;
	};

	struct Prediction_data
	{
		void reset()
		{
			old_curtime = old_frametime = 0.0f;
			old_is_first_time_predicted = old_in_prediction = false;
		}

		float old_curtime = 0.0f;
		float old_frametime = 0.0f;
		bool old_is_first_time_predicted = false;
		bool old_in_prediction = false;
	};

	struct Viewmodel_data
	{
		weapon_t* weapon = nullptr;

		int viewmodel_index = 0;
		int sequence = 0;
		int animation_parity = 0;

		float cycle = 0.0f;
		float animation_time = 0.0f;
	};
public:
	Netvars_data netvars_data[MULTIPLAYER_BACKUP];
	Backup_data backup_data;
	Prediction_data prediction_data;
	Viewmodel_data viewmodel_data;

	void store_netvars();
	void restore_netvars();
	void predict(CUserCmd* m_pcmd);
private:
	static float checkDifference(float predicted, float original) noexcept
	{
		float delta = predicted - original;

		if (fabsf(delta) <= EPSILON)
			return original;
		return predicted;
	}

	static Vector checkDifference(Vector predicted, Vector original) noexcept
	{
		Vector delta = predicted - original;

		if (fabsf(delta.x) <= EPSILON && fabsf(delta.y) <= EPSILON && fabsf(delta.z) <= EPSILON)
			return original;
		return predicted;
	}
};