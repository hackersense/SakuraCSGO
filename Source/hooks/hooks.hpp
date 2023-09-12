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
#include "core\VmtSwap.h"
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

namespace Hooks
{
	extern bool menu_open;
	extern bool input_shouldListen;
	extern int rage_weapon;
	extern int legit_weapon;

	extern ButtonCode_t* input_receivedKeyval;

	typedef long(__stdcall* ResetFn)(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	typedef long(__stdcall* PresentFn)(IDirect3DDevice9*, RECT*, RECT*, HWND, RGNDATA*);

	extern VmtSwap player_hook;
	extern VmtSwap inputinternal_hook;
	extern VmtSwap renderview_hook;
	extern ClientHook* modelrender_hook;
	extern VmtSwap steam_hook;
	//extern ClientHook* directx_hook;
	extern PresentFn oPresent;
	extern ResetFn oReset;
	extern MinHook getforeignfallbackfontname_hook;
	extern MinHook processinterpolatedlist_hook;
	extern MinHook doextrabonesprocessing_hook;
	extern MinHook updateclientsideanimation_hook;
	extern MinHook calcviewmodelbob_hook;
	extern MinHook standardblendingrules_hook;
	extern MinHook setupbones_hook;
	extern MinHook setupvelocity_hook;
	extern MinHook setupmovement_hook;
	extern MinHook setupaliveloop_hook;
	extern MinHook eyeangles_hook;
	extern MinHook updatestate_hook;
	extern MinHook resetstate_hook;
	extern MinHook checkforsequencechang_hook;
	extern MinHook buildtransformations_hook;
	extern MinHook particlecollection_hook;
	extern MinHook modifyeyeposition_hook;
	extern MinHook physicssimulate_hook;
	extern MinHook calculateview_hook;
	extern MinHook clmove_hook;
	extern MinHook bspquery_hook;
	extern MinHook client_hook;
	extern MinHook clientmode_hook;
	extern MinHook clientstate_hook;
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

	void __stdcall hooked_fsn(ClientFrameStage_t stage) noexcept;
	bool __stdcall hooked_createmove(float inputSampleTime, int sequenceNumber, bool active, CUserCmd* cmd, bool& sendPacket) noexcept;
	void __stdcall CHLCreateMove(int sequenceNumber, float inputSampleTime, bool active, bool& sendPacket) noexcept;
	bool __stdcall hooked_drawfog() noexcept;
	void __stdcall hooked_overrideview(CViewSetup* setup) noexcept;
	bool __stdcall hooked_isconnected() noexcept;
	float __stdcall hooked_getscreenaspectratio(int width, int height) noexcept;
	bool __fastcall hooked_ishltv() noexcept;
	bool __stdcall hooked_isplayingdemo() noexcept;
	void __stdcall hooked_emitsound(SoundParams_t params) noexcept;
	void __stdcall hooked_dme(IMatRenderContext* ctx, const DrawModelState_t& state, const ModelRenderInfo_t& info, matrix3x4_t* bone_to_world) noexcept;
	void  __stdcall hooked_postscreeneffects(void* param) noexcept;
	void __fastcall hooked_setkeycodestate(void* ecx, void* edx, ButtonCode_t code, bool bDown) noexcept;
	void __fastcall hooked_setmousecodestate(void* ecx, void* edx, ButtonCode_t code, MouseCodeState_t state) noexcept;
	void __fastcall hooked_sceneend(void* ecx, void* edx) noexcept;
	void __fastcall hooked_findmdl(void* ecx, void* edx, char* filePath) noexcept;
	void __stdcall hooked_painttraverse(vgui::VPANEL panel, bool force_repaint, bool allow_force) noexcept;
	const char* __fastcall hooked_getforeignfallbackfontname(void* ecx, void* edx) noexcept;
	void __fastcall setupVelocityHook(void* ecx, void* edx) noexcept;
	void __fastcall setupMovementHook(void* ecx, void* edx) noexcept;
	void __fastcall setupAliveloopHook(void* ecx, void* edx) noexcept;
	bool __fastcall hooked_setupbones(void* ecx, void* edx, matrix3x4_t* bone_world_out, int max_bones, int bone_mask, float current_time) noexcept;
	void __fastcall hooked_doextrabonesprocessing(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, const matrix3x4_t& matrix, uint8_t* bone_list, void* context) noexcept;
	void __fastcall hooked_standardblendingrules(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, float curtime, int boneMask) noexcept;
	void __fastcall hooked_updateclientsideanimation(void* ecx, void* edx) noexcept;
	void __fastcall hooked_physicssimulate(void* ecx, void* edx) noexcept;
	void __fastcall hooked_modifyeyeposition(void* ecx, void* edx, unsigned int* position) noexcept;
	void __fastcall hooked_calcviewmodelbob(void* ecx, void* edx, float eyeOrigin) noexcept;
	bool __fastcall hooked_tracefilterforheadcollision(void* ecx, void* edx, player_t* player, unsigned int traceParams) noexcept;
	bool __fastcall hooked_shouldskipanimframe(void* ecx, void* edx) noexcept;
	int __fastcall processinterpolatedlist() noexcept;
	void __fastcall hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing) noexcept;
	void __fastcall hooked_packetend(void* ecx, void* edx) noexcept;
	void __fastcall hooked_packetprocess(void* ecx, void* edx, void* packet, bool header) noexcept;
	int __fastcall hooked_senddatagram(INetChannel* network, void* edx, void* datagram) noexcept;
	void __stdcall hooked_setdrawcolor(int r, int g, int b, int a) noexcept;
	void __stdcall hooked_lockcursor() noexcept;
	bool __fastcall hooked_svcheatsgetbool(void* ecx, void* edx) noexcept;
	int __stdcall hooked_listleavesinbox(const Vector& min, const Vector& max, unsigned short* list, int listMax) noexcept;
	void __fastcall hooked_runcommand(void* ecx, void* edx, player_t* player, CUserCmd* m_pcmd, IMoveHelper* move_helper) noexcept;
	bool __stdcall hooked_inprediction() noexcept;
	void __fastcall hooked_gamemovement(void* ecx, void* edx, player_t* ent, CMoveData* move_data) noexcept;
	void __stdcall hooked_onjump(float stamina) noexcept;
	bool __fastcall hooked_writeusercmddeltatobuffer(void* ecx, void* edx, int slot, bufferWrite* buf, int from, int to, bool is_new_command) noexcept;
	void __fastcall hooked_clip_ray_collideable(void* ecx, void* edx, const Ray_t& ray, uint32_t fMask, ICollideable* pCollide, CGameTrace* pTrace) noexcept;
	void __fastcall hooked_trace_ray(void* ecx, void* edx, const Ray_t& ray, unsigned int fMask, ITraceFilter* pTraceFilter, CGameTrace* pTrace) noexcept;
	int __stdcall hooked_getunverifiedfilehashes(void* ecx, void* edx, int maxFiles) noexcept;
	int __fastcall hooked_canloadthirdpartyfiles(void* ecx, void* edx) noexcept;

	LRESULT __stdcall Hooked_WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam);
	HRESULT __stdcall hooked_reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters);
	HRESULT __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region);
	void GUI_Init(IDirect3DDevice9* pDevice);
}
