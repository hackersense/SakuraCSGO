#include "hooks.hpp"

#include <d3d9.h>

#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "freetype.lib")

#include "..\features\misc\misc.h"
#include "..\features\visuals\radar.h"

#include "..\Bytesa.h"
#include "..\features\imgs\weapon_font.h"
#include "..\features\imgs\logo_font.h"
#include "..\features\imgs\skeet_tab.h"
#include "..\features\imgs\main_font.h"
#include "..\features\imgs\timer_font.h"
#include "..\features\menu.h"

static auto d3d_init = false;

namespace WindowInfo
{
	HMODULE moduleHandle;
	HWND window;
	HANDLE handle;
	WNDPROC originalWndProc;
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

namespace Hooks
{
	int rage_weapon = 0;
	int legit_weapon = 0;
	bool menu_open = false;

	LRESULT __stdcall Hooked_WndProc(HWND window, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		static auto is_down = false;
		static auto is_clicked = false;

		if (GetAsyncKeyState(configs.menu.key))
		{
			is_clicked = false;
			is_down = true;
		}
		else if (!GetAsyncKeyState(configs.menu.key) && is_down)
		{
			is_clicked = true;
			is_down = false;
		}
		else
		{
			is_clicked = false;
			is_down = false;
		}

		if (is_clicked && c_menu::get().load_success)
		{
			menu_open = !menu_open;

			if (menu_open && cheat.available() && cheat.globals.current_weapon != -1)
			{
				if (configs.ragebot.enabled)
					Hooks::rage_weapon = cheat.globals.current_weapon;
				else if (configs.legitbot.enabled)
					Hooks::legit_weapon = cheat.globals.current_weapon;
			}
		}

		if (menu_open)
			ImGui_ImplWin32_WndProcHandler(window, msg, wParam, lParam);

		m_inputsystem()->EnableInput(!(menu_open && ((wParam != 'W' && wParam != 'A' && wParam != 'S' && wParam != 'D' && wParam != VK_SHIFT && wParam != VK_CONTROL && wParam != VK_SPACE) || ImGui::GetIO().WantTextInput)) && c_menu::get().load_success);

		return CallWindowProc(WindowInfo::originalWndProc, window, msg, wParam, lParam);
	}

	void GUI_Init(IDirect3DDevice9* pDevice)
	{
		ImGui::CreateContext();

		ImGui_ImplWin32_Init(WindowInfo::window);
		ImGui_ImplDX9_Init(pDevice);

		auto& io = ImGui::GetIO();
		auto& style = ImGui::GetStyle();

		style.WindowMinSize = ImVec2(10, 10);

		ImFontConfig m_config;
		m_config.OversampleH = m_config.OversampleV = 3;
		m_config.PixelSnapH = false;

		static const ImWchar ranges[] =
		{
			0x0020, 0x00FF,
			0x0400, 0x044F,
			0
		};

		// Menu fonts
		c_menu::get().futura_small = io.Fonts->AddFontFromMemoryTTF(futuramediumc, sizeof(futuramediumc), 15.f, &m_config, ranges);
		c_menu::get().futura = io.Fonts->AddFontFromMemoryTTF(futuramediumc, sizeof(futuramediumc), 17.f, &m_config, ranges);
		c_menu::get().futura_large = io.Fonts->AddFontFromMemoryTTF(futuramediumc, sizeof(futuramediumc), 25.f, &m_config, ranges);
		c_menu::get().gotham = io.Fonts->AddFontFromMemoryTTF(GothamPro, sizeof(GothamPro), 13.f, &m_config, ranges);
		c_menu::get().timersz = io.Fonts->AddFontFromMemoryTTF(timers, sizeof(timers), 20.f, &m_config, ranges);
		c_menu::get().weapon_icon2 = io.Fonts->AddFontFromMemoryTTF(weapon_font2, sizeof(weapon_font2), 20, &m_config, io.Fonts->GetGlyphRangesDefault());

		// Icon fonts
		c_menu::get().ico_menu = io.Fonts->AddFontFromMemoryTTF(icomenu, sizeof(icomenu), 20.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		c_menu::get().ico_bottom = io.Fonts->AddFontFromMemoryTTF(iconbot, sizeof(iconbot), 20.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		c_menu::get().tabs = io.Fonts->AddFontFromMemoryTTF(evolve, sizeof(evolve), 32.0f, nullptr, ranges);

		skeet_menu.weapon_font = io.Fonts->AddFontFromMemoryTTF(weapon_font, sizeof(weapon_font), 17.f, &m_config, ranges);
		skeet_menu.tabs = io.Fonts->AddFontFromMemoryTTF(skeet, sizeof(skeet), 50.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.f1 = io.Fonts->AddFontFromMemoryTTF(main_font, sizeof(main_font), 15.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.f2 = io.Fonts->AddFontFromMemoryTTF(main_font, sizeof(main_font), 11.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.f3 = io.Fonts->AddFontFromMemoryTTF(main_font, sizeof(main_font), 8.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.title = io.Fonts->AddFontFromMemoryTTF(futuramediumc, sizeof(futuramediumc), 50.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.text = io.Fonts->AddFontFromMemoryTTF(futuramediumc, sizeof(futuramediumc), 25.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.massiveFont = io.Fonts->AddFontFromMemoryTTF(verdanaBoldBytes, sizeof(verdanaBoldBytes), 34.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());
		skeet_menu.icons = io.Fonts->AddFontFromMemoryTTF(weapon_font, sizeof(weapon_font), 15.f, &m_config, io.Fonts->GetGlyphRangesCyrillic());

		neverlosemenu.g_pd3dDevice = pDevice;

		neverlosemenu.tolstiy = io.Fonts->AddFontFromFileTTF("C:/Windows/fonts/ariblk.ttf", 40.0f);
		neverlosemenu.ne_takoi_tolstiy = io.Fonts->AddFontFromFileTTF("C:/Windows/fonts/arialbd.ttf", 13.0f);
		neverlosemenu.ne_takoi_tolstiy2 = io.Fonts->AddFontFromFileTTF("C:/Windows/fonts/arialbd.ttf", 15.0f);

		unicodeFont.init(io);

		ImGui_ImplDX9_CreateDeviceObjects();

		d3d_init = true;
	}

	HRESULT __stdcall hooked_present(IDirect3DDevice9* device, RECT* src_rect, RECT* dest_rect, HWND dest_wnd_override, RGNDATA* dirty_region)
	{
		//static auto oPresent = directx_hook->getOriginal<PresentFn>(17);

		if (!d3d_init)
			GUI_Init(device);

		IDirect3DVertexDeclaration9* vertex_dec;
		device->GetVertexDeclaration(&vertex_dec);

		IDirect3DVertexShader9* vertex_shader;
		device->GetVertexShader(&vertex_shader);

		c_menu::get().device = device;
		PostProcessing::setDevice(device);
		ImGui_ImplDX9_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();

		//PostProcessing::newFrame();

		if (const auto& displaySize = ImGui::GetIO().DisplaySize; displaySize.x > 0.0f && displaySize.y > 0.0f) {
			imRender->BeginScene();

			if (cheat.globals.loaded_script)
				for (auto& current : c_lua::get().hooks.getHooks(crypt_str("on_draw")))
					current.func();

			imRender->EndScene();

			//c_menu::get().draw(Hooks::menu_open);
			// if (Hooks::menu_open) neverlosemenu.render();
			skeet_menu.set_menu_opened(Hooks::menu_open);
			skeet_menu.draw();

			if (cheat.globals.should_update_radar)
				Radar::get().on_map_load(m_engine()->GetLevelNameShort(), device);
			else
				Radar::get().render();
		}

		ImGui::EndFrame();
		ImGui::Render();

		if (device->BeginScene() == D3D_OK)
		{
			ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());
			device->EndScene();
		}

		device->SetVertexShader(vertex_shader);
		device->SetVertexDeclaration(vertex_dec);

		return oPresent(device, src_rect, dest_rect, dest_wnd_override, dirty_region);
	}

	HRESULT __stdcall hooked_reset(IDirect3DDevice9* pDevice, D3DPRESENT_PARAMETERS* pPresentationParameters)
	{
		//static auto oReset = directx_hook->getOriginal<ResetFn>(16);

		if (!d3d_init)
			return oReset(pDevice, pPresentationParameters);

		ImGui_ImplDX9_InvalidateDeviceObjects();

		PostProcessing::onDeviceReset();

		auto hr = oReset(pDevice, pPresentationParameters);

		if (SUCCEEDED(hr))
			ImGui_ImplDX9_CreateDeviceObjects();

		return hr;
	}

	// Memory Hook
	MinHook modifyeyeposition_hook;
	MinHook getforeignfallbackfontname_hook;
	MinHook shouldskipanimationframe_hook;
	MinHook setupbones_hook;
	MinHook standardblendingrules_hook;
	MinHook processinterpolatedlist_hook;
	MinHook physicssimulate_hook;
	MinHook updateclientsideanimation_hook;
	MinHook doextrabonesprocessing_hook;
	MinHook calcviewmodelbob_hook;
	MinHook calculateview_hook;
	MinHook checkforsequencechang_hook;
	MinHook clmove_hook;
	MinHook particlecollection_hook;
	MinHook senddatagram_hook;
	MinHook performscreenoverlay_hook;
	MinHook isdepthoffieldenabled_hook;
	MinHook tracefilterforheadcollision_hook;
	MinHook buildtransformations_hook;
	MinHook eyeangles_hook;
	MinHook setupvelocity_hook;
	MinHook setupmovement_hook;
	MinHook setupaliveloop_hook;
	MinHook updatestate_hook;
	MinHook resetstate_hook;

	// Return Address
	MinHook returnaddressclient_hook;
	MinHook returnaddressengine_hook;
	MinHook returnaddressstudiorender_hook;
	MinHook returnaddressmaterialsystem_hook;

	ClientHook* modelrender_hook;
	VmtSwap steam_hook;
	MinHook renderview_hook;
	MinHook bspquery_hook;
	MinHook client_hook;
	MinHook clientmode_hook;
	MinHook clientstate_hook;
	MinHook prediction_hook;
	MinHook gamemovement_hook;
	MinHook modelcache_hook;
	MinHook trace_hook;
	MinHook engine_hook;
	MinHook panel_hook;
	MinHook engine_sound_hook;
	MinHook surface_hook;
	MinHook filesystem_hook;
	MinHook svcheats_hook;
	MinHook keyvaluessystem_hook;

	PresentFn oPresent;
	ResetFn oReset;

	C_HookedEvents hooked_events;
}
