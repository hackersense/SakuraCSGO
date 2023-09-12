#include "../structs.hpp"

void AnimationState::setupVelocity() noexcept
{
    m_modelcache()->BeginLock();

    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    Vector vecAbsVelocity = entity->m_vecAbsVelocity();

    // We should call this only if the entity isnt the localPlayer, since we cant figure out other players velocity
    /*
    if (localPlayer && localPlayer.get() != player)
    {
        // prevent the client input velocity vector from exceeding a reasonable magnitude
        if (vecAbsVelocity.LengthSqr() > std::sqrt(CS_PLAYER_SPEED_RUN * CSGO_ANIM_MAX_VEL_LIMIT))
            vecAbsVelocity = vecAbsVelocity.normalized() * (CS_PLAYER_SPEED_RUN * CSGO_ANIM_MAX_VEL_LIMIT);
    }
    */

    //this is a hack to fix shuffling animations due to rounding errors, not part of actual game code
    if (fabsf(vecAbsVelocity.x) < 0.001)
        vecAbsVelocity.x = 0.0f;
    if (fabsf(vecAbsVelocity.y) < 0.001)
        vecAbsVelocity.y = 0.0f;
    if (fabsf(vecAbsVelocity.z) < 0.001)
        vecAbsVelocity.z = 0.0f;

    // save vertical velocity component
    m_flVelocityLengthZ = vecAbsVelocity.z;

    // discard z component
    vecAbsVelocity.z = 0.f;

    // remember if the player is accelerating.
    playerIsAccelerating = (m_vecVelocityLast.LengthSqr() < vecAbsVelocity.LengthSqr());

    // rapidly approach ideal velocity instead of instantly adopt it. This helps smooth out instant velocity changes, like
    // when the player runs headlong into a wall and their velocity instantly becomes zero.
    m_vecVelocity = math::approach(vecAbsVelocity, m_vecVelocity, m_flUpdateTimeDelta * 2000.f);
    m_vecVelocityNormalized = m_vecVelocity.Normalized();

    // save horizontal velocity length
    m_flVelocityLengthXY = std::fminf(m_vecVelocity.Length(), CS_PLAYER_SPEED_RUN);

    if (m_flVelocityLengthXY > 0.f)
        m_vecVelocityNormalizedNonZero = m_vecVelocityNormalized;

    auto currentWeapon = reinterpret_cast<weapon_t*>(m_pActiveWeapon);
    float maxSpeedRun = currentWeapon ? std::fmaxf(currentWeapon->get_max_speed(), 0.001f) : CS_PLAYER_SPEED_RUN;

    //compute speed in various normalized forms
    m_flSpeedNormalized = std::clamp(m_flVelocityLengthXY / maxSpeedRun, 0.f, 1.f);
    m_flFeetSpeedForwardsOrSideWays = m_flVelocityLengthXY / (maxSpeedRun * CS_PLAYER_SPEED_WALK_MODIFIER);
    m_flFeetSpeedCrouchForwardOrSideways = m_flVelocityLengthXY / (maxSpeedRun * CS_PLAYER_SPEED_DUCK_MODIFIER);

    if (m_flFeetSpeedForwardsOrSideWays >= 1.f)
    {
        m_flStaticApproachSpeed = m_flVelocityLengthXY;
    }
    else if (m_flFeetSpeedForwardsOrSideWays < 0.5f)
    {
        m_flStaticApproachSpeed = math::approach(80.f, m_flStaticApproachSpeed, m_flUpdateTimeDelta * 60.f);
    }

    bool startedMovingThisFrame = false;
    bool stoppedMovingThisFrame = false;

    if (m_flVelocityLengthXY > 0.f)
    {
        startedMovingThisFrame = (m_flTimeSinceStartedMoving <= 0.f);
        m_flTimeSinceStoppedMoving = 0.f;
        m_flTimeSinceStartedMoving += m_flUpdateTimeDelta;
    }
    else
    {
        stoppedMovingThisFrame = (m_flTimeSinceStoppedMoving <= 0.f);
        m_flTimeSinceStartedMoving = 0.f;
        m_flTimeSinceStoppedMoving += m_flUpdateTimeDelta;
    }

    if (!m_bAdjustStarted && stoppedMovingThisFrame && m_bOnGround && !m_bOnLadder && !m_bOnLanding && stutterStep < 50)
    {
        setLayerSequence(ANIMATION_LAYER_ADJUST, ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING);
        m_bAdjustStarted = true;
    }

    if (getLayerActivity(ANIMATION_LAYER_ADJUST) == ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING ||
        getLayerActivity(ANIMATION_LAYER_ADJUST) == ACT_CSGO_IDLE_TURN_BALANCEADJUST)
    {
        if (m_bAdjustStarted && m_flFeetSpeedCrouchForwardOrSideways <= 0.25f)
        {
            incrementLayerCycleWeightRateGeneric(ANIMATION_LAYER_ADJUST);
            m_bAdjustStarted = !(isLayerSequenceCompleted(ANIMATION_LAYER_ADJUST));
        }
        else
        {
            m_bAdjustStarted = false;
            float weight = getLayerWeight(ANIMATION_LAYER_ADJUST);
            setLayerWeight(ANIMATION_LAYER_ADJUST, math::approach(0.f, weight, m_flUpdateTimeDelta * 5.f));
            setLayerWeightRate(ANIMATION_LAYER_ADJUST, weight);
        }
    }

    m_flCurrentFeetYaw = m_flGoalFeetYaw;
    m_flGoalFeetYaw = std::clamp(m_flGoalFeetYaw, -360.0f, 360.0f);
    float eyeFootDelta = math::angle_difference(m_flEyeYaw, m_flGoalFeetYaw);

    float aimMatrixWidthRange = math::lerp(std::clamp(m_flFeetSpeedForwardsOrSideWays, 0.f, 1.f), 1.0f, math::lerp(m_flStopToFullRunningFraction, CSGO_ANIM_AIM_NARROW_WALK, CSGO_ANIM_AIM_NARROW_RUN));

    if (m_flDuckAmount > 0.f)
    {
        aimMatrixWidthRange = math::lerp(m_flDuckAmount * std::clamp(m_flFeetSpeedCrouchForwardOrSideways, 0.f, 1.f), aimMatrixWidthRange, CSGO_ANIM_AIM_NARROW_CROUCHMOVING);
    }

    float tempYawMax = m_flAimYawMax * aimMatrixWidthRange;
    float tempYawMin = m_flAimYawMin * aimMatrixWidthRange;

    if (eyeFootDelta > tempYawMax)
    {
        m_flGoalFeetYaw = m_flEyeYaw - fabs(tempYawMax);
    }
    else if (eyeFootDelta < tempYawMin)
    {
        m_flGoalFeetYaw = m_flEyeYaw + fabs(tempYawMin);
    }
    m_flGoalFeetYaw = math::angleNormalize(m_flGoalFeetYaw);

    if (m_flVelocityLengthXY > 0.1f || fabs(m_flVelocityLengthZ) > 100.0f)
    {
        m_flGoalFeetYaw = math::approach_angle(m_flEyeYaw, m_flGoalFeetYaw, m_flUpdateTimeDelta * (30.0f + 20.0f * m_flStopToFullRunningFraction));
        lowerBodyRealignTimer = m_flLastClientSideAnimationUpdateTime + (CSGO_ANIM_LOWER_REALIGN_DELAY * 0.2f);
    }
    else
    {
        m_flGoalFeetYaw = math::approach_angle(entity->m_flLowerBodyYawTarget(), m_flGoalFeetYaw, m_flUpdateTimeDelta * CSGO_ANIM_LOWER_CATCHUP_IDLE);

        if (m_flLastClientSideAnimationUpdateTime > lowerBodyRealignTimer && fabsf(math::angle_difference(m_flGoalFeetYaw, m_flEyeYaw)) > 35.0f)
        {
            lowerBodyRealignTimer = m_flLastClientSideAnimationUpdateTime + CSGO_ANIM_LOWER_REALIGN_DELAY;
        }
    }

    if (m_flVelocityLengthXY <= CS_PLAYER_SPEED_STOPPED && m_bOnGround && !m_bOnLadder && !m_bOnLanding && m_flUpdateTimeDelta > 0.f && std::fabsf(math::angle_difference(m_flCurrentFeetYaw, m_flGoalFeetYaw) / m_flUpdateTimeDelta > 120.f))
    {
        setLayerSequence(ANIMATION_LAYER_ADJUST, ACT_CSGO_IDLE_TURN_BALANCEADJUST);
        m_bAdjustStarted = true;
    }

    // the final model render yaw is aligned to the foot yaw
    if (m_flVelocityLengthXY > 0.f)
    {
        // convert horizontal velocity vec to angular yaw
        float rawYawIdeal = (std::atan2(-m_vecVelocity.y, -m_vecVelocity.x) * 180 / 3.141592654f);
        if (rawYawIdeal < 0.f)
            rawYawIdeal += 360.f;

        m_flIdealVelocityLean = math::angleNormalize(math::angle_difference(rawYawIdeal, m_flGoalFeetYaw));
    }

    // delta between current yaw and ideal velocity derived target (possibly negative!)
    m_flLeanAmount = math::angleNormalize(math::angle_difference(m_flIdealVelocityLean, m_flCurrentTorsoYaw));

    if (startedMovingThisFrame && m_flFeetYawRate <= 0.f)
    {
        m_flCurrentTorsoYaw = m_flIdealVelocityLean;

        // select a special starting cycle that's set by the animator in content
        int moveSeq = getLayerSequence(ANIMATION_LAYER_MOVEMENT_MOVE);
        if (moveSeq != -1)
        {
            StudioSeqdesc seqdesc = entity->get_model_ptr()->getSeqdesc(moveSeq);
            if (seqdesc.numAnimTags > 0)
            {
                if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, 180)) <= EIGHT_WAY_WIDTH) //N
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_N);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, 135)) <= EIGHT_WAY_WIDTH) //NE
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_NE);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, 90)) <= EIGHT_WAY_WIDTH) //E
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_E);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, 45)) <= EIGHT_WAY_WIDTH) //SE
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_SE);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, 0)) <= EIGHT_WAY_WIDTH) //S
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_S);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, -45)) <= EIGHT_WAY_WIDTH) //SW
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_SW);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, -90)) <= EIGHT_WAY_WIDTH) //W
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_W);
                }
                else if (fabsf(math::angle_difference(m_flCurrentTorsoYaw, -135)) <= EIGHT_WAY_WIDTH) //NW
                {
                    m_flFeetCycle = entity->get_first_sequence_anim_tag(moveSeq, ANIMTAG_STARTCYCLE_NW);
                }
            }
        }

        /*
        if (m_flMovingFraction >= 1 && !firstRunSinceInit && abs(m_flLeanAmount) > 45 && onGround && entity->m_boneSnapshots[BONESNAPSHOT_ENTIRE_BODY].GetCurrentWeight() <= 0)
        {
            entity->m_boneSnapshots[BONESNAPSHOT_ENTIRE_BODY].SetShouldCapture(bonesnapshot_get(cl_bonesnapshot_speed_movebegin));
        }
        */
    }
    else
    {
        if (getLayerWeight(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE) >= 1.f)
        {
            m_flCurrentTorsoYaw = m_flIdealVelocityLean;
        }
        else
        {
            /*
            if (m_flMovingFraction >= 1 && !firstRunSinceInit && abs(m_flLeanAmount) > 100 && onGround && entity->m_boneSnapshots[BONESNAPSHOT_ENTIRE_BODY].GetCurrentWeight() <= 0)
            {
                entity->m_boneSnapshots[BONESNAPSHOT_ENTIRE_BODY].SetShouldCapture(bonesnapshot_get(cl_bonesnapshot_speed_movebegin));
            }
            */
            float moveWeight = math::lerp(m_flDuckAmount, std::clamp(m_flFeetSpeedForwardsOrSideWays, 0.f, 1.f), std::clamp(m_flFeetSpeedCrouchForwardOrSideways, 0.f, 1.f));
            float ratio = math::bias(moveWeight, 0.18f) + 0.1f;

            m_flCurrentTorsoYaw = math::angleNormalize(m_flCurrentTorsoYaw + (m_flLeanAmount * ratio));
        }
    }
    poseParamMappings[PLAYER_POSE_PARAM_MOVE_YAW].setValue(entity, m_flCurrentTorsoYaw);

    float aimYaw = math::angle_difference(m_flEyeYaw, m_flGoalFeetYaw);
    if (aimYaw >= 0.f && m_flAimYawMax != 0.f)
    {
        aimYaw = (aimYaw / m_flAimYawMax) * 60.0f;
    }
    else if (m_flAimYawMin != 0.f)
    {
        aimYaw = (aimYaw / m_flAimYawMin) * -60.0f;
    }

    poseParamMappings[PLAYER_POSE_PARAM_BODY_YAW].setValue(entity, aimYaw);

    // we need non-symmetrical arbitrary min/max bounds for vertical aim (pitch) too
    float pitch = math::angle_difference(m_flEyePitch, 0.f);
    if (pitch > 0.f)
    {
        pitch = (pitch / m_flAimPitchMax) * CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MAX;
    }
    else
    {
        pitch = (pitch / m_flAimPitchMin) * CSGO_ANIM_AIMMATRIX_DEFAULT_PITCH_MIN;
    }

    poseParamMappings[PLAYER_POSE_PARAM_BODY_PITCH].setValue(entity, pitch);
    poseParamMappings[PLAYER_POSE_PARAM_SPEED].setValue(entity, m_flFeetSpeedForwardsOrSideWays);
    poseParamMappings[PLAYER_POSE_PARAM_STAND].setValue(entity, 1.0f - (m_flDuckAmount * m_flMovingFraction));

    m_modelcache()->EndLock();
}

void AnimationState::setupMovement() noexcept
{
    m_modelcache()->BeginLock();

    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    if (m_flStopToFullRunningFraction > 0.f && m_flStopToFullRunningFraction < 1.f)
    {
        //currently transitioning between walk and run
        if (m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
        {
            m_flStopToFullRunningFraction += m_flUpdateTimeDelta * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        }
        else // m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK
        {
            m_flStopToFullRunningFraction -= m_flUpdateTimeDelta * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        }
        m_flStopToFullRunningFraction = std::clamp(m_flStopToFullRunningFraction, 0.f, 1.f);
    }

    if (m_flVelocityLengthXY > (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && m_bWalkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK)
    {
        //crossed the walk to run threshold
        m_bWalkToRunTransitionState = ANIM_TRANSITION_WALK_TO_RUN;
        m_flStopToFullRunningFraction = std::fmaxf(0.01f, m_flStopToFullRunningFraction);
    }
    else if (m_flVelocityLengthXY < (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && m_bWalkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
    {
        //crossed the run to walk threshold
        m_bWalkToRunTransitionState = ANIM_TRANSITION_RUN_TO_WALK;
        m_flStopToFullRunningFraction = std::fmaxf(0.99f, m_flStopToFullRunningFraction);
    }

    if (m_iAnimstateModelVersion < 2)
    {
        poseParamMappings[PLAYER_POSE_PARAM_RUN].setValue(entity, m_flStopToFullRunningFraction);
    }
    else
    {
        poseParamMappings[PLAYER_POSE_PARAM_MOVE_BLEND_WALK].setValue(entity, (1.0f - m_flStopToFullRunningFraction) * (1.0f - m_flDuckAmount));
        poseParamMappings[PLAYER_POSE_PARAM_MOVE_BLEND_RUN].setValue(entity, (m_flStopToFullRunningFraction) * (1.0f - m_flDuckAmount));
        poseParamMappings[PLAYER_POSE_PARAM_MOVE_BLEND_CROUCH_WALK].setValue(entity, m_flDuckAmount);
    }

    char weaponMoveSequenceString[MAX_ANIMSTATE_ANIMNAME_CHARS];
    sprintf(weaponMoveSequenceString, "move_%s", memory->getWeaponPrefix(this));

    int weaponMoveSeq = entity->lookupSequence(weaponMoveSequenceString);
    if (weaponMoveSeq == -1)
        weaponMoveSeq = entity->lookupSequence("move");

    if (entity->m_iMoveState() != previousMoveState)
        stutterStep += 10.f;

    previousMoveState = entity->m_iMoveState();
    stutterStep = std::clamp(math::approach(0.f, stutterStep, m_flUpdateTimeDelta * 40.f), 0.f, 100.f);

    // recompute moveweight
    float targetMoveWeight = math::lerp(m_flDuckAmount, std::clamp(m_flFeetSpeedForwardsOrSideWays, 0.f, 1.f), std::clamp(m_flFeetSpeedCrouchForwardOrSideways, 0.f, 1.f));

    if (m_flFeetYawRate <= targetMoveWeight)
    {
        m_flFeetYawRate = targetMoveWeight;
    }
    else
    {
        m_flFeetYawRate = math::approach(targetMoveWeight, m_flFeetYawRate, m_flUpdateTimeDelta * math::simple_spline_remap_val_clamped(stutterStep, 0.0f, 100.0f, 2.f, 20.f));
    }

    Vector vecMoveYawDir;
    math::angle_to_vector(Vector{ 0.f, math::angleNormalize(m_flGoalFeetYaw + m_flCurrentTorsoYaw + 180.f), 0.f }, &vecMoveYawDir);
    float yawDeltaAbsDot = std::fabsf(m_vecVelocityNormalizedNonZero.Dot(vecMoveYawDir));
    m_flFeetYawRate *= math::bias(yawDeltaAbsDot, 0.2f);

    float moveWeightWithAirSmooth = m_flFeetYawRate * m_flMovingFraction;

    // dampen move weight for landings
    moveWeightWithAirSmooth *= std::fmaxf((1.0f - getLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB)), 0.55f);

    float moveCycleRate = 0.f;
    if (m_flVelocityLengthXY > 0.f)
    {
        moveCycleRate = entity->get_sequence_cycle_rate(weaponMoveSeq);
        float sequenceGroundSpeed = std::fmaxf(entity->getSequenceMoveDist(entity->get_model_ptr(), weaponMoveSeq) / (1.0f / moveCycleRate), 0.001f);
        moveCycleRate *= m_flVelocityLengthXY / sequenceGroundSpeed;

        moveCycleRate *= math::lerp(m_flStopToFullRunningFraction, 1.0f, CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER);
    }

    float localCycleIncrement = (moveCycleRate * m_flUpdateTimeDelta);

    m_flFeetCycle = math::clamp_cycle(m_flFeetCycle + localCycleIncrement);

    moveWeightWithAirSmooth = std::clamp(moveWeightWithAirSmooth, 0.f, 1.f);
    updateAnimLayer(ANIMATION_LAYER_MOVEMENT_MOVE, weaponMoveSeq, localCycleIncrement, moveWeightWithAirSmooth, m_flFeetCycle);

    // blend in a strafe direction-change pose when the player changes strafe dir

    // get the user's left and right button pressed states
    bool moveRight = (buttons & (IN_MOVERIGHT)) != 0;
    bool moveLeft = (buttons & (IN_MOVELEFT)) != 0;
    bool moveForward = (buttons & (IN_FORWARD)) != 0;
    bool moveBackward = (buttons & (IN_BACK)) != 0;

    Vector vecForward;
    Vector vecRight;
    math::angle_to_vector_all(Vector{ 0.f, m_flGoalFeetYaw, 0.f }, &vecForward, &vecRight, nullptr);
    vecRight = vecRight.Normalized();
    float velToRightDot = m_vecVelocityNormalizedNonZero.Dot(vecRight);
    float velToForwardDot = m_vecVelocityNormalizedNonZero.Dot(vecForward);

    // We're interested in if the player's desired direction (indicated by their held buttons) is opposite their current velocity.
    // This indicates a strafing direction change in progress.

    bool bStrafeRight = (m_flFeetSpeedForwardsOrSideWays >= 0.73f && moveRight && !moveLeft && velToRightDot < -0.63f);
    bool bStrafeLeft = (m_flFeetSpeedForwardsOrSideWays >= 0.73f && moveLeft && !moveRight && velToRightDot > 0.63f);
    bool bStrafeForward = (m_flFeetSpeedForwardsOrSideWays >= 0.65f && moveForward && !moveBackward && velToForwardDot < -0.55f);
    bool bStrafeBackward = (m_flFeetSpeedForwardsOrSideWays >= 0.65f && moveBackward && !moveForward && velToForwardDot > 0.55f);

    entity->m_bStrafing() = (bStrafeRight || bStrafeLeft || bStrafeForward || bStrafeBackward);

    if (entity->m_bStrafing())
    {
        if (!strafeChanging)
        {
            durationStrafing = 0.f;

            /*
            if (!firstRunSinceInit && !strafeChanging && onGround && entity->m_boneSnapshots[BONESNAPSHOT_UPPER_BODY].GetCurrentWeight() <= 0)
            {
                entity->m_boneSnapshots[BONESNAPSHOT_UPPER_BODY].SetShouldCapture(bonesnapshot_get(cl_bonesnapshot_speed_strafestart));
            }
            */
        }

        strafeChanging = true;

        strafeChangeWeight = math::approach(1.f, strafeChangeWeight, m_flUpdateTimeDelta * 20.f);
        strafeChangeCycle = math::approach(0.f, strafeChangeCycle, m_flUpdateTimeDelta * 10.f);

        poseParamMappings[PLAYER_POSE_PARAM_STRAFE_DIR].setValue(entity, math::angleNormalize(m_flCurrentTorsoYaw));

    }
    else if (strafeChangeWeight > 0.f)
    {
        durationStrafing += m_flUpdateTimeDelta;

        if (durationStrafing > 0.08f)
            strafeChangeWeight = math::approach(0.f, strafeChangeWeight, m_flUpdateTimeDelta * 5.f);

        strafeSequence = entity->lookupSequence("strafe");
        float rate = entity->get_sequence_cycle_rate(strafeSequence);
        strafeChangeCycle = std::clamp(strafeChangeCycle + m_flUpdateTimeDelta * rate, 0.f, 1.f);
    }

    if (strafeChangeWeight <= 0.f)
    {
        strafeChanging = false;
    }

    // keep track of if the player is on the ground, and if the player has just touched or left the ground since the last check
    bool previousGroundState = m_bOnGround;
    m_bOnGround = (entity->m_fFlags() & 1);

    m_bLandedOnGroundThisFrame = (!m_bFirstRunSinceInit && previousGroundState != m_bOnGround && m_bOnGround);
    m_bLeftTheGroundThisFrame = (previousGroundState != m_bOnGround && !m_bOnGround);

    float distanceFell = 0.f;
    if (m_bLeftTheGroundThisFrame)
    {
        m_flLastOriginZ = m_vecOrigin.z;
    }

    if (m_bLandedOnGroundThisFrame)
    {
        distanceFell = fabs(m_flLastOriginZ - m_vecOrigin.z);
        float distanceFallNormalizedBiasRange = math::bias(math::simple_spline_remap_val_clamped(distanceFell, 12.0f, 72.0f, 0.0f, 1.0f), 0.4f);

        m_flHeadHeightOrOffsetFromHittingGroundAnimation = std::clamp(math::bias(m_flDurationInAir, 0.3f), 0.1f, 1.0f);
        m_flLandingDuckAdditiveSomething = std::fmax(m_flHeadHeightOrOffsetFromHittingGroundAnimation, distanceFallNormalizedBiasRange);

    }
    else
    {
        m_flLandingDuckAdditiveSomething = math::approach(0.f, m_flLandingDuckAdditiveSomething, m_flUpdateTimeDelta * 2.f);
    }

    // the in-air smooth value is a fuzzier representation of if the player is on the ground or not.
    // It will approach 1 when the player is on the ground and 0 when in the air. Useful for blending jump animations.
    m_flMovingFraction = math::approach(m_bOnGround ? 1.f : 0.f, m_flMovingFraction, math::lerp(m_flDuckAmount, CSGO_ANIM_ONGROUND_FUZZY_APPROACH, CSGO_ANIM_ONGROUND_FUZZY_APPROACH_CROUCH) * m_flUpdateTimeDelta);
    m_flMovingFraction = std::clamp(m_flMovingFraction, 0.f, 1.f);

    strafeChangeWeight *= (1.0f - m_flDuckAmount);
    strafeChangeWeight *= m_flMovingFraction;
    strafeChangeWeight = std::clamp(strafeChangeWeight, 0.f, 1.f);

    if (strafeSequence != -1)
        updateAnimLayer(ANIMATION_LAYER_MOVEMENT_STRAFECHANGE, strafeSequence, 0.f, strafeChangeWeight, strafeChangeCycle);

    //ladders
    bool previouslyOnLadder = m_bOnLadder;
    m_bOnLadder = !m_bOnGround && entity->get_move_type() == MOVETYPE_LADDER;
    bool startedLadderingThisFrame = (!previouslyOnLadder && m_bOnLadder);
    bool stoppedLadderingThisFrame = (previouslyOnLadder && !m_bOnLadder);

    if (m_flLadderWeight > 0.f || m_bOnLadder)
    {
        if (startedLadderingThisFrame)
        {
            setLayerSequence(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ACT_CSGO_CLIMB_LADDER);
        }

        if (std::abs(m_flVelocityLengthZ) > 100.f)
        {
            m_flLadderSpeed = math::approach(1, m_flLadderSpeed, m_flUpdateTimeDelta * 10.0f);
        }
        else
        {
            m_flLadderSpeed = math::approach(0.f, m_flLadderSpeed, m_flUpdateTimeDelta * 10.0f);
        }
        m_flLadderSpeed = std::clamp(m_flLadderSpeed, 0.f, 1.f);

        if (m_bOnLadder)
        {
            m_flLadderWeight = math::approach(1.f, m_flLadderWeight, m_flUpdateTimeDelta * 5.0f);
        }
        else
        {
            m_flLadderWeight = math::approach(0.f, m_flLadderWeight, m_flUpdateTimeDelta * 10.0f);
        }
        m_flLadderWeight = std::clamp(m_flLadderWeight, 0.f, 1.f);

        Vector vecLadderNormal = entity->m_vecLadderNormal();
        Vector angLadder;
        angLadder = math::vector_to_angles(vecLadderNormal);
        float ladderYaw = math::angle_difference(angLadder.y, m_flGoalFeetYaw);
        poseParamMappings[PLAYER_POSE_PARAM_LADDER_YAW].setValue(entity, ladderYaw);

        float ladderClimbCycle = getLayerCycle(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB);
        ladderClimbCycle += (m_vecOrigin.z - m_vecLastOrigin.z) * math::lerp(m_flLadderSpeed, 0.010f, 0.004f);

        poseParamMappings[PLAYER_POSE_PARAM_LADDER_SPEED].setValue(entity, m_flLadderSpeed);

        if (getLayerActivity(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB) == ACT_CSGO_CLIMB_LADDER)
        {
            setLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, m_flLadderWeight);
        }

        setLayerCycle(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ladderClimbCycle);

        // fade out jump if we're climbing
        if (m_bOnLadder)
        {
            float idealJumpWeight = 1.0f - m_flLadderWeight;
            if (getLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL) > idealJumpWeight)
            {
                setLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, idealJumpWeight);
            }
        }
    }
    else
    {
        m_flLadderSpeed = 0.f;
    }

    if (m_bOnGround)
    {
        if (!m_bOnLanding && (m_bLandedOnGroundThisFrame || stoppedLadderingThisFrame))
        {
            if (m_flDurationInAir > 1.f)
                setLayerSequence(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ACT_CSGO_LAND_HEAVY);
            else
                setLayerSequence(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, ACT_CSGO_LAND_LIGHT);
            setLayerCycle(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0.f);
            m_bOnLanding = true;
        }
        m_flDurationInAir = 0.f;

        if (m_bOnLanding && getLayerActivity(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB) != ACT_CSGO_CLIMB_LADDER)
        {
            jumping = false;

            incrementLayerCycle(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, false);
            incrementLayerCycle(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false);

            poseParamMappings[PLAYER_POSE_PARAM_JUMP_FALL].setValue(entity, 0.f);

            if (isLayerSequenceCompleted(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB))
            {
                m_bOnLanding = false;
                setLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0.f);
                setLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, 0.f);
                m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;
            }
            else
            {

                float landWeight = getLayerIdealWeightFromSeqCycle(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB) * m_flHeadHeightOrOffsetFromHittingGroundAnimation;

                // if we hit the ground crouched, reduce the land animation as a function of crouch, since the land animations move the head up a bit ( and this is undesirable )
                landWeight *= std::clamp((1.0f - m_flDuckAmount), 0.2f, 1.0f);

                setLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, landWeight);

                // fade out jump because land is taking over
                float currentJumpFallWeight = getLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
                if (currentJumpFallWeight > 0.f)
                {
                    currentJumpFallWeight = math::approach(0.f, currentJumpFallWeight, m_flUpdateTimeDelta * 10.0f);
                    setLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, currentJumpFallWeight);
                }

            }
        }

        if (!m_bOnLanding && !jumping && m_flLadderWeight <= 0.f)
        {
            setLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, 0.f);
        }
    }
    else if (!m_bOnLadder)
    {
        m_bOnLanding = false;

        // we're in the air
        if (m_bLeftTheGroundThisFrame || stoppedLadderingThisFrame)
        {
            // If entered the air by jumping, then we already set the jump activity.
            // But if we're in the air because we strolled off a ledge or the floor collapsed or something,
            // we need to set the fall activity here.
            if (!jumping)
            {
                setLayerSequence(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, ACT_CSGO_FALL);
            }

            m_flDurationInAir = 0.f;
        }

        m_flDurationInAir += m_flUpdateTimeDelta;

        incrementLayerCycle(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, false);

        // increase jump weight
        float jumpWeight = getLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
        float nextJumpWeight = getLayerIdealWeightFromSeqCycle(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL);
        if (nextJumpWeight > jumpWeight)
        {
            setLayerWeight(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, nextJumpWeight);
        }

        // bash any lingering land weight to zero
        float lingeringLandWeight = getLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB);
        if (lingeringLandWeight > 0)
        {
            lingeringLandWeight *= math::smooth_step_bounds(0.2f, 0.0f, m_flDurationInAir);
            setLayerWeight(ANIMATION_LAYER_MOVEMENT_LAND_OR_CLIMB, lingeringLandWeight);
        }

        // blend jump into fall. This is a no-op if we're playing a fall anim.
        poseParamMappings[PLAYER_POSE_PARAM_JUMP_FALL].setValue(entity, std::clamp(math::smooth_step_bounds(0.72f, 1.52f, m_flDurationInAir), 0.f, 1.f));
    }
    m_modelcache()->EndLock();
}

void AnimationState::setupAliveLoop() noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    if (getLayerActivity(ANIMATION_LAYER_ALIVELOOP) != ACT_CSGO_ALIVE_LOOP)
    {
        m_modelcache()->BeginLock();
        setLayerSequence(ANIMATION_LAYER_ALIVELOOP, ACT_CSGO_ALIVE_LOOP);
        setLayerCycle(ANIMATION_LAYER_ALIVELOOP, math::random_float(0.f, 1.f));
        auto layer = entity->get_animlayer(ANIMATION_LAYER_ALIVELOOP);
        if (layer)
        {
            float newRate = entity->get_sequence_cycle_rate(layer->m_iSequence);
            newRate *= math::random_float(0.8f, 1.1f);
            setLayerRate(ANIMATION_LAYER_ALIVELOOP, newRate);
        }
        m_modelcache()->EndLock();
        incrementLayerCycle(ANIMATION_LAYER_ALIVELOOP, true);
    }
    else
    {
        if (m_pActiveWeapon && m_pActiveWeapon != m_pLastActiveWeapon)
        {
            //re-roll act on weapon change
            float retainCycle = getLayerCycle(ANIMATION_LAYER_ALIVELOOP);
            setLayerSequence(ANIMATION_LAYER_ALIVELOOP, ACT_CSGO_ALIVE_LOOP);
            setLayerCycle(ANIMATION_LAYER_ALIVELOOP, retainCycle);
            incrementLayerCycle(ANIMATION_LAYER_ALIVELOOP, true);
        }
        else if (isLayerSequenceCompleted(ANIMATION_LAYER_ALIVELOOP))
        {
            //re-roll rate
            m_modelcache()->BeginLock();
            auto layer = entity->get_animlayer(ANIMATION_LAYER_ALIVELOOP);
            if (layer)
            {
                float newRate = entity->get_sequence_cycle_rate(layer->m_iSequence);
                newRate *= math::random_float(0.8f, 1.1f);
                setLayerRate(ANIMATION_LAYER_ALIVELOOP, newRate);
            }
            m_modelcache()->EndLock();
            incrementLayerCycle(ANIMATION_LAYER_ALIVELOOP, true);
        }
        else
        {
            float weightOutPoseBreaker = math::simple_spline_remap_val_clamped(m_flFeetSpeedForwardsOrSideWays, 0.55f, 0.9f, 1.0f, 0.0f);
            setLayerWeight(ANIMATION_LAYER_ALIVELOOP, weightOutPoseBreaker);
            incrementLayerCycle(ANIMATION_LAYER_ALIVELOOP, true);
        }
    }
}

void AnimationState::doAnimationEvent(int animationEvent) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    switch (animationEvent)
    {
    case PLAYERANIMEVENT_COUNT:
        return;
    case PLAYERANIMEVENT_THROW_GRENADE_UNDERHAND:
    {
        //addActivityModifier("underhand");
    }
    case PLAYERANIMEVENT_FIRE_GUN_PRIMARY:
    case PLAYERANIMEVENT_THROW_GRENADE:
    {
        auto currentWeapon = reinterpret_cast<weapon_t*>(m_pActiveWeapon);
        if (!currentWeapon)
            break;

        if (currentWeapon && animationEvent == PLAYERANIMEVENT_FIRE_GUN_PRIMARY && currentWeapon->m_iItemDefinitionIndex() == WEAPON_C4)
        {
            setLayerSequence(ANIMATION_LAYER_WHOLE_BODY, ACT_CSGO_PLANT_BOMB);
            m_bPlantAnimStarted = true;
            //*reinterpret_cast<int*>(animState + 316) = 1;//+316 plantAnimStarted
        }
        else
        {
            setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_PRIMARY);
        }
        break;
    }
    case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_PRIMARY_OPT_1);
        break;
    }
    case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_SPECIAL1:
    case PLAYERANIMEVENT_FIRE_GUN_PRIMARY_OPT_SPECIAL1:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_PRIMARY_OPT_2);
        break;
    }
    case PLAYERANIMEVENT_FIRE_GUN_SECONDARY:
    {
        auto currentWeapon = reinterpret_cast<weapon_t*>(m_pActiveWeapon);
        if (!currentWeapon)
            break;

        if (currentWeapon->is_sniper())
        {
            // hack: sniper rifles use primary fire anim when 'alt' firing, meaning scoped.
            setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_PRIMARY);
        }
        else
        {
            setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_SECONDARY);
        }
        break;
    }
    case PLAYERANIMEVENT_FIRE_GUN_SECONDARY_SPECIAL1:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_FIRE_SECONDARY_OPT_1);
        break;
    }
    case PLAYERANIMEVENT_GRENADE_PULL_PIN:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_OPERATE);
        break;
    }
    case PLAYERANIMEVENT_SILENCER_ATTACH:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_SILENCER_ATTACH);
        break;
    }
    case PLAYERANIMEVENT_SILENCER_DETACH:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_SILENCER_DETACH);
        break;
    }
    case PLAYERANIMEVENT_RELOAD:
    {
        auto currentWeapon = reinterpret_cast<weapon_t*>(m_pActiveWeapon);
        if (!currentWeapon)
            break;

        if (currentWeapon && currentWeapon->is_shotgun() && currentWeapon->m_iItemDefinitionIndex() != WEAPON_MAG7)
            break;

        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_RELOAD);
        break;
    }
    case PLAYERANIMEVENT_RELOAD_START:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_RELOAD_START);
        break;
    }
    case PLAYERANIMEVENT_RELOAD_LOOP:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_RELOAD_LOOP);
        break;
    }
    case PLAYERANIMEVENT_RELOAD_END:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_RELOAD_END);
        break;
    }
    case PLAYERANIMEVENT_CATCH_WEAPON:
    {
        setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_CATCH);
        break;
    }
    case PLAYERANIMEVENT_CLEAR_FIRING:
    {
        m_bPlantAnimStarted = false;
        break;
    }
    case PLAYERANIMEVENT_DEPLOY:
    {
        auto currentWeapon = reinterpret_cast<weapon_t*>(m_pActiveWeapon);
        if (!currentWeapon)
            break;

        if (currentWeapon && getLayerActivity(ANIMATION_LAYER_WEAPON_ACTION) == ACT_CSGO_DEPLOY &&
            getLayerCycle(ANIMATION_LAYER_WEAPON_ACTION) < 0.15f/*CSGO_ANIM_DEPLOY_RATELIMIT*/)
        {
            // we're already deploying
            deployRateLimiting = true;
        }
        else
        {
            setLayerSequence(ANIMATION_LAYER_WEAPON_ACTION, ACT_CSGO_DEPLOY);
        }
        break;
    }
    case PLAYERANIMEVENT_JUMP:
    {
        // note: this event means a jump is definitely happening, not just that a jump is desired
        jumping = true;
        setLayerSequence(ANIMATION_LAYER_MOVEMENT_JUMP_OR_FALL, ACT_CSGO_JUMP);
        break;
    }
    default:
        break;
    }
}

void AnimationState::updateAnimLayer(size_t layerIndex, int sequence, float playbackRate, float weight, float cycle) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    if (sequence >= 2)
    {
        m_modelcache()->BeginLock();
        auto& layer = *entity->get_animlayer(layerIndex);
        if (&layer)
        {
            layer.m_iSequence = sequence;
            layer.m_flPlaybackRate = playbackRate;
            layer.m_flCycle = std::clamp(cycle, 0.0f, 1.0f);
            layer.m_flWeight = std::clamp(weight, 0.0f, 1.0f);
        }
        m_modelcache()->EndLock();
    }
}

bool animstatePoseParamCache::init(player_t* player, char* poseParamName) noexcept
{
    m_modelcache()->BeginLock();
    m_szName = poseParamName;
    int poseIndex = memory->lookUpPoseParameter(player->get_model_ptr(), poseParamName);
    m_iIndex = poseIndex;
    if (poseIndex != -1)
        m_bInitialized = true;
    m_modelcache()->EndLock();
    return m_bInitialized;
}

void animstatePoseParamCache::setValue(player_t* player, float value) noexcept
{
    if (!m_bInitialized)
    {
        init(player, m_szName);
    }
    if (m_bInitialized && player)
    {
        m_modelcache()->BeginLock();
        player->setPoseParameter(value, m_iIndex);
        m_modelcache()->EndLock();
    }
}

int32_t AnimationState::selectSequenceFromActivityModifier(int activity) noexcept
{
    bool isPlayerDucked = m_flDuckAmount > 0.55f;
    bool isPlayerRunning = m_flFeetSpeedForwardsOrSideWays > 0.25f;

    int32_t layerSequence = -1;
    switch (activity)
    {
    case ACT_CSGO_JUMP:
        layerSequence = 15 + static_cast <int32_t>(isPlayerRunning);
        if (isPlayerDucked)
            layerSequence = 17 + static_cast <int32_t>(isPlayerRunning);
        break;
    case ACT_CSGO_ALIVE_LOOP:
        layerSequence = 8;
        if (m_pLastActiveWeapon != m_pActiveWeapon)
            layerSequence = 9;
        break;
    case ACT_CSGO_IDLE_ADJUST_STOPPEDMOVING:
        layerSequence = 6;
        break;
    case ACT_CSGO_FALL:
        layerSequence = 14;
        break;
    case ACT_CSGO_IDLE_TURN_BALANCEADJUST:
        layerSequence = 4;
        break;
    case ACT_CSGO_LAND_LIGHT:
        layerSequence = 20;
        if (isPlayerRunning)
            layerSequence = 22;

        if (isPlayerDucked)
        {
            layerSequence = 21;
            if (isPlayerRunning)
                layerSequence = 19;
        }
        break;
    case ACT_CSGO_LAND_HEAVY:
        layerSequence = 23;
        if (isPlayerDucked)
            layerSequence = 24;
        break;

    case ACT_CSGO_CLIMB_LADDER:
        layerSequence = 13;
        break;
    default:
        break;
    }

    return layerSequence;
}

void AnimationState::setLayerSequence(size_t layer, int32_t activity) noexcept
{
    if (activity == ACT_INVALID)
        return;

    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto hdr = entity->get_model_ptr();
    if (!hdr)
        return;

    const auto sequence = selectSequenceFromActivityModifier(activity);

    if (sequence < 2)
        return;

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;

    l.m_flPlaybackRate = entity->get_layer_sequence_cycle_rate(&l, sequence);
    l.m_iSequence = sequence;
    l.m_flCycle = l.m_flWeight = 0.f;
}

void AnimationState::setLayerRate(size_t layer, float newRate) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;
    l.m_flPlaybackRate = newRate;
}

void AnimationState::setLayerCycle(size_t layer, float newCycle) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;
    l.m_iSequence = math::clamp_cycle(newCycle);
}

void AnimationState::setLayerWeight(size_t layer, float weight) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;
    l.m_flCycle = std::clamp(weight, 0.f, 1.f);
}

void AnimationState::setLayerWeightRate(size_t layer, float previous) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;

    float flNewRate = (l.m_flWeight - previous) / m_flUpdateTimeDelta;
    l.m_flWeightDeltaRate = flNewRate;
}

void AnimationState::incrementLayerCycle(size_t layer, bool allowLoop) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;

    if (fabsf(l.m_flPlaybackRate) <= 0)
        return;

    float currentCycle = l.m_flCycle;
    currentCycle += m_flUpdateTimeDelta * l.m_flPlaybackRate;

    if (!allowLoop && currentCycle >= 1)
    {
        currentCycle = 0.999f;
    }

    l.m_flCycle = (math::clamp_cycle(currentCycle));
}

void AnimationState::incrementLayerCycleWeightRateGeneric(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;

    float flWeightPrevious = getLayerWeight(layer);
    incrementLayerCycle(layer, false);
    setLayerWeight(layer, getLayerIdealWeightFromSeqCycle(layer));
    setLayerWeightRate(layer, flWeightPrevious);
}

void AnimationState::incrementLayerWeight(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return;

    if (abs(l.m_flWeightDeltaRate) <= 0)
        return;

    float currentWeight = l.m_flWeight;
    currentWeight += m_flUpdateTimeDelta * l.m_flWeightDeltaRate;
    currentWeight = std::clamp(currentWeight, 0.f, 1.f);

    l.m_flWeight = currentWeight;
}

float AnimationState::getLayerIdealWeightFromSeqCycle(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return 0.f;

    auto seqdesc = entity->get_model_ptr()->getSeqdesc(l.m_iSequence);

    float cycle = l.m_flCycle;
    if (cycle >= 0.999f)
        cycle = 1.f;
    float easeIn = seqdesc.fadeInTime;
    float easeOut = seqdesc.fadeOutTime;
    float idealWeight = 1.f;

    if (easeIn > 0.f && cycle < easeIn)
    {
        idealWeight = math::smooth_step_bounds(0.0f, easeIn, cycle);
    }
    else if (easeOut < 1.f && cycle > easeOut)
    {
        idealWeight = math::smooth_step_bounds(1.0f, easeOut, cycle);
    }

    if (idealWeight < 0.0015f)
        return 0.f;

    return std::clamp(idealWeight, 0.f, 1.f);
}

bool AnimationState::isLayerSequenceCompleted(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (&l)
        return (m_flUpdateTimeDelta * l.m_flPlaybackRate) + l.m_flCycle >= 1.0f;
    return false;
}

float AnimationState::getLayerCycle(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return 0.f;
    return l.m_flCycle;
}

float AnimationState::getLayerWeight(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return 0.f;
    return l.m_flWeight;
}

int AnimationState::getLayerActivity(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    AnimationLayer* l = entity->get_animlayer(layer);
    if (l)
        return memory->getSequenceActivity(entity, l->m_iSequence);
    return ACT_INVALID;
}

int AnimationState::getLayerSequence(size_t layer) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    auto& l = *entity->get_animlayer(layer);
    if (!&l)
        return 0;
    return l.m_iSequence;
}

int AnimationState::getTicksFromCycle(float playbackRate, float cycle, float previousCycle) noexcept
{
    int ticks = 0;

    if (m_bOnLadder || playbackRate <= 0.0f)
        return ticks;

    float targetCycle = cycle;

    if (targetCycle != previousCycle)
    {
        float currentCycle = previousCycle;
        float timeDelta = m_flUpdateTimeDelta != 0.0f ? m_flUpdateTimeDelta : m_globals()->m_intervalpertick;
        while (currentCycle < targetCycle)
        {
            currentCycle += playbackRate * timeDelta;
            ticks++;
        }
    }

    return ticks;
}

float AnimationState::calculatePlaybackRate(Vector velocity) noexcept
{
    auto entity = reinterpret_cast<player_t*>(m_pBaseEntity);

    const float m_flUpdateTimeDelta = m_globals()->m_intervalpertick;

    const Vector vecVelocity = math::approach(velocity, this->m_vecVelocity, m_flUpdateTimeDelta * 2000.f);
    const float velocityLengthXY = std::fminf(vecVelocity.Length(), CS_PLAYER_SPEED_RUN);

    float walkToRunTransition = this->m_flStopToFullRunningFraction;
    float walkToRunTransitionState = this->m_bWalkToRunTransitionState;

    if (walkToRunTransition > 0.f && walkToRunTransition < 1.f)
    {
        if (walkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
        {
            walkToRunTransition += m_flUpdateTimeDelta * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        }
        else
        {
            walkToRunTransition -= m_flUpdateTimeDelta * CSGO_ANIM_WALK_TO_RUN_TRANSITION_SPEED;
        }
        walkToRunTransition = std::clamp(walkToRunTransition, 0.f, 1.f);
    }

    if (velocityLengthXY > (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && walkToRunTransitionState == ANIM_TRANSITION_RUN_TO_WALK)
    {
        walkToRunTransitionState = ANIM_TRANSITION_WALK_TO_RUN;
        walkToRunTransition = std::fmaxf(0.01f, walkToRunTransition);
    }
    else if (velocityLengthXY < (CS_PLAYER_SPEED_RUN * CS_PLAYER_SPEED_WALK_MODIFIER) && walkToRunTransitionState == ANIM_TRANSITION_WALK_TO_RUN)
    {
        walkToRunTransitionState = ANIM_TRANSITION_RUN_TO_WALK;
        walkToRunTransition = std::fmaxf(0.99f, walkToRunTransition);
    }

    char weaponMoveSequenceString[MAX_ANIMSTATE_ANIMNAME_CHARS];
    sprintf(weaponMoveSequenceString, "move_%s", memory->getWeaponPrefix(this));

    int weaponMoveSeq = entity->lookupSequence(weaponMoveSequenceString);
    if (weaponMoveSeq == -1)
        weaponMoveSeq = entity->lookupSequence("move");

    float moveCycleRate = 0.f;
    if (velocityLengthXY > 0.f)
    {
        moveCycleRate = entity->get_sequence_cycle_rate(weaponMoveSeq);
        float sequenceGroundSpeed = std::fmaxf(entity->getSequenceMoveDist(entity->get_model_ptr(), weaponMoveSeq) / (1.0f / moveCycleRate), 0.001f);
        moveCycleRate *= velocityLengthXY / sequenceGroundSpeed;

        moveCycleRate *= math::lerp2(walkToRunTransition, 1.0f, CSGO_ANIM_RUN_ANIM_PLAYBACK_MULTIPLIER);
    }

    return moveCycleRate * m_flUpdateTimeDelta;
}