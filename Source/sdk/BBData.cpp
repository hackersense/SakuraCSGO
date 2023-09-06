#include "BBData.h"
#include "..\features\ragebot\ragebot.h"
#include "..\features\visuals\dormant_esp.h"
#include "..\features\visuals\player_esp.h"

class RadarPlayer_t
{
public:
	Vector origin;						//0x0000
	Vector angle;						//0x000C
	Vector spottedAngle;				//0x0018
	std::uintptr_t tabRelated;			//0x0024
	std::byte pad[0xC];					//0x0028
	float spottedTime;					//0x0034
	float spottedFzraction;				//0x0038
	float time;							//0x003C
	std::byte pad2[0x4];				//0x0040
	int playerIndex;					//0x0044
	int entityIndex;					//0x0048
	std::byte pad3[0x4];				//0x004C
	int health;							//0x0050
	char name[32];						//0x0058
	std::byte pad4[0x75];				//0x0074
	bool spotted;						//0x00E9
	std::byte pad5[0x8A];				//0x00EA
};

class CCSGO_HudRadar
{
public:
	std::byte pad[0x14C];
	RadarPlayer_t radar_info[65];
};

static BoxInfo player_info[65] = {};

bool calculation_skeleton(BoxInfo& info, player_t* e, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	if (!e)
		return false;

	auto model = e->GetModel();

	if (!model)
		return false;

	auto studio_model = m_modelinfo()->GetStudioModel(model);

	if (!studio_model)
		return false;

	auto get_bone_position = [&](int bone) -> Vector
	{
		return Vector(matrix[bone][0][3], matrix[bone][1][3], matrix[bone][2][3]);
	};

	//auto upper_direction = get_bone_position(7) - get_bone_position(6);
	//auto breast_bone = get_bone_position(6) + upper_direction * 0.5f;

	for (int i = 0; i < studio_model->numBones; i++)
	{
		auto bone = studio_model->getBone(i);

		if (bone && (bone->flags & BONE_USED_BY_HITBOX) && (bone->parent != -1))
		{
			auto child = get_bone_position(i);
			auto parent = get_bone_position(bone->parent);

			//auto delta_child = child - breast_bone;
			//auto delta_parent = parent - breast_bone;

			//if (delta_parent.Length() < 9.0f && delta_child.Length() < 9.0f)
			//	parent = breast_bone;

			//if (i == 5)
			//	child = breast_bone;

			//if ((fabs(delta_child.z) < 5.0f && delta_parent.Length() < 5.0f && delta_child.Length() < 5.0f) || i == 6)
			//	continue;

			info.child[i] = child;
			info.parent[i] = parent;
		}
	}

	return true;
}

void BBData::update()
{
	if (!cheat.available())
		return;

	static int lastFrame;
	if (lastFrame == m_globals()->m_framecount)
		return;
	lastFrame = m_globals()->m_framecount;

	for (int i = 0; i < 4; i++)
		for (int j = 0; j < 4; j++)
			viewMatrix[i][j] = math::world_to_screen_matrix()[i][j];

	c_dormant_esp::get().start();

	const auto radar_base = std::uintptr_t(memory->findHudElement(memory->hud, "CCSGO_HudRadar"));
	const auto hud_radar = reinterpret_cast<CCSGO_HudRadar*>(radar_base - 0x14);

	for (int i = 1; i < m_globals()->m_maxclients; i++)
	{
		auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));

		BoxInfo& info = player_info[i];

		if (!entity->valid(false, false))
		{
			info.NoValid();
			continue;
		}

		auto valid_dormant = false;
		auto backup_flags = entity->m_fFlags();
		auto& backup_origin = entity->GetAbsOrigin();

		if (entity->IsDormant() && cheat.local()->is_alive())
			valid_dormant = c_dormant_esp::get().adjust_sound(entity);
		else
		{
			playeresp::get().health[i] = entity->m_iHealth();
			c_dormant_esp::get().m_cSoundPlayers[i].reset(true, entity->GetAbsOrigin(), entity->m_fFlags());
		}

		if (radar_base && hud_radar && entity->IsDormant() && entity->m_iTeamNum() != cheat.local()->m_iTeamNum() && entity->m_bSpotted())
			playeresp::get().health[i] = hud_radar->radar_info[i].health;

		if (!playeresp::get().health[i])
		{
			if (entity->IsDormant())
			{
				entity->m_fFlags() = backup_flags;
				entity->set_abs_origin(backup_origin);
			}

			info.NoValid();

			continue;
		}

		auto fast = 2.5f * m_globals()->m_frametime;
		auto slow = 0.25f * m_globals()->m_frametime;

		if (entity->IsDormant())
		{
			auto& origin = entity->GetAbsOrigin();

			if (origin == ZERO)
				playeresp::get().esp_alpha_fade[i] = 0.0f;
			else if (!valid_dormant && playeresp::get().esp_alpha_fade[i] > 0.0f)
				playeresp::get().esp_alpha_fade[i] -= slow;
			else if (valid_dormant && playeresp::get().esp_alpha_fade[i] < 1.0f)
				playeresp::get().esp_alpha_fade[i] += fast;
		}
		else if (playeresp::get().esp_alpha_fade[i] < 1.0f)
			playeresp::get().esp_alpha_fade[i] += fast;

		playeresp::get().esp_alpha_fade[i] = math::clamp(playeresp::get().esp_alpha_fade[i], 0.0f, 1.0f);

		if (entity == cheat.local() && !cheat.local()->is_alive())
			info.NoValid();
		else
		{
			info.player = entity;

			if (!entity->IsDormant())
			{
				/*
				auto records = &player_records[i];
				if (!records->empty() && cheat.local()->is_alive())
				{
					auto record = &records->front();
					if (record->valid() && (configs.ragebot.enabled || configs.legitbot.enabled))
						info.skeletonValid = calculation_skeleton(info, entity, record->matrixes_data.main);
					else
						info.skeletonValid = calculation_skeleton(info, entity, entity->m_CachedBoneData().Base());
				}
				else
				*/
					info.skeletonValid = calculation_skeleton(info, entity, entity->m_CachedBoneData().Base());
			}
			else
			{
				info.ResetSkeleton();
			}

			info.absOrigin = entity->GetAbsOrigin();
			info.renderOrigin = entity->GetRenderOrigin();
			info.valid = util::get_bbox(entity, info.box);
		}

		if (entity->IsDormant())
		{
			entity->m_fFlags() = backup_flags;
			entity->set_abs_origin(backup_origin);
		}
	}
}

BoxInfo& BBData::getBox(int index)
{
	if (index < 65 && index > 0)
		return player_info[index];
	else
		return BoxInfo{};
}

void BBData::reset(int index)
{
	if (index < 65 && index > 0)
		player_info[index].reset();
}
