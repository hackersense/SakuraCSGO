#pragma once

#include "../math/Vector.hpp"
#include "../misc/UtlVector.hpp"

typedef void* FileNameHandle_t;
enum soundlevel_t;

struct SoundInfo_t
{
	std::byte	pad[40];
	float		m_flVolume;
	std::byte	pad1[12];
	int			m_nEntityIndex;
	int			m_nChannel;
	int			m_nPitch;
	int			m_nFlags;
	int			m_nSoundIndex;
};

/*
struct SoundInfo2_t
{
	int				nSequenceNumber;
	int				nEntityIndex;
	int				nChannel;
	const char*		pszName;		// UNDONE: Make this a FilenameHandle_t to avoid bugs with arrays of these
	Vector			vOrigin;
	Vector			vDirection;
	float			fVolume;
	soundlevel_t	Soundlevel;
	bool			bLooping;
	int				nPitch;
	int				nSpecialDSP;
	Vector			vListenerOrigin;
	int				nFlags;
	int 			nSoundNum;
	float			fDelay;
	bool			bIsSentence;
	bool			bIsAmbient;
	int				nSpeakerEntity;
};
*/

struct SoundParams_t
{
	void*					m_nFilter;
	int						m_nEntityIndex;
	int						m_nChannel;
	const char*				m_nSoundEntry;
	int						m_nSoundEntryHash;
	const char*				m_nSample;
	float					m_flVolume;
	int						m_nSeed;
	int						m_nSoundLevel;
	int						m_nFlags;
	int						m_nPitch;
	const Vector&			m_vOrigin;
	const Vector&			m_vDirection;
	SafeCUtlVector<Vector>* m_vOrigins;
	bool					m_bUpdatePosition;
	float					m_flSoundTime;
	int						m_nSpeakerEntity;
	void*					m_nSoundParams;
};

struct ActiveChannels_t
{
	int		m_nCount;
	short	m_sList[128];
};

struct Channel_t
{
	std::byte	pad[244];
	int			m_nSoundSource;
	std::byte	pad1[56];
	Vector		m_pOrigin;
	Vector		m_pDirection;
	std::byte	pad2[80];
};

struct ActiveSoundInfo_t
{
	int					m_nGuid;
	void*				m_nFileNameHandle;
	int					m_nSoundSource;
	int					m_nChannel;
	int					m_nSpeakerEntity;
	float				m_flVolume;
	float				m_flLastSpatializedVolume;
	float				m_flRadius;
	int					m_nPitch;
	Vector*				m_pOrigin;
	Vector*				m_pDirection;
	bool				m_bUpdatePositions;
	bool				m_bIsSentence;
	bool				m_bDryMix;
	bool				m_bSpeaker;
	bool				m_bFromServer;
};

class VSoundEmitter
{
public:
	const char* GetSoundName(int index)
	{
		using SoundNameFn = const char*(__thiscall*)(void*, int);
		return call_virtual<SoundNameFn>(this, 46)(this, index);
	}
};

class IEngineSound
{
public:
	int EmitSound(SoundParams_t data)
	{
		using EmitSoundFn = int(__thiscall*)(void*, SoundParams_t);
		return call_virtual<EmitSoundFn>(this, 5)(this, data);
	}

	void GetActiveSounds(SafeCUtlVector<ActiveSoundInfo_t>& soundList)
	{
		using GetActiveSoundsFn = void(__thiscall*)(void*, SafeCUtlVector<ActiveSoundInfo_t>&);
		return call_virtual<GetActiveSoundsFn>(this, 19)(this, soundList);
	}
};
