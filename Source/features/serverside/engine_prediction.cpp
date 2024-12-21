#include "engine_prediction.h"

void engineprediction::store_netvars()
{
	const int tickbase = localPlayer->m_nTickBase();

	//auto data = &netvarData.at(tickbase % MULTIPLAYER_BACKUP);// m_clientstate()->m_iCommandAck

	Netvars_data netvars{ };

	netvars.tickbase = tickbase;

	netvars.aimPunchAngle = localPlayer->m_aimPunchAngle();
	netvars.aimPunchAngleVelocity = localPlayer->m_aimPunchAngleVel();
	netvars.baseVelocity = localPlayer->m_vecBaseVelocity();
	netvars.duckAmount = localPlayer->m_flDuckAmount();
	netvars.duckSpeed = localPlayer->m_flDuckSpeed();
	netvars.fallVelocity = localPlayer->m_flFallVelocity();
	netvars.thirdPersonRecoil = localPlayer->m_flThirdpersonRecoil();
	netvars.velocity = localPlayer->m_vecVelocity();
	netvars.velocityModifier = localPlayer->m_flVelocityModifier();
	netvars.viewPunchAngle = localPlayer->m_viewPunchAngle();
	netvars.viewOffset = localPlayer->m_vecViewOffset();

	netvarData.at(tickbase % MULTIPLAYER_BACKUP) = netvars;
}

void engineprediction::restore_netvars()
{
	const int tickbase = localPlayer->m_nTickBase();

	const auto& netvars = netvarData.at(tickbase % MULTIPLAYER_BACKUP);// (m_clientstate()->m_iCommandAck - 1)

	if (!&netvars)
		return;

	if (netvars.tickbase != tickbase)
		return;

	localPlayer->m_aimPunchAngle() = checkDifference(localPlayer->m_aimPunchAngle(), netvars.aimPunchAngle);
	localPlayer->m_aimPunchAngleVel() = checkDifference(localPlayer->m_aimPunchAngleVel(), netvars.aimPunchAngleVelocity);
	localPlayer->m_vecBaseVelocity() = checkDifference(localPlayer->m_vecBaseVelocity(), netvars.baseVelocity);
	localPlayer->m_flDuckAmount() = std::clamp(checkDifference(localPlayer->m_flDuckAmount(), netvars.duckAmount), 0.0f, 1.0f);
	localPlayer->m_flDuckSpeed() = checkDifference(localPlayer->m_flDuckSpeed(), netvars.duckSpeed);
	localPlayer->m_flFallVelocity() = checkDifference(localPlayer->m_flFallVelocity(), netvars.fallVelocity);
	localPlayer->m_flThirdpersonRecoil() = checkDifference(localPlayer->m_flThirdpersonRecoil(), netvars.thirdPersonRecoil);
	localPlayer->m_vecVelocity() = checkDifference(localPlayer->m_vecVelocity(), netvars.velocity);
	localPlayer->m_flVelocityModifier() = checkDifference(localPlayer->m_flVelocityModifier(), netvars.velocityModifier);
	localPlayer->m_viewPunchAngle() = checkDifference(localPlayer->m_viewPunchAngle(), netvars.viewPunchAngle);
	localPlayer->m_vecViewOffset() = checkDifference(localPlayer->m_vecViewOffset(), netvars.viewOffset);
}

/*
void engineprediction::update_vel()
{
	static int m_iLastCmdAck = 0;
	static float m_flNextCmdTime = 0.f;

	if (m_iLastCmdAck != m_clientstate()->m_nLastCommandAck || m_flNextCmdTime != m_clientstate()->m_flNextCmdTime)
	{
		if (cheat.globals.last_velocity_modifier != localPlayer->m_flVelocityModifier())
		{
			m_prediction()->m_bPreviousAckHadErrors = true;
			cheat.globals.last_velocity_modifier = localPlayer->m_flVelocityModifier();
		}

		m_iLastCmdAck = m_clientstate()->m_nLastCommandAck;
		m_flNextCmdTime = m_clientstate()->m_flNextCmdTime;
	}
}
*/

void engineprediction::predict(CUserCmd* m_pcmd)
{
	//if (m_clientstate()->m_iDeltaTick > 0)
	m_prediction()->Update(m_clientstate()->m_iDeltaTick, m_clientstate()->m_iDeltaTick > 0, m_clientstate()->m_nLastCommandAck, m_clientstate()->m_nLastOutgoingCommand + m_clientstate()->m_iChokedCommands);
	
	inPrediction = true;

	backup_data.flags = localPlayer->m_fFlags();
	backup_data.velocity = localPlayer->m_vecVelocity();

	*memory->predictionRandomSeed = MD5_PseudoRandom(m_pcmd->m_command_number) & INT_MAX;
	*memory->predictionPlayer = reinterpret_cast<int>(localPlayer.get());

	prediction_data.old_curtime = m_globals()->m_curtime;
	prediction_data.old_frametime = m_globals()->m_frametime;
	prediction_data.old_is_first_time_predicted = m_prediction()->m_bIsFirstTimePredicted;
	prediction_data.old_in_prediction = m_prediction()->m_bInPrediction;

	m_globals()->m_curtime = util::server_time();
	m_globals()->m_frametime = m_prediction()->m_bEnginePaused ? 0.0f : m_globals()->m_intervalpertick;
	m_prediction()->m_bIsFirstTimePredicted = false;
	m_prediction()->m_bInPrediction = true;
	//m_prediction()->m_bOldCLPredictValue = false;
	//m_prediction()->split->m_bIsFirstTimePredicted = false;

	if (m_pcmd->m_impulse)
		*reinterpret_cast<uint32_t*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x320C) = m_pcmd->m_impulse;

	m_pcmd->m_buttons |= *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x3344);
	m_pcmd->m_buttons &= ~(*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(localPlayer.get()) + 0x3340));

	localPlayer->updateButtonState(m_pcmd->m_buttons);

	m_gamemovement()->StartTrackPredictionErrors(localPlayer.get());
	m_prediction()->CheckMovingGround(localPlayer.get(), m_globals()->m_frametime);

	localPlayer->runPreThink();
	localPlayer->runThink();

	memory->moveHelper->SetHost(localPlayer.get());

	//CMoveData move_data;
	//move_data.m_flForwardMove = m_pcmd->m_forwardmove;
	//move_data.m_flSideMove = m_pcmd->m_sidemove;
	//move_data.m_flUpMove = m_pcmd->m_upmove;
	//move_data.m_nButtons = m_pcmd->m_buttons;
	//move_data.m_vecViewAngles = m_pcmd->m_viewangles;
	//move_data.m_vecAngles = m_pcmd->m_viewangles;
	//move_data.m_nImpulseCommand = m_pcmd->m_impulse;
	//memset(&move_data, 0, sizeof(CMoveData));

	m_prediction()->SetupMove(localPlayer.get(), m_pcmd, memory->moveHelper, memory->moveData);
	m_gamemovement()->ProcessMovement(localPlayer.get(), memory->moveData);
	m_prediction()->FinishMove(localPlayer.get(), m_pcmd, memory->moveData);
	memory->moveHelper->ProcessImpacts();

	localPlayer->runPostThink();

	m_gamemovement()->FinishTrackPredictionErrors(localPlayer.get());
	memory->moveHelper->SetHost(nullptr);
	m_gamemovement()->Reset();

	*memory->predictionRandomSeed = -1;
	*memory->predictionPlayer = 0;

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;

	m_prediction()->m_bIsFirstTimePredicted = prediction_data.old_is_first_time_predicted;
	m_prediction()->m_bInPrediction = prediction_data.old_in_prediction;

	/*
	auto viewmodel = localPlayer->m_hViewModel().Get();

	if (viewmodel)
	{
		viewmodel_data.weapon = viewmodel->m_hWeapon().Get();

		viewmodel_data.viewmodel_index = viewmodel->m_nViewModelIndex();
		viewmodel_data.sequence = viewmodel->m_nSequence();
		viewmodel_data.animation_parity = viewmodel->m_nAnimationParity();

		viewmodel_data.cycle = viewmodel->m_flCycle();
		viewmodel_data.animation_time = viewmodel->m_flAnimTime();
	}
	*/
	inPrediction = false;
	const auto activeWeapon = cheat.globals.weapon;
	if (activeWeapon && !activeWeapon->is_grenade() && !activeWeapon->is_knife())
		activeWeapon->update_accuracy_penality();
}

/*
void engineprediction::finish()
{
	if (prediction_data.prediction_stage != STAGE::FINISH)
		return;

	*memory->predictionRandomSeed = -1;
	*memory->predictionPlayer = 0;

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;
}
*/