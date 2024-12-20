#pragma once

#include <cstdint>

#include "..\math\Vector.hpp"
#include "..\misc\BitBuffer.h"

class CClockDriftMgr {
public:
	float	m_flClockOffsets[16];
	int		m_iCurClockOffset;
	int		m_iServerTick;
	int		m_iClientTick;
};

class INetChannel
{
public:
	std::byte	pad[24];
	int			m_nOutSequenceNr;
	int			m_nInSequenceNr;
	int			m_nOutSequenceNrAck;
	int			m_nOutReliableState;
	int			m_nInReliableState;
	int			m_nChokedPackets;

	VIRTUAL_METHOD(void, SetTimeOut, 4, (float seconds), (this, seconds))
	VIRTUAL_METHOD(float, GetLatency, 9, (int flow), (this, flow))
	VIRTUAL_METHOD(bool, SendNetMsg, 40, (void* msg, bool forceReliable = false, bool voice = false), (this, msg, forceReliable, voice))
	VIRTUAL_METHOD(int, SendDatagram, 46, (), (this, 0))
	VIRTUAL_METHOD(bool, Transmit, 49, (bool onlyReliable), (this, onlyReliable))
	VIRTUAL_METHOD(int, RequestFile, 62, (const char* fileName), (this, fileName))
};

class INetMessage
{
public:
	virtual					~INetMessage() {};

	// Use these to setup who can hear whose voice.
	// Pass in client indices (which are their ent indices - 1).

	virtual void			SetNetChannel(INetChannel* netchan) = 0; // netchannel this message is from/for
	virtual void			SetReliable(bool state) = 0;	// set to true if it's a reliable message

	virtual bool			Process(void) = 0; // calles the recently set handler to process this message

	virtual	bool			ReadFromBuffer(bufferRead& buffer) = 0; // returns true if parsing was OK
	virtual	bool			WriteToBuffer(bufferWrite& buffer) = 0;	// returns true if writing was OK

	virtual bool			IsReliable(void) const = 0;  // true, if message needs reliable handling

	virtual int				GetType(void) const = 0; // returns module specific header tag eg svc_serverinfo
	virtual int				GetGroup(void) const = 0;	// returns net message group of this message
	virtual const char*		GetName(void) const = 0;	// returns network message name, eg "svc_serverinfo"
	virtual INetChannel*	GetNetChannel(void) const = 0;
	virtual const char*		ToString(void) const = 0; // returns a human readable string about message content
};

class EventInfo {
public:
	enum {
		EVENT_INDEX_BITS = 8,
		EVENT_DATA_LEN_BITS = 11,
		MAX_EVENT_DATA = 192,  // ( 1<<8 bits == 256, but only using 192 below )
	};

	short					m_sClassID;
	float					m_flFireDelay;
	const void*				m_pSendTable;
	const ClientClass*		m_pClientClass;
	int						m_iBits;
	uint8_t*				m_iData;
	int						m_iFlags;
	std::byte				pad[0x1C];
	EventInfo*				m_pNext;
};

class CClientState
{
public:
	void ForceFullUpdate()
	{
		m_iDeltaTick = -1;
	}

	std::byte			pad[0x9C];
	INetChannel*		m_pNetChannel;
	int					m_iChallengeNr;
	std::byte			pad1[4];
	double				m_dConnectTime;
	int					m_iRetryNumber;
	std::byte			pad2[0x54];
	int					m_iSignonState;
	std::byte			pad3[8];
	float				m_flNextCmdTime;
	int					m_iServerCount;
	int					m_iCurrentSequence;
	std::byte           pad4[8];
	CClockDriftMgr		m_iClockDriftMgr;
	int					m_iDeltaTick;
	bool				m_bPaused;
	std::byte			pad5[7];
	int					m_iViewEntity;
	int					m_iPlayerSlot;
	char				m_szLevelName[260];
	char				m_szLevelNameShort[80];
	char				m_szGroupName[80];
	char				m_szLastLevelNameShort[80];
	std::byte			pad6[0xC];
	int					m_iMaxClients;
	std::byte			pad7[0xFF3];
	std::uint32_t		m_iStringTableContainer;
	std::byte			pad8[0x3991];
	float				m_flLastServerTickTime;
	bool				m_bInSimulation;
	std::byte			pad9[3];
	int					m_iOldTickcount;
	float				m_flTickRemainder;
	float				m_flFrameTime;
	int					m_nLastOutgoingCommand;
	int					m_iChokedCommands;
	int					m_nLastCommandAck;
	int					m_iCommandAck;
	int					m_iSoundSequence;
	int					m_mLastProgressPercent;
	bool				m_mIsHLTV;
	std::byte			pad10[0x4B];
	Vector				m_vAngViewPoint;
	std::byte			pad11[0xD0];
	EventInfo*			m_pEvents;
};
