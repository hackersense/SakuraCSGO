#pragma once

#include "../math/QAngle.hpp"
#include "../misc/CUserCmd.hpp"
#include "IMoveHelper.hpp"

class CMoveData {
public:
	bool    m_bFirstRunOfFunctions : 1;
	bool    m_bGameCodeMovedPlayer : 1;
	int     m_nPlayerHandle;        // edict index on server, client entity handle on client=
	int     m_nImpulseCommand;      // Impulse command issued.
	Vector  m_vecViewAngles;        // Command view angles (local space)
	Vector  m_vecAbsViewAngles;     // Command view angles (world space)
	int     m_nButtons;             // Attack buttons.
	int     m_nOldButtons;          // From host_client->oldbuttons;
	float   m_flForwardMove;
	float   m_flSideMove;
	float   m_flUpMove;
	float   m_flMaxSpeed;
	float   m_flClientMaxSpeed;
	Vector  m_vecVelocity;          // edict::velocity        // Current movement direction.
	Vector  m_vecAngles;            // edict::angles
	Vector  m_vecOldAngles;
	float   m_outStepHeight;        // how much you climbed this move
	Vector  m_outWishVel;           // This is where you tried 
	Vector  m_outJumpVel;           // This is your jump velocity
	Vector  m_vecConstraintCenter;
	float   m_flConstraintRadius;
	float   m_flConstraintWidth;
	float   m_flConstraintSpeedFactor;
	std::byte pad[20];
	Vector  m_vecAbsOrigin;
};

class player_t;

class IGameMovement {
public:
	virtual						~IGameMovement(void) { }

	virtual void				ProcessMovement(player_t* pPlayer, CMoveData* pMove) = 0;
	virtual void				Reset(void) = 0;
	virtual void				StartTrackPredictionErrors(player_t* pPlayer) = 0;
	virtual void				FinishTrackPredictionErrors(player_t* pPlayer) = 0;
	virtual void				DiffPrint(char const* fmt, ...) = 0;
	virtual Vector const&		GetPlayerMins(bool ducked) const = 0;
	virtual Vector const&		GetPlayerMaxs(bool ducked) const = 0;
	virtual Vector const&		GetPlayerViewOffset(bool ducked) const = 0;
	virtual bool				IsMovingPlayerStuck(void) const = 0;
	virtual player_t*			GetMovingPlayer(void) const = 0;
	virtual void				UnblockPusher(player_t* pPlayer, player_t* pPusher) = 0;
	virtual void				SetupMovementBounds(CMoveData* pMove) = 0;

	std::byte pad[4];
	player_t* player;
	CMoveData* moveData;
	int oldWaterLevel;
	float waterEntryTime;
	int onLadder;
	Vector vecForward;
	Vector vecRight;
	Vector vecUp;
	int cachedGetPointContents[64][3];
	Vector cachedGetPointContentsPoint[64][3];
	int speedCropped;
	bool processingMovement;
	bool inStuckTest;
	float stuckCheckTime[64 + 1][2];
	void* traceListData;
	int traceCount;
};

class IPrediction
{
public:
	std::byte					pad[4];
	std::uintptr_t				m_iLastGround;
	bool						m_bInPrediction;
	bool						m_bOldCLPredictValue;
	bool						m_bIsFirstTimePredicted;
	bool						m_bEnginePaused;
	int							m_iPreviousStartFrame;
	int							m_iIncomingPacketNumber;
	float						m_flLastServerWorldTimeStamp;

	struct Split
	{
		bool						m_bIsFirstTimePredicted;
		std::byte					pad1[3];
		int							m_iCommandsPredicted;
		int							m_iServerCommandsAcknowledged;
		bool						m_bPreviousAckHadErrors;
		float						m_flIdealPitch;
		int							m_nLastCommandAcknowledged;
		bool						m_bPreviousAckErrorTriggersFullLatchReset;
		CUtlVector<std::uintptr_t>	m_vecEntitiesWithPredictionErrorsInLastAck;
		bool						m_bPerformedTickShift;
	};

	Split split[1];

	VIRTUAL_METHOD(void, Update, 3, (int startFrame, bool validFrame, int incAck, int outCmd), (this, startFrame, validFrame, incAck, outCmd))
	VIRTUAL_METHOD(void, SetLocalViewAngles, 13, (Vector& angles), (this, angles))
	VIRTUAL_METHOD(void, CheckMovingGround, 18, (entity_t* localPlayer, double frameTime), (this, localPlayer, frameTime))
	VIRTUAL_METHOD(void, RunCommand, 19, (player_t* localPlayer, CUserCmd* cmd, IMoveHelper* moveHelper), (this, localPlayer, cmd, moveHelper))
	VIRTUAL_METHOD(void, SetupMove, 20, (player_t* localPlayer, CUserCmd* cmd, IMoveHelper* moveHelper, CMoveData* moveData), (this, localPlayer, cmd, moveHelper, moveData))
	VIRTUAL_METHOD(void, FinishMove, 21, (player_t* localPlayer, CUserCmd* cmd, CMoveData* moveData), (this, localPlayer, cmd, moveData))
};