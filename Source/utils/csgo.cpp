#include "csgo.hpp"
#include "..\includes.hpp"

C_CSGO g_csgo;

#undef m_client
#undef m_clientmode
#undef m_clientstate
#undef m_cvar
#undef m_cvar2
#undef m_debugoverlay
#undef m_device
#undef m_engine
#undef m_soundemitter
#undef m_enginesound
#undef m_panoramauiengine
#undef m_entitylist
#undef m_eventmanager
#undef m_gamemovement
#undef m_gamerules
#undef m_globals
#undef m_glow
#undef m_input
#undef m_inputinternal
#undef m_studiorender
#undef m_inputsystem
#undef m_localize
#undef m_enginetool
#undef m_materialsystem
#undef m_materialsystem2
#undef m_memalloc
#undef m_modelcache
#undef m_modelinfo
#undef m_modelrender
#undef m_movehelper
#undef m_movedata
#undef m_panel
#undef m_physsurface
#undef m_playerresource
#undef m_postprocessing
#undef m_prediction
#undef m_renderview
#undef m_surface
#undef m_trace
#undef m_viewrenderbeams
#undef m_soundservices
#undef m_basefilesystem
#undef m_filesystem
#undef m_weaponsystem

template<typename T>
static T* get_interface(const char* module_name, const char* interface_name, bool exact = false) {
	T* iface = nullptr;
	InterfaceReg* register_list;
	int part_match_len = strlen(interface_name);

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(module_name), crypt_str("CreateInterface")));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

	for (InterfaceReg* cur = register_list; cur; cur = cur->m_pNext) {
		if (exact) {
			if (!strcmp(cur->m_pName, interface_name))
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	if (iface == nullptr && cheat.isDebugMode)
		MessageBoxA(nullptr, ("Failed to find " + std::string(interface_name) + " interface!").c_str(), cheat.cheatname.c_str(), MB_OK | MB_ICONERROR);
	return iface;
}

template<typename T>
static T* getInterface(const char* module_name, const char* interfaceName)
{
	if (const auto createInterface = reinterpret_cast<std::add_pointer_t<void* __cdecl(const char* name, int* returnCode)>>(
		GetProcAddress(GetModuleHandleA(module_name), crypt_str("CreateInterface"))
		)) {
		if (void* foundInterface = createInterface(interfaceName, nullptr))
			return reinterpret_cast<T*>(foundInterface);
	}
	if (cheat.isDebugMode)
		MessageBoxA(nullptr, ("Failed to find " + std::string(interfaceName) + " interface!").c_str(), cheat.cheatname.c_str(), MB_OK | MB_ICONERROR);
}

IDirect3DDevice9* C_CSGO::m_device() {
	if (!p_device)
		p_device = **reinterpret_cast<IDirect3DDevice9***>(util::FindSignature(crypt_str("shaderapidx9.dll"), crypt_str("A1 ? ? ? ? 50 8B 08 FF 51 0C")) + 1);

	return p_device;
}

IVEngineClient* C_CSGO::m_engine() {
	if (!p_engine)
		p_engine = get_interface<IVEngineClient>(crypt_str("engine.dll"), crypt_str("VEngineClient"));

	return p_engine;
}

IInputSystem* C_CSGO::m_inputsystem() {
	if (!p_inputsystem)
		p_inputsystem = get_interface<IInputSystem>(crypt_str("inputsystem.dll"), crypt_str("InputSystemVersion"));

	return p_inputsystem;
}

IBaseClientDLL* C_CSGO::m_client() {
	if (!p_client)
		p_client = getInterface<IBaseClientDLL>(crypt_str("client.dll"), crypt_str("VClient018"));

	return p_client;
}

IClientEntityList* C_CSGO::m_entitylist() {
	if (!p_entitylist)
		p_entitylist = get_interface<IClientEntityList>(crypt_str("client.dll"), crypt_str("VClientEntityList"));

	return p_entitylist;
}

ICvar* C_CSGO::m_cvar() {
	if (!p_cvar)
		p_cvar = get_interface<ICvar>(crypt_str("vstdlib.dll"), crypt_str("VEngineCvar"));

	return p_cvar;
}

NEWCvar* C_CSGO::m_cvar2() {
	if (!p_cvar2)
		p_cvar2 = get_interface<NEWCvar>(crypt_str("vstdlib.dll"), crypt_str("VEngineCvar"));

	return p_cvar2;
}

IEngineTrace* C_CSGO::m_trace() {
	if (!p_trace)
		p_trace = get_interface<IEngineTrace>(crypt_str("engine.dll"), crypt_str("EngineTraceClient"));

	return p_trace;
}

VSoundEmitter* C_CSGO::m_soundemitter() {
	if (!p_soundemitter)
		p_soundemitter = get_interface<VSoundEmitter>(crypt_str("soundemittersystem.dll"), crypt_str("VSoundEmitter"));

	return p_soundemitter;
}

IEngineSound* C_CSGO::m_enginesound() {
	if (!p_enginesound)
		p_enginesound = get_interface<IEngineSound>(crypt_str("engine.dll"), crypt_str("IEngineSoundClient"));

	return p_enginesound;
}

IInputInternal* C_CSGO::m_inputinternal() {
	if (!p_inputinternal)
		p_inputinternal = get_interface<IInputInternal>(crypt_str("vgui2.dll"), crypt_str("VGUI_InputInternal"));

	return p_inputinternal;
}

IVStudioRender* C_CSGO::m_studiorender() {
	if (!p_studiorender)
		p_studiorender = getInterface<IVStudioRender>(crypt_str("studiorender.dll"), crypt_str("VStudioRender026"));

	return p_studiorender;
}

IVRenderView* C_CSGO::m_renderview() {
	if (!p_renderview)
		p_renderview = get_interface<IVRenderView>(crypt_str("engine.dll"), crypt_str("VEngineRenderView"));

	return p_renderview;
}

IMDLCache* C_CSGO::m_modelcache() {
	if (!p_modelcache)
		p_modelcache = get_interface<IMDLCache>(crypt_str("datacache.dll"), crypt_str("MDLCache"));

	return p_modelcache;
}

IVModelRender* C_CSGO::m_modelrender() {
	if (!p_modelrender)
		p_modelrender = get_interface<IVModelRender>(crypt_str("engine.dll"), crypt_str("VEngineModel"));

	return p_modelrender;
}

IPanoramaUIEngine* C_CSGO::m_panoramauiengine() {
	if (!p_panoramauiengine)
		p_panoramauiengine = get_interface<IPanoramaUIEngine>(crypt_str("panorama.dll"), crypt_str("PanoramaUIEngine"));

	return p_panoramauiengine;
}

IMaterialSystem* C_CSGO::m_materialsystem() {
	if (!p_materialsystem)
		p_materialsystem = getInterface<IMaterialSystem>(crypt_str("materialsystem.dll"), crypt_str("VMaterialSystem080"));

	return p_materialsystem;
}

MaterialSystem* C_CSGO::m_materialsystem2() {
	if (!p_materialsystem2)
		p_materialsystem2 = getInterface<MaterialSystem>(crypt_str("materialsystem.dll"), crypt_str("VMaterialSystem080"));

	return p_materialsystem2;
}

IPanel* C_CSGO::m_panel() {
	if (!p_panel)
		p_panel = get_interface<IPanel>(crypt_str("vgui2.dll"), crypt_str("VGUI_Panel"));

	return p_panel;
}

IVModelInfo* C_CSGO::m_modelinfo() {
	if (!p_modelinfo)
		p_modelinfo = get_interface<IVModelInfo>(crypt_str("engine.dll"), crypt_str("VModelInfoClient"));

	return p_modelinfo;
}

IPrediction* C_CSGO::m_prediction() {
	if (!p_prediciton)
		p_prediciton = get_interface<IPrediction>(crypt_str("client.dll"), crypt_str("VClientPrediction"));

	return p_prediciton;
}

IGameMovement* C_CSGO::m_gamemovement() {
	if (!p_gamemovement)
		p_gamemovement = get_interface<IGameMovement>(crypt_str("client.dll"), crypt_str("GameMovement"));

	return p_gamemovement;
}

ISurface* C_CSGO::m_surface() {
	if (!p_surface)
		p_surface = get_interface<ISurface>(crypt_str("vguimatsurface.dll"), crypt_str("VGUI_Surface"));

	return p_surface;
}

IVDebugOverlay* C_CSGO::m_debugoverlay() {
	if (!p_debugoverlay)
		p_debugoverlay = get_interface<IVDebugOverlay>(crypt_str("engine.dll"), crypt_str("VDebugOverlay"));

	return p_debugoverlay;
}

IPhysicsSurfaceProps* C_CSGO::m_physsurface() {
	if (!p_physsurface)
		p_physsurface = get_interface<IPhysicsSurfaceProps>(crypt_str("vphysics.dll"), crypt_str("VPhysicsSurfaceProps"));

	return p_physsurface;
}

IGameEventManager* C_CSGO::m_eventmanager() {
	if (!p_eventmanager)
		p_eventmanager = getInterface<IGameEventManager>(crypt_str("engine.dll"), crypt_str("GAMEEVENTSMANAGER002"));

	return p_eventmanager;
}

IViewRenderBeams* C_CSGO::m_viewrenderbeams() {
	if (!p_viewrenderbeams)
		p_viewrenderbeams = *reinterpret_cast<IViewRenderBeams**>(util::FindSignature(crypt_str("client.dll"), crypt_str("B9 ?? ?? ?? ?? A1 ?? ?? ?? ?? FF 10 A1 ?? ?? ?? ?? B9")) + 1);

	return p_viewrenderbeams;
}

IMemAlloc* C_CSGO::m_memalloc() {
	if (!p_memalloc)
		p_memalloc = *reinterpret_cast<IMemAlloc**>(GetProcAddress(GetModuleHandleW(crypt_str(L"tier0.dll")), crypt_str("g_pMemAlloc")));

	return p_memalloc;
}

IClientMode* C_CSGO::m_clientmode() {
	if (!p_clientmode)
		p_clientmode = **reinterpret_cast<IClientMode***>((*reinterpret_cast<uintptr_t**>(m_client()))[10] + 5);

	return p_clientmode;
}

CGlobalVarsBase* C_CSGO::m_globals() {
	if (!p_globals)
		p_globals = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(m_client()))[11] + 0xA);
	//p_globals = **reinterpret_cast<CGlobalVarsBase***>((*reinterpret_cast<uintptr_t**>(m_client()))[0] + 0x1F);

	return p_globals;
}

CGlowObjectManager* C_CSGO::m_glow() {
	if (!p_glow)
		p_glow = *reinterpret_cast<CGlowObjectManager**>(util::FindSignature(crypt_str("client.dll"), crypt_str("0F 11 05 ?? ?? ?? ?? 83 C8 01")) + 3);

	return p_glow;
}

CClientState* C_CSGO::m_clientstate() {
	if (!p_clientstate)
		p_clientstate = **reinterpret_cast<CClientState***>((*reinterpret_cast<uintptr_t**> (m_engine()))[12] + 0x10);

	return p_clientstate;
}

IMoveHelper* C_CSGO::m_movehelper() {
	if (!p_movehelper)
		p_movehelper = **reinterpret_cast<IMoveHelper***>(util::FindSignature(crypt_str("client.dll"), crypt_str("8B 0D ?? ?? ?? ?? 8B 46 08 68")) + 2);

	return p_movehelper;
}

CMoveData* C_CSGO::m_movedata() {
	if (!p_movedata)
		p_movedata = **reinterpret_cast<CMoveData***>(util::FindSignature(crypt_str("client.dll"), crypt_str("A1 ? ? ? ? F3 0F 59 CD")) + 1);

	return p_movedata;
}

CInput* C_CSGO::m_input() {
	if (!p_input)
		p_input = *reinterpret_cast<CInput**>((*reinterpret_cast<uintptr_t**>(m_client()))[16] + 1);
		//p_input = *reinterpret_cast<CInput**>(util::FindSignature(crypt_str("client.dll"), crypt_str("B9 ? ? ? ? F3 0F 11 04 24 FF 50 10")) + 1);

	return p_input;
}

CSPlayerResource* C_CSGO::m_playerresource()
{
	if (!p_playerresource)
		p_playerresource = **reinterpret_cast<CSPlayerResource***>(util::FindSignature(crypt_str("client.dll"), crypt_str("74 30 8B 35 ? ? ? ? 85 F6")) + 4);
	else if (cheat.globals.should_update_playerresource)
	{
		cheat.globals.should_update_playerresource = false;
		p_playerresource = **reinterpret_cast<CSPlayerResource***>(util::FindSignature(crypt_str("client.dll"), crypt_str("74 30 8B 35 ? ? ? ? 85 F6")) + 4);
	}

	return p_playerresource;
}

CSGameRulesProxy* C_CSGO::m_gamerules()
{
	if (!p_gamerules)
		p_gamerules = **reinterpret_cast<CSGameRulesProxy***>(util::FindSignature(crypt_str("client.dll"), crypt_str("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6")) + 1);
	else if (cheat.globals.should_update_gamerules)
	{
		cheat.globals.should_update_gamerules = false;
		p_gamerules = **reinterpret_cast<CSGameRulesProxy***>(util::FindSignature(crypt_str("client.dll"), crypt_str("A1 ? ? ? ? 8B 0D ? ? ? ? 6A 00 68 ? ? ? ? C6")) + 1);
	}

	return p_gamerules;
}

IBaseFileSystem* C_CSGO::m_basefilesystem()
{
	if (!p_basefilesystem)
		p_basefilesystem = getInterface<IBaseFileSystem>(crypt_str("filesystem_stdio.dll"), crypt_str("VBaseFileSystem011"));

	return p_basefilesystem;
}

IFileSystem* C_CSGO::m_filesystem()
{
	if (!p_filesystem)
		p_filesystem = getInterface<IFileSystem>(crypt_str("filesystem_stdio.dll"), crypt_str("VFileSystem017"));

	return p_filesystem;
}

ILocalize* C_CSGO::m_localize()
{
	if (!p_localize)
		p_localize = getInterface<ILocalize>(crypt_str("localize.dll"), crypt_str("Localize_001"));

	return p_localize;
}

IEngineTool* C_CSGO::m_enginetool()
{
	if (!p_enginetool)
		p_enginetool = get_interface<IEngineTool>(crypt_str("engine.dll"), crypt_str("VEngineTool"));

	return p_enginetool;
}

DWORD C_CSGO::m_postprocessing() {
	if (!p_postprocessing)
		p_postprocessing = *(DWORD*)(util::FindSignature(crypt_str("client.dll"), crypt_str("80 3D ? ? ? ? ? 53 56 57 0F 85")) + 2);

	return p_postprocessing;
}

DWORD C_CSGO::m_ccsplayerrenderablevftable() {
	if (!p_ccsplayerrenderablevftable)
		p_ccsplayerrenderablevftable = *(DWORD*)(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C 24 0C")) + 0x4E);

	return p_ccsplayerrenderablevftable;
}

IWeaponSystem* C_CSGO::m_weaponsystem() {

	if (!p_weaponsystem)
		p_weaponsystem = *reinterpret_cast<IWeaponSystem**>(util::FindSignature(crypt_str("client.dll"), crypt_str("8B 35 ? ? ? ? FF 10 0F B7 C0")) + 2);

	return p_weaponsystem;
}