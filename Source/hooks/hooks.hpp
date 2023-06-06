#pragma once

#include "..\ImGui\imgui.h"
#include "..\ImGui\imgui_internal.h"
#include "..\ImGui\imgui_impl_dx9.h"
#include "..\ImGui\imgui_impl_win32.h"

#include "..\includes.hpp"
#include "..\sdk\interfaces\IBaseClientDll.hpp"
#include "..\sdk\interfaces\IClientMode.hpp"
#include "..\sdk\misc\CUserCmd.hpp"
#include "..\sdk\misc\ParticleCollection.h"
#include "..\sdk\interfaces\IInputSystem.hpp"
#include "..\sdk\interfaces\IMDLCache.hpp"
#include "..\sdk\interfaces\IGameEventManager.hpp"
#include "core\MinHook.h"
#include "core\ClientHook.h"
#include "..\utils\util.hpp"

class C_HookedEvents : public IGameEventListener
{
public:
	void FireGameEvent(IGameEvent* event);
	void Initialize();
	void Uninstall();
	int GetEventDebugID();
	void AddListener(const char* name);
};

namespace WindowInfo
{
	extern HMODULE moduleHandle;
	extern HWND window;
	extern HANDLE handle;
	extern WNDPROC originalWndProc;
}

class c_baseplayeranimationstate;

namespace Hooks
{
	extern bool menu_open;
	extern bool input_shouldListen;
	extern int rage_weapon;
	extern int legit_weapon;

	extern ButtonCode_t* input_receivedKeyval;

	typedef long(__stdcall* ResetFn)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	typedef long(__stdcall* PresentFn)(IDirect3DDevice9*, RECT*, RECT*, HWND, RGNDATA*);

	extern ClientHook* player_hook;
	extern ClientHook* clientstate_hook;
	extern ClientHook* clientmode_hook;
	extern ClientHook* inputinternal_hook;
	extern ClientHook* renderview_hook;
	extern ClientHook* materialsys_hook;
	extern ClientHook* modelrender_hook;
	extern ClientHook* bspquery_hook;
	extern ClientHook* steam_hook;
	extern ClientHook* directx_hook;
	extern PresentFn oPresent;
	extern ResetFn oReset;
	extern MinHook client_hook;
	extern MinHook prediction_hook;
	extern MinHook gamemovement_hook;
	extern MinHook modelcache_hook;
	extern MinHook trace_hook;
	extern MinHook engine_hook;
	extern MinHook panel_hook;
	extern MinHook engine_sound_hook;
	extern MinHook surface_hook;
	extern MinHook filesystem_hook;
	extern MinHook svcheats_hook;
	extern MinHook keyvaluessystem_hook;
	extern MinHook returnaddressclient_hook;
	extern MinHook returnaddressengine_hook;
	extern MinHook returnaddressstudiorender_hook;
	extern MinHook returnaddressmaterialsystem_hook;
	extern MinHook senddatagram_hook;
	extern MinHook performscreenoverlay_hook;
	extern MinHook isdepthoffieldenabled_hook;
	extern MinHook tracefilterforheadcollision_hook;

	extern C_HookedEvents hooked_events;

	using GetForeignFallbackFontNameFn = const char* (__thiscall*)(void*);
	using SetupBonesFn = bool(__thiscall*)(void*, matrix3x4_t*, int, int, float);
	using DoExtraBonesProcessingFn = void(__thiscall*)(player_t*, c_studio_hdr*, Vector*, Quaternion*, const matrix3x4_t&, uint8_t*, void*);
	using StandardBlendingRulesFn = void(__thiscall*)(player_t*, c_studio_hdr*, Vector*, Quaternion*, float, int);
	using UpdateClientSideAnimationFn = void(__fastcall*)(player_t*);
	using PhysicsSimulateFn = void(__fastcall*)(player_t*);
	using ModifyEyePositionFn = void(__thiscall*)(c_baseplayeranimationstate*, Vector&);
	using CalcViewmodelBobFn = void(__thiscall*)(player_t*, Vector&);
	using CheckForSequenceChangeFn = void(__thiscall*)(void*, void*, int, bool, bool);
	using ParticleCollectionFn = void(__thiscall*)(ParticleCollection* thisPointer);
	using ProcessInterpolatedListFn = int(*)(void);
	using CLMoveFn = void(__cdecl*)(float, bool);

	extern DWORD original_getforeignfallbackfontname;
	extern DWORD original_setupbones;
	extern DWORD original_doextrabonesprocessing;
	extern DWORD original_standardblendingrules;
	extern DWORD original_physicssimulate;
	extern DWORD original_modifyeyeposition;
	extern DWORD original_updateclientsideanimation;
	extern DWORD original_calcviewmodelbob;
	extern DWORD original_processinterpolatedlist;
	extern DWORD original_checkforsequencechange;
	extern DWORD original_particlecollection;
	extern DWORD original_clmove;

	void __stdcall hooked_fsn(ClientFrameStage_t stage);
	bool __stdcall hooked_createmove(float inputSampleTime, int sequenceNumber, bool active, CUserCmd* cmd, bool& sendPacket);
	void __stdcall CHLCreateMove(int sequenceNumber, float inputSampleTime, bool active, bool& sendPacket);
	bool __fastcall hooked_drawfog(void* ecx, void* edx);
	void __stdcall hooked_overrideview(CViewSetup* setup);
	bool __stdcall hooked_isconnected();
	float __stdcall hooked_getscreenaspectratio(int width, int height);
	bool __fastcall hooked_ishltv();
	bool __stdcall hooked_isplayingdemo();
	void __fastcall hooked_emitsound(SoundParams_t params);
	void __stdcall hooked_dme(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world);
	void  __fastcall hooked_postscreeneffects(void* thisptr, void* edx, CViewSetup* setup);
	void __fastcall hooked_setkeycodestate(void* thisptr, void* edx, ButtonCode_t code, bool bDown);
	void __fastcall hooked_setmousecodestate(void* thisptr, void* edx, ButtonCode_t code, MouseCodeState_t state);
	void __fastcall hooked_sceneend(void* ecx, void* edx);
	void __fastcall hooked_findmdl(void* ecx, void* edx, char* filePath);
	void __stdcall hooked_painttraverse(vgui::VPANEL panel, bool force_repaint, bool allow_force);
	const char* __fastcall hooked_getforeignfallbackfontname(void* ecx, uint32_t i);
	_declspec(noinline)const char* getforeignfallbackfontname_detour(void* ecx, uint32_t i);
	bool __fastcall hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time);
	void __fastcall hooked_doextrabonesprocessing(player_t* player, void* edx, c_studio_hdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context);
	void __fastcall hooked_standardblendingrules(player_t* player, int i, c_studio_hdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask);
	void __fastcall hooked_updateclientsideanimation(player_t* player, uint32_t i);
	void __fastcall hooked_physicssimulate(player_t* player, void* edx);
	void __fastcall hooked_modifyeyeposition(c_baseplayeranimationstate* state, void* edx, Vector& position);
	void __fastcall hooked_calcviewmodelbob(player_t* player, void* edx, Vector& position);
	bool __fastcall hooked_tracefilterforheadcollision(void* ecx, void* edx, player_t* player, unsigned int traceParams);
	bool __fastcall hooked_shouldskipanimframe(void* ecx, void* edx);
	int __fastcall processinterpolatedlist();
	IMaterial* __fastcall hooked_getmaterial(void* ecx, void* edx, const char* material_name, const char* texture_group_name, bool complain, const char* complain_prefix);
	void __fastcall hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing);
	void __fastcall hooked_packetend(void* ecx, void* edx);
	void __fastcall hooked_packetprocess(void* ecx, void* edx, void* packet, bool header);
	int __fastcall hooked_senddatagram(INetChannel* network, void* edx, void* datagram);
	void __stdcall hooked_setdrawcolor(int r, int g, int b, int a);
	void __stdcall hooked_lockcursor();
	bool __fastcall hooked_svcheatsgetbool(void* ecx, void* edx);
	int __fastcall hooked_listleavesinbox(void* ecx, void* edx, Vector& mins, Vector& maxs, unsigned short* list, int list_max);
	void __fastcall hooked_runcommand(void* ecx, void* edx, player_t* player, CUserCmd* m_pcmd, IMoveHelper* move_helper);
	bool __stdcall hooked_inprediction();
	void __fastcall hooked_gamemovement(void* ecx, void* edx, player_t* ent, CMoveData* move_data);
	bool __fastcall hooked_writeusercmddeltatobuffer(void* ecx, void* edx, int slot, bufferWrite* buf, int from, int to, bool is_new_command);
	void __fastcall hooked_clip_ray_collideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace);
	void __fastcall hooked_trace_ray(void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace);
	int __stdcall hooked_getunverifiedfilehashes(void* ecx, void* edx, int maxFiles);
	int __fastcall hooked_canloadthirdpartyfiles(void* ecx, void* edx);

	LRESULT __stdcall Hooked_WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
	HRESULT __stdcall hooked_reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);
	void GUI_Init(IDirect3DDevice9* pDevice);

	typedef void(__thiscall* SetKeyCodeState_t) (void*, ButtonCode_t, bool);
	extern SetKeyCodeState_t o_SetKeyCodeState;

	typedef void(__thiscall* SetMouseCodeState_t) (void*, ButtonCode_t, MouseCodeState_t);
	extern SetMouseCodeState_t o_SetMouseCodeState;
}
