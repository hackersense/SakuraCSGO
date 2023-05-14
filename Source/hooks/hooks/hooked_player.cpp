#include "..\hooks.hpp"
#include "..\..\features\serverside\local_animations.h"
#include "..\..\features\serverside\engine_prediction.h"

bool __fastcall Hooks::hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time)
{
	auto result = true;

	static auto r_jiggle_bones = m_cvar()->FindVar(crypt_str("r_jiggle_bones"));
	auto r_jiggle_bones_backup = r_jiggle_bones->GetInt();

	r_jiggle_bones->SetValue(0);

	if (!ecx)
		result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else if (!configs.ragebot.enabled && !configs.legitbot.enabled)
		result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
	else
	{
		auto player = reinterpret_cast<player_t*>(uintptr_t(ecx) - 4);

		if (!player->valid(false, false))
			result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
		else
		{
			auto animstate = player->get_animation_state();
			auto previous_weapon = animstate ? animstate->m_pLastBoneSetupWeapon : nullptr;

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = animstate->m_pActiveWeapon;

			if (cheat.globals.setuping_bones)
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (configs.legitbot.enabled && player != cheat.local())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!cheat.local()->is_alive())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (player == cheat.local())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (!player->m_CachedBoneData().Count())
				result = ((SetupBonesFn)original_setupbones)(ecx, bone_world_out, max_bones, bone_mask, current_time);
			else if (bone_world_out && max_bones != -1)
				memcpy(bone_world_out, player->m_CachedBoneData().Base(), player->m_CachedBoneData().Count() * sizeof(matrix3x4_t));

			if (previous_weapon)
				animstate->m_pLastBoneSetupWeapon = previous_weapon;
		}
	}

	r_jiggle_bones->SetValue(r_jiggle_bones_backup);
	return result;
}

void __fastcall Hooks::hooked_standardblendingrules(player_t* player, int i, c_studio_hdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask)
{
	auto backup_effects = player->m_fEffects();

	if (player == cheat.local())
		player->m_fEffects() |= 8;

	((StandardBlendingRulesFn)original_standardblendingrules)(player, hdr, pos, q, curtime, boneMask);

	if (player == cheat.local())
		player->m_fEffects() = backup_effects;
	//player->m_fEffects() &= ~8;
}

void __fastcall Hooks::hooked_doextrabonesprocessing(player_t* player, void* edx, c_studio_hdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context)
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
}

void __fastcall Hooks::hooked_updateclientsideanimation(player_t* player, uint32_t i)
{
	if (cheat.globals.updating_skins)
		return;

	if (cheat.globals.updating_animation)
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (player == cheat.local())
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (cheat.local() && !cheat.local()->is_alive())
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (!configs.ragebot.enabled && !configs.legitbot.enabled)
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);

	if (!player->valid(false, false))
		return ((UpdateClientSideAnimationFn)original_updateclientsideanimation)(player);
}

void __fastcall Hooks::hooked_physicssimulate(player_t* player, void* edx)
{
	const int simulation_tick = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(player) + 0x2AC);

	if (player != cheat.local() || !cheat.local()->is_alive() || m_globals()->m_tickcount == simulation_tick)
	{
		((PhysicsSimulateFn)original_physicssimulate)(player);
		return;
	}

	engineprediction::get().restore_netvars();
	((PhysicsSimulateFn)original_physicssimulate)(player);
	engineprediction::get().store_netvars();
}

void __fastcall Hooks::hooked_modifyeyeposition(c_baseplayeranimationstate* state, void* edx, Vector& position)
{
	if (state && cheat.globals.in_createmove)
		return ((ModifyEyePositionFn)original_modifyeyeposition)(state, position);
}

void __fastcall Hooks::hooked_calcviewmodelbob(player_t* player, void* edx, Vector& position)
{
	if (!configs.esp.removals[REMOVALS_LANDING_BOB] || player != cheat.local() || !cheat.local()->is_alive())
		return ((CalcViewmodelBobFn)original_calcviewmodelbob)(player, position);
}

bool __fastcall Hooks::hooked_tracefilterforheadcollision(void* ecx, void* edx, player_t* player, unsigned int traceParams)
{
	static auto original = tracefilterforheadcollision_hook.getOriginal<bool>(player, traceParams);

	if (!cheat.local() || !cheat.local()->is_alive())
		return original(ecx, player, traceParams);

	if (!player || !player->is_player() || player == cheat.local())
		return original(ecx, player, traceParams);

	if (fabsf(player->GetAbsOrigin().z - cheat.local()->GetAbsOrigin().z) < 10.0f)
		return false;

	return original(ecx, player, traceParams);
}

bool __fastcall Hooks::hooked_shouldskipanimframe(void* ecx, void* edx)
{
	return false;
}

int __fastcall Hooks::processinterpolatedlist()
{
	static auto allow_extrapolation = *reinterpret_cast<bool**>(util::FindSignature(crypt_str("client.dll"), crypt_str("A2 ? ? ? ? 8B 45 E8")) + 1);

	if (allow_extrapolation)
		*allow_extrapolation = false;

	return ((ProcessInterpolatedListFn)original_processinterpolatedlist)();
}