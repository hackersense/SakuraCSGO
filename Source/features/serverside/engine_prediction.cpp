#include "engine_prediction.h"

void engineprediction::store_netvars()
{
	const int tickbase = cheat.local()->m_nTickBase();

	auto data = &netvars_data[tickbase % MULTIPLAYER_BACKUP];// m_clientstate()->m_iCommandAck

	data->tickbase = tickbase;

	data->aimPunchAngle = cheat.local()->m_aimPunchAngle();
	data->aimPunchAngleVelocity = cheat.local()->m_aimPunchAngleVel();
	data->baseVelocity = cheat.local()->m_vecBaseVelocity();
	data->duckAmount = cheat.local()->m_flDuckAmount();
	data->duckSpeed = cheat.local()->m_flDuckSpeed();
	data->fallVelocity = cheat.local()->m_flFallVelocity();
	data->thirdPersonRecoil = cheat.local()->m_flThirdpersonRecoil();
	data->velocity = cheat.local()->m_vecVelocity();
	data->velocityModifier = cheat.local()->m_flVelocityModifier();
	data->viewPunchAngle = cheat.local()->m_viewPunchAngle();
	data->viewOffset = cheat.local()->m_vecViewOffset();
}

void engineprediction::restore_netvars()
{
	const int tickbase = cheat.local()->m_nTickBase();

	auto data = &netvars_data[tickbase % MULTIPLAYER_BACKUP];// (m_clientstate()->m_iCommandAck - 1)

	if (!data || data->tickbase != tickbase)
		return;

	cheat.local()->m_aimPunchAngle() = checkDifference(cheat.local()->m_aimPunchAngle(), data->aimPunchAngle);
	cheat.local()->m_aimPunchAngleVel() = checkDifference(cheat.local()->m_aimPunchAngleVel(), data->aimPunchAngleVelocity);
	cheat.local()->m_vecBaseVelocity() = checkDifference(cheat.local()->m_vecBaseVelocity(), data->baseVelocity);
	cheat.local()->m_flDuckAmount() = std::clamp(checkDifference(cheat.local()->m_flDuckAmount(), data->duckAmount), 0.0f, 1.0f);
	cheat.local()->m_flDuckSpeed() = checkDifference(cheat.local()->m_flDuckSpeed(), data->duckSpeed);
	cheat.local()->m_flFallVelocity() = checkDifference(cheat.local()->m_flFallVelocity(), data->fallVelocity);
	cheat.local()->m_flThirdpersonRecoil() = checkDifference(cheat.local()->m_flThirdpersonRecoil(), data->thirdPersonRecoil);
	cheat.local()->m_vecVelocity() = checkDifference(cheat.local()->m_vecVelocity(), data->velocity);
	cheat.local()->m_flVelocityModifier() = checkDifference(cheat.local()->m_flVelocityModifier(), data->velocityModifier);
	cheat.local()->m_viewPunchAngle() = checkDifference(cheat.local()->m_viewPunchAngle(), data->viewPunchAngle);
	cheat.local()->m_vecViewOffset() = checkDifference(cheat.local()->m_vecViewOffset(), data->viewOffset);
}

/*
void engineprediction::update_vel()
{
	static int m_iLastCmdAck = 0;
	static float m_flNextCmdTime = 0.f;

	if (m_iLastCmdAck != m_clientstate()->m_nLastCommandAck || m_flNextCmdTime != m_clientstate()->m_flNextCmdTime)
	{
		if (cheat.globals.last_velocity_modifier != cheat.local()->m_flVelocityModifier())
		{
			m_prediction()->m_bPreviousAckHadErrors = true;
			cheat.globals.last_velocity_modifier = cheat.local()->m_flVelocityModifier();
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
	
	backup_data.flags = cheat.local()->m_fFlags();
	backup_data.velocity = cheat.local()->m_vecVelocity();

	*memory->predictionRandomSeed = MD5_PseudoRandom(m_pcmd->m_command_number) & INT_MAX;
	*memory->predictionPlayer = reinterpret_cast<int>(cheat.local());

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
		cheat.local()->m_nImpulse() = m_pcmd->m_impulse;

	m_pcmd->m_buttons |= *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(cheat.local()) + 0x3344);
	m_pcmd->m_buttons &= ~(*reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(cheat.local()) + 0x3340));

	cheat.local()->updateButtonState(m_pcmd->m_buttons);

	m_gamemovement()->StartTrackPredictionErrors(cheat.local());
	m_prediction()->CheckMovingGround(cheat.local(), m_globals()->m_frametime);

	cheat.local()->runPreThink();
	cheat.local()->runThink();

	m_movehelper()->SetHost(cheat.local());

	//CMoveData move_data;
	//move_data.m_flForwardMove = m_pcmd->m_forwardmove;
	//move_data.m_flSideMove = m_pcmd->m_sidemove;
	//move_data.m_flUpMove = m_pcmd->m_upmove;
	//move_data.m_nButtons = m_pcmd->m_buttons;
	//move_data.m_vecViewAngles = m_pcmd->m_viewangles;
	//move_data.m_vecAngles = m_pcmd->m_viewangles;
	//move_data.m_nImpulseCommand = m_pcmd->m_impulse;
	//memset(&move_data, 0, sizeof(CMoveData));

	m_prediction()->SetupMove(cheat.local(), m_pcmd, m_movehelper(), m_movedata());
	m_gamemovement()->ProcessMovement(cheat.local(), m_movedata());
	m_prediction()->FinishMove(cheat.local(), m_pcmd, m_movedata());
	m_movehelper()->ProcessImpacts();

	cheat.local()->runPostThink();

	m_gamemovement()->FinishTrackPredictionErrors(cheat.local());
	m_movehelper()->SetHost(nullptr);
	m_gamemovement()->Reset();

	*memory->predictionRandomSeed = -1;
	*memory->predictionPlayer = 0;

	m_globals()->m_curtime = prediction_data.old_curtime;
	m_globals()->m_frametime = prediction_data.old_frametime;

	m_prediction()->m_bIsFirstTimePredicted = prediction_data.old_is_first_time_predicted;
	m_prediction()->m_bInPrediction = prediction_data.old_in_prediction;

	/*
	auto viewmodel = cheat.local()->m_hViewModel().Get();

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