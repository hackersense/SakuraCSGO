#include "local_animations.h"
#include "..\ragebot\antiaim.h"

void local_animations::run(ClientFrameStage_t stage)
{
	if (m_clientstate()->m_iDeltaTick == -1)
		return;

	if (!fakelag::get().condition && key_binds::get().get_key_bind_state(key_bind_features::FAKE_DUCK))
	{
		if (stage == FRAME_NET_UPDATE_END)
		{
			fake_server_update = false;

			if (cheat.local()->m_flSimulationTime() != fake_simulation_time)
			{
				fake_server_update = true;
				fake_simulation_time = cheat.local()->m_flSimulationTime();
			}

			cheat.local()->get_animlayers()[3].m_flWeight = 0.0f;
			cheat.local()->get_animlayers()[3].m_flCycle = 0.0f;
			cheat.local()->get_animlayers()[12].m_flWeight = 0.0f;

			update_fake_animations();
		}
		else if (stage == FRAME_RENDER_START)
		{
			auto animstate = cheat.local()->get_animation_state();

			if (!animstate)
				return;

			real_server_update = false;

			if (cheat.local()->m_flSimulationTime() != real_simulation_time)
			{
				real_server_update = true;
				real_simulation_time = cheat.local()->m_flSimulationTime();
			}

			cheat.local()->get_animlayers()[3].m_flWeight = 0.0f;
			cheat.local()->get_animlayers()[3].m_flCycle = 0.0f;
			cheat.local()->get_animlayers()[12].m_flWeight = 0.0f;

			if (configs.misc.slide_walk == 2) {
				static auto alpha = 1.0f;
				static auto switch_alpha = false;

				if (alpha <= 0.0f || alpha >= 1.0f)
					switch_alpha = !switch_alpha;

				alpha += switch_alpha ? 1.75f * m_globals()->m_frametime : -1.75f * m_globals()->m_frametime;
				alpha = math::clamp(alpha, 0.0f, 1.0f);

				if (cheat.local()->m_fFlags() & FL_ONGROUND)
					if (switch_alpha) {
						cheat.local()->m_flPoseParameter()[0] = -1.f;
						cheat.local()->m_flPoseParameter()[1] = 1.f;
					}
					else if (!switch_alpha) {
						cheat.local()->m_flPoseParameter()[0] = 1.f;
						cheat.local()->m_flPoseParameter()[1] = -1.f;
					}
			}

			update_local_animations(animstate);

			if (configs.misc.slide_walk == 2 && !(cheat.local()->m_fFlags() & FL_ONGROUND) && animstate->m_bInHitGroundAnimation)
			{
				cheat.local()->m_flPoseParameter()[6] = 1;
				cheat.local()->m_flPoseParameter()[3] = 1;
				cheat.local()->m_flPoseParameter()[7] = 1;
			}
		}
	}
	else if (stage == FRAME_RENDER_START)
	{
		auto animstate = cheat.local()->get_animation_state();

		if (!animstate)
			return;

		real_server_update = false;
		fake_server_update = false;

		if (cheat.local()->m_flSimulationTime() != real_simulation_time || cheat.local()->m_flSimulationTime() != fake_simulation_time)
		{
			real_server_update = fake_server_update = true;
			real_simulation_time = fake_simulation_time = cheat.local()->m_flSimulationTime();
		}

		cheat.local()->get_animlayers()[3].m_flWeight = 0.0f;
		cheat.local()->get_animlayers()[3].m_flCycle = 0.0f;
		cheat.local()->get_animlayers()[12].m_flWeight = 0.0f;

		if (configs.misc.slide_walk == 2) {
			static auto alpha = 1.0f;
			static auto switch_alpha = false;

			if (alpha <= 0.0f || alpha >= 1.0f)
				switch_alpha = !switch_alpha;

			alpha += switch_alpha ? 1.75f * m_globals()->m_frametime : -1.75f * m_globals()->m_frametime;
			alpha = math::clamp(alpha, 0.0f, 1.0f);

			if (cheat.local()->m_fFlags() & FL_ONGROUND)
			{
				if (switch_alpha) {
					cheat.local()->m_flPoseParameter()[0] = -1.f;
					cheat.local()->m_flPoseParameter()[1] = 1.f;
				}
				else if (!switch_alpha) {
					cheat.local()->m_flPoseParameter()[0] = 1.f;
					cheat.local()->m_flPoseParameter()[1] = -1.f;
				}
			}
		}

		update_fake_animations();
		update_local_animations(animstate);

		if (configs.misc.slide_walk == 2 && !(cheat.local()->m_fFlags() & FL_ONGROUND) && animstate->m_bInHitGroundAnimation)
		{
			cheat.local()->m_flPoseParameter()[6] = 1;
			cheat.local()->m_flPoseParameter()[3] = 1;
			cheat.local()->m_flPoseParameter()[7] = 1;
		}
	}
}

void local_animations::update_prediction_animations()
{
	auto alloc = !local_data.prediction_animstate;
	auto change = !alloc && handle != &cheat.local()->GetRefEHandle();
	auto reset = !alloc && !change && cheat.local()->m_flSpawnTime() != spawntime;

	if (change)
		m_memalloc()->Free(local_data.prediction_animstate);

	if (reset)
	{
		util::reset_state(local_data.prediction_animstate);
		spawntime = cheat.local()->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.prediction_animstate = (c_baseplayeranimationstate*)m_memalloc()->Alloc(sizeof(c_baseplayeranimationstate));

		if (local_data.prediction_animstate)
			util::create_state(local_data.prediction_animstate, cheat.local());

		handle = (CBaseHandle*)&cheat.local()->GetRefEHandle();
		spawntime = cheat.local()->m_flSpawnTime();
	}

	if (!alloc && !change && !reset)
	{
		float pose_parameter[24];
		memcpy(pose_parameter, &cheat.local()->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[15];
		memcpy(layers, cheat.local()->get_animlayers(), cheat.local()->animlayer_count() * sizeof(AnimationLayer));

		local_data.prediction_animstate->m_pBaseEntity = cheat.local();
		util::update_state(local_data.prediction_animstate, ZERO);

		cheat.local()->setup_bones_fixed(cheat.globals.prediction_matrix, cheat.local()->m_CachedBoneData().m_Size, BONE_USED_BY_ANYTHING, m_globals()->m_curtime);

		memcpy(&cheat.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(cheat.local()->get_animlayers(), layers, cheat.local()->animlayer_count() * sizeof(AnimationLayer));
	}
}

void local_animations::update_fake_animations()
{
	auto alloc = !local_data.animstate;
	auto change = !alloc && handle != &cheat.local()->GetRefEHandle();
	auto reset = !alloc && !change && cheat.local()->m_flSpawnTime() != spawntime;

	if (change)
		m_memalloc()->Free(local_data.animstate);

	if (reset)
	{
		util::reset_state(local_data.animstate);
		spawntime = cheat.local()->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.animstate = static_cast<c_baseplayeranimationstate*>(m_memalloc()->Alloc(sizeof(c_baseplayeranimationstate)));

		if (local_data.animstate)
			util::create_state(local_data.animstate, cheat.local());

		handle = (CBaseHandle*)&cheat.local()->GetRefEHandle();
		spawntime = cheat.local()->m_flSpawnTime();
	}

	if (!alloc && !change && !reset && fake_server_update)
	{
		float pose_parameter[24];
		memcpy(pose_parameter, &cheat.local()->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[15];
		memcpy(layers, cheat.local()->get_animlayers(), cheat.local()->animlayer_count() * sizeof(AnimationLayer));

		auto backup_frametime = m_globals()->m_frametime;
		auto backup_curtime = m_globals()->m_curtime;

		m_globals()->m_frametime = m_globals()->m_intervalpertick;
		m_globals()->m_curtime = cheat.local()->m_flSimulationTime();

		local_data.animstate->m_pBaseEntity = cheat.local();
		util::update_state(local_data.animstate, local_animations::get().local_data.fake_angles);

		local_data.animstate->m_bInHitGroundAnimation = false;
		local_data.animstate->m_flLandingDuckAdditiveSomething = 0.0f;
		local_data.animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

		cheat.local()->setup_bones_fixed(cheat.globals.fake_matrix, cheat.local()->m_CachedBoneData().m_Size, BONE_USED_BY_ANYTHING, m_globals()->m_curtime);
		local_data.visualize_lag = configs.esp.visualize_lag;

		if (!local_data.visualize_lag)
		{
			const auto origin = cheat.local()->GetRenderOrigin();
			for (auto& i : cheat.globals.fake_matrix)
			{
				i[0][3] -= origin.x;
				i[1][3] -= origin.y;
				i[2][3] -= origin.z;
			}
		}

		m_globals()->m_frametime = backup_frametime;
		m_globals()->m_curtime = backup_curtime;

		memcpy(&cheat.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(cheat.local()->get_animlayers(), layers, cheat.local()->animlayer_count() * sizeof(AnimationLayer));
	}
}

void local_animations::update_local_animations(c_baseplayeranimationstate* animstate)
{
	if (tickcount != m_globals()->m_tickcount)
	{
		tickcount = m_globals()->m_tickcount;
		memcpy(layers, cheat.local()->get_animlayers(), cheat.local()->animlayer_count() * sizeof(AnimationLayer));

		if (local_data.animstate)
			animstate->m_flDuckAmount = local_data.animstate->m_flDuckAmount;

		animstate->m_iLastClientSideAnimationUpdateFramecount = 0;
		util::update_state(animstate, local_animations::get().local_data.fake_angles);

		if (real_server_update)
		{
			abs_angles = animstate->m_flGoalFeetYaw;
			//abs_angles = cheat.local()->GetAbsAngles().y;
			memcpy(pose_parameter, &cheat.local()->m_flPoseParameter(), 24 * sizeof(float));
		}
	}
	else
		animstate->m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount;

	animstate->m_flGoalFeetYaw = antiaim::get().condition(cheat.get_command()) ? abs_angles : local_animations::get().local_data.real_angles.y;
	cheat.local()->set_abs_angles(Vector(0.0f, abs_angles, local_animations::get().local_data.real_angles.z));

	memcpy(cheat.local()->get_animlayers(), layers, cheat.local()->animlayer_count() * sizeof(AnimationLayer));
	memcpy(&cheat.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
}
