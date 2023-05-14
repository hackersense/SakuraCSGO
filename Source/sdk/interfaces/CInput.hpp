#pragma once

#include "../misc/CUserCmd.hpp"

#define MULTIPLAYER_BACKUP 150

class CInput
{
public:
	std::byte			pad[12];
	bool				m_bTrackIRAvailable;
	bool				m_bMouseInitialized;
	bool				m_bMouseActive;
	std::byte			pad1[154];
	bool				m_bCameraInThirdPerson;
	bool				m_bCameraMovingWithMouse;
	Vector				m_vecCameraOffset;
	bool				m_bCameraDistanceMove;
	int					m_nCameraOldX;
	int					m_nCameraOldY;
	int					m_nCameraX;
	int					m_nCameraY;
	bool				m_bCameraIsOrthographic;
	Vector				m_vecPreviousViewAngles;
	Vector				m_vecPreviousViewAnglesTilt;
	float				m_flLastForwardMove;
	int					m_nClearInputState;
	CUserCmd*			m_pCommands;
	CVerifiedUserCmd*	m_pVerifiedCommands;

	CUserCmd* GetUserCmd(int sequence_number)
	{
		return &m_pCommands[sequence_number % MULTIPLAYER_BACKUP];
	}

	CUserCmd* GetUserCmd(int slot, int sequence_number)
	{
		return &m_pCommands[slot, sequence_number % MULTIPLAYER_BACKUP];
	}

	CVerifiedUserCmd* GetVerifiedUserCmd(int sequence_number)
	{
		return &m_pVerifiedCommands[sequence_number % MULTIPLAYER_BACKUP];
	}

	CVerifiedUserCmd* GetVerifiedUserCmd(int slot, int sequence_number)
	{
		return &m_pVerifiedCommands[slot, sequence_number % MULTIPLAYER_BACKUP];
	}
};