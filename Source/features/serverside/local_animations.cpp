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

			if (localPlayer->m_flSimulationTime() != fake_simulation_time)
			{
				fake_server_update = true;
				fake_simulation_time = localPlayer->m_flSimulationTime();
			}

			localPlayer->get_animlayers()[3].m_flWeight = 0.0f;
			localPlayer->get_animlayers()[3].m_flCycle = 0.0f;
			localPlayer->get_animlayers()[12].m_flWeight = 0.0f;

			cheat.globals.updatingFake = true;
			update_fake_animations();
			cheat.globals.updatingFake = false;
		}
		else if (stage == FRAME_RENDER_START)
		{
			auto animstate = localPlayer->get_animation_state();

			if (!animstate)
				return;

			real_server_update = false;

			if (localPlayer->m_flSimulationTime() != real_simulation_time)
			{
				real_server_update = true;
				real_simulation_time = localPlayer->m_flSimulationTime();
			}

			localPlayer->get_animlayers()[3].m_flWeight = 0.0f;
			localPlayer->get_animlayers()[3].m_flCycle = 0.0f;
			localPlayer->get_animlayers()[12].m_flWeight = 0.0f;

			if (configs.misc.slide_walk == 2) {
				static auto alpha = 1.0f;
				static auto switch_alpha = false;

				if (alpha <= 0.0f || alpha >= 1.0f)
					switch_alpha = !switch_alpha;

				alpha += switch_alpha ? 1.75f * m_globals()->m_frametime : -1.75f * m_globals()->m_frametime;
				alpha = math::clamp(alpha, 0.0f, 1.0f);

				if (localPlayer->m_fFlags() & FL_ONGROUND)
					if (switch_alpha) {
						localPlayer->m_flPoseParameter()[0] = -1.f;
						localPlayer->m_flPoseParameter()[1] = 1.f;
					}
					else if (!switch_alpha) {
						localPlayer->m_flPoseParameter()[0] = 1.f;
						localPlayer->m_flPoseParameter()[1] = -1.f;
					}
			}

			update_local_animations(animstate);

			if (configs.misc.slide_walk == 2 && !(localPlayer->m_fFlags() & FL_ONGROUND) && animstate->m_bOnLanding)
			{
				localPlayer->m_flPoseParameter()[6] = 1;
				localPlayer->m_flPoseParameter()[3] = 1;
				localPlayer->m_flPoseParameter()[7] = 1;
			}
		}
	}
	else if (stage == FRAME_RENDER_START)
	{
		auto animstate = localPlayer->get_animation_state();

		if (!animstate)
			return;

		real_server_update = false;
		fake_server_update = false;

		if (localPlayer->m_flSimulationTime() != real_simulation_time || localPlayer->m_flSimulationTime() != fake_simulation_time)
		{
			real_server_update = fake_server_update = true;
			real_simulation_time = fake_simulation_time = localPlayer->m_flSimulationTime();
		}

		localPlayer->get_animlayers()[3].m_flWeight = 0.0f;
		localPlayer->get_animlayers()[3].m_flCycle = 0.0f;
		localPlayer->get_animlayers()[12].m_flWeight = 0.0f;

		if (configs.misc.slide_walk == 2) {
			static auto alpha = 1.0f;
			static auto switch_alpha = false;

			if (alpha <= 0.0f || alpha >= 1.0f)
				switch_alpha = !switch_alpha;

			alpha += switch_alpha ? 1.75f * m_globals()->m_frametime : -1.75f * m_globals()->m_frametime;
			alpha = math::clamp(alpha, 0.0f, 1.0f);

			if (localPlayer->m_fFlags() & FL_ONGROUND)
			{
				if (switch_alpha) {
					localPlayer->m_flPoseParameter()[0] = -1.f;
					localPlayer->m_flPoseParameter()[1] = 1.f;
				}
				else if (!switch_alpha) {
					localPlayer->m_flPoseParameter()[0] = 1.f;
					localPlayer->m_flPoseParameter()[1] = -1.f;
				}
			}
		}

		cheat.globals.updatingFake = true;
		update_fake_animations();
		cheat.globals.updatingFake = false;
		update_local_animations(animstate);

		if (configs.misc.slide_walk == 2 && !(localPlayer->m_fFlags() & FL_ONGROUND) && animstate->m_bOnLanding)
		{
			localPlayer->m_flPoseParameter()[6] = 1;
			localPlayer->m_flPoseParameter()[3] = 1;
			localPlayer->m_flPoseParameter()[7] = 1;
		}
	}
}

void local_animations::update_prediction_animations()
{
	auto alloc = !local_data.prediction_animstate;
	auto change = !alloc && handle != &localPlayer->GetRefEHandle();
	auto reset = !alloc && !change && localPlayer->m_flSpawnTime() != spawntime;

	if (change)
		m_memalloc()->Free(local_data.prediction_animstate);

	if (reset)
	{
		util::reset_state(local_data.prediction_animstate);
		spawntime = localPlayer->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.prediction_animstate = (AnimationState*)m_memalloc()->Alloc(sizeof(AnimationState));

		if (local_data.prediction_animstate)
			util::create_state(local_data.prediction_animstate, localPlayer.get());

		handle = (CBaseHandle*)&localPlayer->GetRefEHandle();
		spawntime = localPlayer->m_flSpawnTime();
	}

	if (!alloc && !change && !reset)
	{
		float pose_parameter[24];
		memcpy(pose_parameter, &localPlayer->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[15];
		memcpy(layers, localPlayer->get_animlayers(), localPlayer->animlayer_count() * sizeof(AnimationLayer));

		local_data.prediction_animstate->m_pBaseEntity = localPlayer.get();
		util::update_state(local_data.prediction_animstate, ZERO);

		localPlayer->setup_bones_fixed(cheat.globals.prediction_matrix, localPlayer->m_CachedBoneData().m_Size, BONE_USED_BY_ANYTHING, m_globals()->m_curtime);

		memcpy(&localPlayer->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(localPlayer->get_animlayers(), layers, localPlayer->animlayer_count() * sizeof(AnimationLayer));
	}
}

void local_animations::update_fake_animations()
{
	auto alloc = !local_data.animstate;
	auto change = !alloc && handle != &localPlayer->GetRefEHandle();
	auto reset = !alloc && !change && localPlayer->m_flSpawnTime() != spawntime;

	if (change)
		m_memalloc()->Free(local_data.animstate);

	if (reset)
	{
		util::reset_state(local_data.animstate);
		spawntime = localPlayer->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.animstate = static_cast<AnimationState*>(m_memalloc()->Alloc(sizeof(AnimationState)));

		if (local_data.animstate)
			util::create_state(local_data.animstate, localPlayer.get());

		handle = (CBaseHandle*)&localPlayer->GetRefEHandle();
		spawntime = localPlayer->m_flSpawnTime();
	}

	if (!alloc && !change && !reset && fake_server_update)
	{
		float pose_parameter[24];
		memcpy(pose_parameter, &localPlayer->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[15];
		memcpy(layers, localPlayer->get_animlayers(), localPlayer->animlayer_count() * sizeof(AnimationLayer));

		auto backup_frametime = m_globals()->m_frametime;
		auto backup_curtime = m_globals()->m_curtime;

		m_globals()->m_frametime = m_globals()->m_intervalpertick;
		m_globals()->m_curtime = localPlayer->m_flSimulationTime();

		local_data.animstate->m_pBaseEntity = localPlayer.get();
		util::update_state(local_data.animstate, local_animations::get().local_data.fake_angles);

		local_data.animstate->m_bOnLanding = false;
		local_data.animstate->m_flLandingDuckAdditiveSomething = 0.0f;
		local_data.animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

		localPlayer->setup_bones_fixed(cheat.globals.fake_matrix, localPlayer->m_CachedBoneData().m_Size, BONE_USED_BY_ANYTHING, m_globals()->m_curtime);
		local_data.visualize_lag = configs.esp.visualize_lag;

		if (!local_data.visualize_lag)
		{
			const Vector origin = localPlayer->GetRenderOrigin();
			for (auto i : cheat.globals.fake_matrix)
			{
				i[0][3] -= origin.x;
				i[1][3] -= origin.y;
				i[2][3] -= origin.z;
			}
		}

		m_globals()->m_frametime = backup_frametime;
		m_globals()->m_curtime = backup_curtime;

		memcpy(&localPlayer->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(localPlayer->get_animlayers(), layers, localPlayer->animlayer_count() * sizeof(AnimationLayer));
	}
}

void local_animations::update_local_animations(AnimationState* animstate)
{
	if (tickcount != m_globals()->m_tickcount)
	{
		tickcount = m_globals()->m_tickcount;
		memcpy(layers, localPlayer->get_animlayers(), localPlayer->animlayer_count() * sizeof(AnimationLayer));

		if (local_data.animstate)
			animstate->m_flDuckAmount = local_data.animstate->m_flDuckAmount;

		animstate->m_iLastClientSideAnimationUpdateFramecount = 0;
		util::update_state(animstate, local_animations::get().local_data.fake_angles);

		if (real_server_update)
		{
			abs_angles = animstate->m_flGoalFeetYaw;
			//abs_angles = localPlayer->GetAbsAngles().y;
			memcpy(pose_parameter, &localPlayer->m_flPoseParameter(), 24 * sizeof(float));
		}
	}
	else
		animstate->m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount;

	animstate->m_flGoalFeetYaw = antiaim::get().condition(cheat.get_command()) ? abs_angles : local_animations::get().local_data.real_angles.y;
	localPlayer->set_abs_angles(Vector(0.0f, abs_angles, local_animations::get().local_data.real_angles.z));

	memcpy(localPlayer->get_animlayers(), layers, localPlayer->animlayer_count() * sizeof(AnimationLayer));
	memcpy(&localPlayer->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
}
