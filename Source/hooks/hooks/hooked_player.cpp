#include "..\hooks.hpp"
#include "..\..\features\serverside\local_animations.h"
#include "..\..\features\serverside\engine_prediction.h"

void __fastcall Hooks::setupVelocityHook(void* ecx, void* edx) noexcept
{
	static auto original = setupvelocity_hook.getOriginal<void>();

	auto animState = reinterpret_cast<AnimationState*>(ecx);

	auto entity = reinterpret_cast<player_t*>(animState->m_pBaseEntity);
	if (!entity || !entity->is_alive() || !entity->is_player() || !localPlayer || entity != localPlayer.get())
		return original(ecx);

	if (cheat.globals.updatingFake)
		return original(ecx);

	animState->setupVelocity();
}

void __fastcall Hooks::setupMovementHook(void* ecx, void* edx) noexcept
{
	static auto original = setupmovement_hook.getOriginal<void>();

	auto animState = reinterpret_cast<AnimationState*>(ecx);

	auto entity = reinterpret_cast<player_t*>(animState->m_pBaseEntity);
	if (!entity || !entity->is_alive() || !entity->is_player() || !localPlayer || entity != localPlayer.get())
		return original(ecx);

	if (cheat.globals.updatingFake)
		return original(ecx);

	animState->setupMovement();
}

void __fastcall Hooks::setupAliveloopHook(void* ecx, void* edx) noexcept
{
	static auto original = setupaliveloop_hook.getOriginal<void>();

	auto animState = reinterpret_cast<AnimationState*>(ecx);

	auto entity = reinterpret_cast<player_t*>(animState->m_pBaseEntity);
	if (!entity || !entity->is_alive() || !entity->is_player() || !localPlayer || entity != localPlayer.get())
		return original(ecx);

	if (cheat.globals.updatingFake)
		return original(ecx);

	animState->setupAliveLoop();
}

bool __fastcall Hooks::hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time) noexcept
{
	auto result = true;

	static auto r_jiggle_bones = m_cvar2()->FindVar(crypt_str("r_jiggle_bones"));
	auto r_jiggle_bones_backup = r_jiggle_bones->GetInt();

	r_jiggle_bones->SetValue(!configs.visuals.disable_jiggle_bones);

	if (!ecx)
		result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else if (!configs.ragebot.enabled && !configs.legitbot.enabled)
		result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else
	{
		auto player = reinterpret_cast<player_t*>(uintptr_t(ecx) - 4);

		if (!player->valid(false, false))
			result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
		else
		{
			auto animstate = player->get_animation_state();
			auto previous_weapon = animstate ? animstate->m_pLastBoneSetupWeapon : nullptr;

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = animstate->m_pActiveWeapon;

			if (cheat.globals.setuping_bones)
				result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (configs.legitbot.enabled && player != localPlayer.get())
				result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!localPlayer->is_alive())
				result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (player == localPlayer.get())
				result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!player->m_CachedBoneData().Count())
				result = setupbones_hook.callOriginal<bool, void*>(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (bone_world_out && max_bones != -1)
				memcpy(bone_world_out, player->m_CachedBoneData().Base(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = previous_weapon;
		}
	}

	r_jiggle_bones->SetValue(r_jiggle_bones_backup);
	return result;
}

void __fastcall Hooks::hooked_standardblendingrules(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask) noexcept
{
	const auto entity = reinterpret_cast<player_t*>(ecx);

	//auto backup_effects = entity->m_fEffects();

	//if (player == localPlayer)
	entity->m_fEffects() |= 8;

	standardblendingrules_hook.callOriginal<void, void*>(ecx, hdr, pos, q, curtime, boneMask);

	//if (player == localPlayer)
	//	player->m_fEffects() = backup_effects;
	entity->m_fEffects() &= ~8;
}

void __fastcall Hooks::hooked_doextrabonesprocessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context) noexcept
{
	/*
if (!player)
	return;

if (!player->is_alive() || player->IsDormant() || !player->get_animation_state())
	return;

float backup = player->get_animation_state()->m_flMovingFraction;
player->get_animation_state()->m_flMovingFraction = 0;

((DoExtraBonesProcessingFn)original_doextrabonesprocessing)(player, hdr, pos, q, matrix, bone_list, context);

player->get_animation_state()->m_flMovingFraction = backup;
*/
	return;
}

void __fastcall Hooks::hooked_updateclientsideanimation(void* ecx, void* edx) noexcept
{
	if (cheat.globals.updating_skins)
		return;

	if (!configs.ragebot.enabled && !configs.legitbot.enabled)
		return updateclientsideanimation_hook.callOriginal<void, void*>(ecx);

	if (cheat.globals.updating_animation)
		return updateclientsideanimation_hook.callOriginal<void, void*>(ecx);

	auto entity = reinterpret_cast<player_t*>(ecx);

	if (entity == localPlayer.get())
		return updateclientsideanimation_hook.callOriginal<void, void*>(ecx);

	if (localPlayer && !localPlayer->is_alive())
		return updateclientsideanimation_hook.callOriginal<void, void*>(ecx);

	if (!entity->valid(false, false))
		return updateclientsideanimation_hook.callOriginal<void, void*>(ecx);
}

void __fastcall Hooks::hooked_physicssimulate(void* ecx, void* edx) noexcept
{
	const auto entity = reinterpret_cast<player_t*>(ecx);

	if (entity != localPlayer.get() || !localPlayer->is_alive())
		return physicssimulate_hook.callOriginal<void, void*>(ecx);

	const int simulation_tick = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(ecx) + 0x2AC);
	
	if (m_globals()->m_tickcount == simulation_tick)
		return;

	CommandContext* commandContext = localPlayer->getCommandContext();
	if (!commandContext || !commandContext->m_bNeedsProcessing)
		return;

	//engineprediction::get().restore_netvars();
	physicssimulate_hook.callOriginal<void, void*>(ecx);
	engineprediction::get().store_netvars();
}

void __fastcall Hooks::hooked_modifyeyeposition(void* ecx, void* edx, unsigned int* position) noexcept
{
	if (ecx && cheat.globals.in_createmove)
		return modifyeyeposition_hook.callOriginal<void, void*>(ecx, position);
}

void __fastcall Hooks::hooked_calcviewmodelbob(void* ecx, void* edx, float eyeOrigin) noexcept
{
	auto entity = reinterpret_cast<player_t*>(ecx);

	if (!configs.visuals.removals[REMOVALS_LANDING_BOB] || !entity || !entity->is_alive() || !entity->is_player() || !localPlayer || entity != localPlayer.get())
		return calcviewmodelbob_hook.callOriginal<void, void*>(ecx, eyeOrigin);
}

bool __fastcall Hooks::hooked_tracefilterforheadcollision(void* ecx, void* edx, player_t* player, unsigned int traceParams) noexcept
{
	static auto original = tracefilterforheadcollision_hook.getOriginal<bool>(player, traceParams);

	if (!localPlayer || !localPlayer->is_alive())
		return original(ecx, player, traceParams);

	if (!player || !player->is_player() || player == localPlayer.get())
		return original(ecx, player, traceParams);

	if (fabsf(player->GetAbsOrigin().z - localPlayer->GetAbsOrigin().z) < 10.0f)
		return false;

	return original(ecx, player, traceParams);
}

bool __fastcall Hooks::hooked_shouldskipanimframe(void* ecx, void* edx) noexcept
{
	return false;
}

int __fastcall Hooks::processinterpolatedlist() noexcept
{
	static auto allow_extrapolation = *reinterpret_cast<bool**>(util::FindSignature(crypt_str("client.dll"), crypt_str("A2 ? ? ? ? 8B 45 E8")) + 1);

	if (allow_extrapolation)
		*allow_extrapolation = false;

	return processinterpolatedlist_hook.callOriginal<int>();
}