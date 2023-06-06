#pragma once

#include "..\sdk\interfaces\IVEngineClient.hpp"
#include "..\sdk\interfaces\IInputSystem.hpp"
#include "..\sdk\interfaces\IBaseClientDll.hpp"
#include "..\sdk\interfaces\IClientEntityList.hpp"
#include "..\sdk\interfaces\IClientMode.hpp"
#include "..\sdk\interfaces\ICvar.hpp"
#include "..\sdk\interfaces\IEngineTrace.hpp"
#include "..\sdk\interfaces\IEngineSound.hpp"
#include "..\sdk\interfaces\IInputInternal.hpp"
#include "..\sdk\interfaces\IVStudioRender.hpp"
#include "..\sdk\interfaces\IRenderView.hpp"
#include "..\sdk\interfaces\IVModelRender.hpp"
#include "..\sdk\interfaces\IMaterialSystem.hpp"
#include "..\sdk\interfaces\IPanel.hpp"
#include "..\sdk\interfaces\IVModelInfoClient.hpp"
#include "..\sdk\interfaces\IMDLCache.hpp"
#include "..\sdk\interfaces\IMemAlloc.h"
#include "..\sdk\interfaces\CClientState.hpp"
#include "..\sdk\interfaces\IPrediction.hpp"
#include "..\sdk\interfaces\IMoveHelper.hpp"
#include "..\sdk\interfaces\CInput.hpp"
#include "..\sdk\interfaces\ISurface.hpp"
#include "..\sdk\interfaces\IVDebugOverlay.hpp"
#include "..\sdk\interfaces\IViewRenderBeams.hpp"
#include "..\sdk\interfaces\IPhysics.hpp"
#include "..\sdk\interfaces\ILocalize.hpp"
#include "..\sdk\interfaces\IPanoramaUIEngine.hpp"
#include "..\sdk\misc\GlobalVars.hpp"
#include "..\sdk\misc\glow_outline_effect.hpp"
#include "..\sdk\interfaces\IGameEventManager.hpp"
#include "..\sdk\interfaces\CSPlayerResource.h"
#include "..\sdk\interfaces\IWeaponSystem.h"
#include "..\sdk\new\MaterialSystem.h"
#include <d3dx9.h>
#include <d3d9.h>

class InterfaceReg
{
private:
	using InstantiateInterfaceFn = void*(*)();
public:
	InstantiateInterfaceFn m_CreateFn;
	const char *m_pName;
	InterfaceReg *m_pNext;
};

class C_CSGO
{
public:
	IDirect3DDevice9* m_device();
	IVEngineClient* m_engine();
	IInputSystem* m_inputsystem();
	IBaseClientDLL* m_client();
	IClientEntityList* m_entitylist();
	ICvar* m_cvar();
	IEngineTrace* m_trace();
	VSoundEmitter* m_soundemitter();
	IEngineSound* m_enginesound();
	IInputInternal* m_inputinternal();
	IVStudioRender* m_studiorender();
	IVRenderView* m_renderview();
	IMDLCache* m_modelcache();
	IVModelRender* m_modelrender();
	IPanoramaUIEngine* m_panoramauiengine();
	IMaterialSystem* m_materialsystem();
	MaterialSystem* m_materialsystem2();
	IPanel* m_panel();
	IVModelInfoClient* m_modelinfo();
	IPrediction* m_prediction();
	IGameMovement* m_gamemovement();
	ISurface* m_surface();
	IVDebugOverlay* m_debugoverlay();
	IPhysicsSurfaceProps* m_physsurface();
	IGameEventManager* m_eventmanager();
	IViewRenderBeams* m_viewrenderbeams();
	IMemAlloc* m_memalloc();
	IClientMode* m_clientmode();
	CGlobalVarsBase* m_globals();
	CGlowObjectManager* m_glow();
	CClientState* m_clientstate();
	IMoveHelper* m_movehelper();
	CMoveData* m_movedata();
	CInput* m_input();
	CSPlayerResource* m_playerresource();
	CSGameRulesProxy* m_gamerules();
	ILocalize* m_localize();
	IEngineTool* m_enginetool();
	IBaseFileSystem* m_basefilesystem();
	IFileSystem* m_filesystem();
	IWeaponSystem* m_weaponsystem();
	DWORD m_postprocessing();
	DWORD m_ccsplayerrenderablevftable();
private:
	IDirect3DDevice9 * p_device = nullptr;
	IVEngineClient* p_engine = nullptr;
	IInputSystem * p_inputsystem = nullptr;
	IBaseClientDLL * p_client = nullptr;
	IClientEntityList * p_entitylist = nullptr;
	IEngineTrace * p_trace = nullptr;
	VSoundEmitter* p_soundemitter = nullptr;
	IEngineSound * p_enginesound = nullptr;
	ICvar * p_cvar = nullptr;
	IInputInternal* p_inputinternal = nullptr;
	IVStudioRender* p_studiorender = nullptr;
	IVRenderView * p_renderview = nullptr;
	IMDLCache* p_modelcache = nullptr;
	IVModelRender* p_modelrender = nullptr;
	IPanoramaUIEngine* p_panoramauiengine = nullptr;
	IMaterialSystem* p_materialsystem = nullptr;
	MaterialSystem* p_materialsystem2 = nullptr;
	IPanel * p_panel = nullptr;
	IVModelInfoClient * p_modelinfo = nullptr;
	IPrediction * p_prediciton = nullptr;
	IGameMovement * p_gamemovement = nullptr;
	ISurface * p_surface = nullptr;
	IVDebugOverlay * p_debugoverlay = nullptr;
	IPhysicsSurfaceProps * p_physsurface = nullptr;
	IGameEventManager * p_eventmanager = nullptr;
	IViewRenderBeams * p_viewrenderbeams = nullptr;
	IMemAlloc * p_memalloc = nullptr;
	IClientMode * p_clientmode = nullptr;
	CGlobalVarsBase * p_globals = nullptr;
	CGlowObjectManager * p_glow = nullptr;
	CClientState * p_clientstate = nullptr;
	IMoveHelper* p_movehelper = nullptr;
	CMoveData* p_movedata = nullptr;
	CInput * p_input = nullptr;
	CSPlayerResource * p_playerresource = nullptr;
	CSGameRulesProxy * p_gamerules = nullptr;
	ILocalize* p_localize = nullptr;
	IEngineTool* p_enginetool = nullptr;
	IBaseFileSystem* p_basefilesystem = nullptr;
	IFileSystem* p_filesystem = nullptr;
	IWeaponSystem* p_weaponsystem = nullptr;
	DWORD p_postprocessing = 0;
	DWORD p_ccsplayerrenderablevftable = 0;
};

extern C_CSGO g_csgo;