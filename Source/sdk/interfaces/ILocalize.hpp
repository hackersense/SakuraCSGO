#pragma once

#include "IAppSystem.hpp"
#include "..\misc\vfunc.hpp"

class ILocalizeTextQuery
{
public:
	virtual int ComputeTextWidth(const wchar_t* pString) = 0;
};

class ILocalizationChangeCallback
{
public:
	virtual void OnLocalizationChanged() = 0;
};

class ILocalize : public IAppSystem
{
public:
	virtual bool					AddFile(const char* fileName, const char* pPathID = nullptr, bool bIncludeFallbackSearchPaths = false) = 0;
	virtual void					RemoveAll() = 0;
	virtual wchar_t*				Find(const char* tokenName) = 0;
	virtual const wchar_t*			FindSafe(const char* tokenName) = 0;
	virtual int						ConvertANSIToUnicode(const char* ansi, wchar_t* unicode, int unicodeBufferSizeInBytes) = 0;
	virtual int						ConvertUnicodeToANSI(const wchar_t* unicode, char* ansi, int ansiBufferSize) = 0;
	virtual unsigned				FindIndex(const char* tokenName) = 0;
	virtual void					ConstructString(wchar_t* unicodeOuput, int unicodeBufferSizeInBytes, const wchar_t* formatString, int numFormatParameters, ...) = 0;
	virtual const char*				GetNameByIndex(unsigned index) = 0;
	virtual wchar_t*				GetValueByIndex(unsigned index) = 0;
};

class IBaseFileSystem
{
public:
	int Read(void* pOutput, int size, void* file)
	{
		using Fn = int(__thiscall*)(void*, void*, int, void*);
		return call_virtual<Fn>(this, 0)(this, pOutput, size, file);
	}

	void* Open(const char* pFileName, const char* pOptions, const char* pathID = nullptr)
	{
		using Fn = void* (__thiscall*)(void*, const char*, const char*, const char*);
		return call_virtual<Fn>(this, 2)(this, pFileName, pOptions, pathID);
	}

	void Close(void* file)
	{
		using Fn = void(__thiscall*)(void*, void*);
		return call_virtual<Fn>(this, 3)(this, file);
	}

	unsigned int Size(void* file)
	{
		using Fn = unsigned int(__thiscall*)(void*, void*);
		return call_virtual<Fn>(this, 7)(this, file);
	}
};

class IFileSystem
{
public:
	virtual ~IFileSystem() {}
};

class IBaseInterface
{
public:
	virtual	~IBaseInterface() {}
};

enum soundlevel_t
{
	SNDLVL_NONE = 0,
	SNDLVL_20dB = 20, // rustling leaves
	SNDLVL_25dB = 25, // whispering
	SNDLVL_30dB = 30, // library
	SNDLVL_35dB = 35,
	SNDLVL_40dB = 40,
	SNDLVL_45dB = 45, // refrigerator
	SNDLVL_50dB = 50, // 3.9 // average home
	SNDLVL_55dB = 55, // 3.0
	SNDLVL_IDLE = 60, // 2.0
	SNDLVL_60dB = 60, // 2.0 // normal conversation, clothes dryer
	SNDLVL_65dB = 65, // 1.5 // washing machine, dishwasher
	SNDLVL_STATIC = 66, // 1.25
	SNDLVL_70dB = 70, // 1.0 // car, vacuum cleaner, mixer, electric sewing machine
	SNDLVL_NORM = 75,
	SNDLVL_75dB = 75, // 0.8 // busy traffic
	SNDLVL_80dB = 80, // 0.7 // mini-bike, alarm clock, noisy restaurant, office tabulator, outboard motor, passing snowmobile
	SNDLVL_TALKING = 80, // 0.7
	SNDLVL_85dB = 85, // 0.6 // average factory, electric shaver
	SNDLVL_90dB = 90, // 0.5 // screaming child, passing motorcycle, convertible ride on frw
	SNDLVL_95dB = 95,
	SNDLVL_100dB = 100, // 0.4 // subway train, diesel truck, woodworking shop, pneumatic drill, boiler shop, jackhammer
	SNDLVL_105dB = 105, // helicopter, power mower
	SNDLVL_110dB = 110, // snowmobile drvrs seat, inboard motorboat, sandblasting
	SNDLVL_120dB = 120, // auto horn, propeller aircraft
	SNDLVL_130dB = 130, // air raid siren
	SNDLVL_GUNFIRE = 140, // 0.27 // THRESHOLD OF PAIN, gunshot, jet engine
	SNDLVL_140dB = 140, // 0.2
	SNDLVL_150dB = 150, // 0.2
	SNDLVL_180dB = 180, // rocket launching
	// NOTE: Valid soundlevel_t values are 0-255.
	// 256-511 are reserved for sounds using goldsrc compatibility attenuation.
};

class IEngineToolFramework : public IBaseInterface
{
public:
	virtual int			GetToolCount() const = 0;
	virtual char const* GetToolName(int index) const = 0;
	virtual void		SwitchToTool(int index) = 0;

	virtual bool		IsTopmostTool(void* sys) const = 0;

	virtual void*		GetToolSystem(int index) const = 0;
	virtual void*		GetTopmostTool() = 0;

	virtual void		ShowCursor(bool show) = 0;
	virtual bool		IsCursorVisible() const = 0;

	virtual bool		LoadToolModule(char const* pToolModule, bool bSwitchToFirst) = 0;
};

class IEngineTool : public IEngineToolFramework
{
public:
	virtual void		GetServerFactory(CreateInterfaceFn& factory) = 0;
	virtual void		GetClientFactory(CreateInterfaceFn& factory) = 0;
	virtual float		GetSoundDuration(const char* pszName) = 0;
	virtual bool		IsSoundStillPlaying(int guid) = 0;

	virtual int			StartSound(
		int iUserData,
		bool staticsound,
		int iEntIndex,
		int iChannel,
		const char* pSample,
		float flVolume,
		soundlevel_t iSoundlevel,
		const Vector& origin,
		const Vector& direction,
		int iFlags = 0,
		int iPitch = 0,
		bool bUpdatePositions = true,
		float delay = 0.0f,
		int speakerentity = -1) = 0;

	virtual void		StopSoundByGuid(int guid) = 0;

	virtual void		SetVolumeByGuid(int guid, float flVolume) = 0;

	virtual float		GetSoundDuration(int guid) = 0;

	virtual bool		IsLoopingSound(int guid) = 0;
	virtual void		ReloadSound(const char* pSample) = 0;
	virtual void		StopAllSounds() = 0;
	virtual float		GetMono16Samples(const char* pszName, CUtlVector< short >& sampleList) = 0;
	virtual void		SetAudioState(const AudioState_t& audioState) = 0;

	virtual void		Command(char const* cmd) = 0;

	virtual void		Execute() = 0;

	virtual char const* GetCurrentMap() = 0;
	virtual void		ChangeToMap(char const* mapname) = 0;
	virtual bool		IsMapValid(char const* mapname) = 0;

	virtual void		RenderView(CViewSetup& view, int nFlags, int whatToRender) = 0;

	virtual bool		IsInGame() = 0;

	virtual bool		IsConnected() = 0;

	virtual int			GetMaxClients() = 0;

	virtual bool		IsGamePaused() = 0;
	virtual void		SetGamePaused(bool paused) = 0;

	virtual float		GetTimescale() = 0;
	virtual void		SetTimescale(float scale) = 0;

	virtual float		GetRealTime() = 0;
	virtual float		GetRealFrameTime() = 0;

	virtual float		Time() = 0;

	virtual float		HostFrameTime() = 0;
	virtual float		HostTime() = 0;
	virtual int			HostTick() = 0;
	virtual int			HostFrameCount() = 0;

	virtual float		ServerTime() = 0;
	virtual float		ServerFrameTime() = 0;
	virtual int			ServerTick() = 0;
	virtual float		ServerTickInterval() = 0;

	virtual float		ClientTime() = 0;
	virtual float		ClientFrameTime() = 0;
	virtual int			ClientTick() = 0;

	virtual void		SetClientFrameTime(float frametime) = 0;

	virtual void		ForceUpdateDuringPause() = 0;

	virtual model_t*	GetModel(void* hEntity) = 0;

	virtual StudioHdr*  GetStudioModel(void* hEntity) = 0;

	virtual void		Con_NPrintf(int pos, const char* fmt, ...) = 0;

	virtual void		Con_NXPrintf(const struct con_nprint_s* info, const char* fmt, ...) = 0;

	virtual void        GetGameDir(char* szGetGameDir, int maxlength) = 0;

	virtual void		GetScreenSize(int& width, int& height) = 0;

	virtual void		SetMainView(const Vector& vecOrigin, const QAngle& angles) = 0;

	virtual bool		GetPlayerView(CViewSetup& playerView, int x, int y, int w, int h) = 0;

	virtual void		CreatePickingRay(const CViewSetup& viewSetup, int x, int y, Vector& org, Vector& forward) = 0;

	virtual bool		PrecacheSound(const char* pName, bool bPreload = false) = 0;
	virtual bool		PrecacheModel(const char* pName, bool bPreload = false) = 0;
};
