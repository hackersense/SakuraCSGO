#include <ShlObj_core.h>
#include <unordered_map>
#include "menu.h"
#include "loli.h"
#include "../constchars.h"
#include "../features/misc/logs.h"
#include "../features/movement/walkbot.h"
#include "../ImGui/imgui_freetype.h"
#include "../utils/FileUtils.h"
#include "../profileChanger/Protobuffs.h"
#include "../steam_sdk/isteamfriends.h"
#include "../steam_sdk/isteamuser.h"
#include "../steam_sdk/isteamutils.h"

#include <WinInet.h>
#include <urlmon.h>
#pragma comment(lib, "urlmon.lib")

std::vector<std::string> files;
std::vector<std::string> scripts;
std::string editing_script;
IDirect3DTexture9* all_skins[36];
IDirect3DTexture9* loli_image = nullptr;
static float buttonPositionX = 226;
auto selected_script = 0;
auto loaded_editing_script = false;

LPDIRECT3DTEXTURE9 c_menu::steam_image()
{
	LPDIRECT3DTEXTURE9 asdgsdgadsg;

	int iImage = SteamFriends()->GetLargeFriendAvatar(SteamUser()->GetSteamID());
	if (iImage == -1)
		return nullptr;
	uint32 uAvatarWidth, uAvatarHeight;
	if (!SteamUtils()->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return nullptr;
	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];
	if (!SteamUtils()->GetImageRGBA(iImage, pAvatarRGBA, uImageSizeInBytes))
	{
		delete[] pAvatarRGBA;
		return nullptr;
	}
	auto res = c_menu::get().device->CreateTexture(uAvatarWidth,
		uAvatarHeight,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&asdgsdgadsg,
		nullptr);
	std::vector<uint8_t> texData;
	texData.resize(uAvatarWidth * uAvatarHeight * 4u);
	CopyConvert(pAvatarRGBA,
		texData.data(),
		uAvatarWidth * uAvatarHeight * 4u);
	D3DLOCKED_RECT rect;
	res = asdgsdgadsg->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
	auto src = texData.data();
	auto dst = reinterpret_cast<uint8_t*>(rect.pBits);
	for (auto y = 0u; y < uAvatarHeight; ++y)
	{
		std::copy(src, src + (uAvatarWidth * 4), dst);

		src += uAvatarWidth * 4;
		dst += rect.Pitch;
	}
	res = asdgsdgadsg->UnlockRect(0);
	delete[] pAvatarRGBA;
	return asdgsdgadsg;
}

LPDIRECT3DTEXTURE9 c_menu::steam_image_friends(CSteamID SteamId)
{
	LPDIRECT3DTEXTURE9 asdgsdgadsg;

	int iImage = SteamFriends()->GetLargeFriendAvatar(SteamId);
	if (iImage == -1)
		return nullptr;
	uint32 uAvatarWidth, uAvatarHeight;
	if (!SteamUtils()->GetImageSize(iImage, &uAvatarWidth, &uAvatarHeight))
		return nullptr;
	const int uImageSizeInBytes = uAvatarWidth * uAvatarHeight * 4;
	uint8* pAvatarRGBA = new uint8[uImageSizeInBytes];
	if (!SteamUtils()->GetImageRGBA(iImage, pAvatarRGBA, uImageSizeInBytes))
	{
		delete[] pAvatarRGBA;
		return nullptr;
	}
	auto res = c_menu::get().device->CreateTexture(uAvatarWidth,
		uAvatarHeight,
		1,
		D3DUSAGE_DYNAMIC,
		D3DFMT_A8R8G8B8,
		D3DPOOL_DEFAULT,
		&asdgsdgadsg,
		nullptr);
	std::vector<uint8_t> texData;
	texData.resize(uAvatarWidth * uAvatarHeight * 4u);
	CopyConvert(pAvatarRGBA,
		texData.data(),
		uAvatarWidth * uAvatarHeight * 4u);
	D3DLOCKED_RECT rect;
	res = asdgsdgadsg->LockRect(0, &rect, nullptr, D3DLOCK_DISCARD);
	auto src = texData.data();
	auto dst = reinterpret_cast<uint8_t*>(rect.pBits);
	for (auto y = 0u; y < uAvatarHeight; ++y)
	{
		std::copy(src, src + (uAvatarWidth * 4), dst);

		src += uAvatarWidth * 4;
		dst += rect.Pitch;
	}
	res = asdgsdgadsg->UnlockRect(0);
	delete[] pAvatarRGBA;
	return asdgsdgadsg;
}

std::string get_weapon(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			switch (custom_index)
			{
			case 0: return crypt_str("weapon_knife");
			case 1: return crypt_str("weapon_bayonet");
			case 2: return crypt_str("weapon_knife_css");
			case 3: return crypt_str("weapon_knife_flip");
			case 4: return crypt_str("weapon_knife_gut");
			case 5: return crypt_str("weapon_knife_karambit");
			case 6: return crypt_str("weapon_knife_m9_bayonet");
			case 7: return crypt_str("weapon_knife_tactical");
			case 8: return crypt_str("weapon_knife_falchion");
			case 9: return crypt_str("weapon_knife_survival_bowie");
			case 10: return crypt_str("weapon_knife_butterfly");
			case 11: return crypt_str("weapon_knife_push");
			case 12: return crypt_str("weapon_knife_cord");
			case 13: return crypt_str("weapon_knife_canis");
			case 14: return crypt_str("weapon_knife_ursus");
			case 15: return crypt_str("weapon_knife_gypsy_jackknife");
			case 16: return crypt_str("weapon_knife_outdoor");
			case 17: return crypt_str("weapon_knife_stiletto");
			case 18: return crypt_str("weapon_knife_widowmaker");
			case 19: return crypt_str("weapon_knife_skeleton");
			}
		}
		else
		{
			switch (custom_index)
			{
			case 0: return crypt_str("ct_gloves");
			case 1: return crypt_str("studded_bloodhound_gloves");
			case 2: return crypt_str("t_gloves");
			case 3: return crypt_str("ct_gloves");
			case 4: return crypt_str("sporty_gloves");
			case 5: return crypt_str("slick_gloves");
			case 6: return crypt_str("leather_handwraps");
			case 7: return crypt_str("motorcycle_gloves");
			case 8: return crypt_str("specialist_gloves");
			case 9: return crypt_str("studded_hydra_gloves");
			case 10: return crypt_str("studded_brokenfang_gloves");
			}
		}
	}
	else
	{
		switch (id)
		{
		case 0: return crypt_str("knife");
		case 1: return crypt_str("gloves");
		case 2: return crypt_str("weapon_ak47");
		case 3: return crypt_str("weapon_aug");
		case 4: return crypt_str("weapon_awp");
		case 5: return crypt_str("weapon_cz75a");
		case 6: return crypt_str("weapon_deagle");
		case 7: return crypt_str("weapon_elite");
		case 8: return crypt_str("weapon_famas");
		case 9: return crypt_str("weapon_fiveseven");
		case 10: return crypt_str("weapon_g3sg1");
		case 11: return crypt_str("weapon_galilar");
		case 12: return crypt_str("weapon_glock");
		case 13: return crypt_str("weapon_m249");
		case 14: return crypt_str("weapon_m4a1_silencer");
		case 15: return crypt_str("weapon_m4a1");
		case 16: return crypt_str("weapon_mac10");
		case 17: return crypt_str("weapon_mag7");
		case 18: return crypt_str("weapon_mp5sd");
		case 19: return crypt_str("weapon_mp7");
		case 20: return crypt_str("weapon_mp9");
		case 21: return crypt_str("weapon_negev");
		case 22: return crypt_str("weapon_nova");
		case 23: return crypt_str("weapon_hkp2000");
		case 24: return crypt_str("weapon_p250");
		case 25: return crypt_str("weapon_p90");
		case 26: return crypt_str("weapon_bizon");
		case 27: return crypt_str("weapon_revolver");
		case 28: return crypt_str("weapon_sawedoff");
		case 29: return crypt_str("weapon_scar20");
		case 30: return crypt_str("weapon_ssg08");
		case 31: return crypt_str("weapon_sg556");
		case 32: return crypt_str("weapon_tec9");
		case 33: return crypt_str("weapon_ump45");
		case 34: return crypt_str("weapon_usp_silencer");
		case 35: return crypt_str("weapon_xm1014");
		default: return crypt_str("unknown");
		}
	}
}

std::string get_wepname(int id, int custom_index = -1, bool knife = true)
{
	if (custom_index > -1)
	{
		if (knife)
		{
			if (custom_index >= 0 || custom_index < 20)
				return crypt_str("Knife");
		}
		else
		{
			if (custom_index >= 0 || custom_index < 10)
				return crypt_str("Glove");
		}
	}
	else
	{
		switch (id)
		{
		case 0: return crypt_str("Knife");
		case 1: return crypt_str("Glove");
		case 2: return crypt_str("AK47");
		case 3: return crypt_str("AUG");
		case 4: return crypt_str("AWP");
		case 5: return crypt_str("CZ75-Auto");
		case 6: return crypt_str("Deagle");
		case 7: return crypt_str("Elite");
		case 8: return crypt_str("Famas");
		case 9: return crypt_str("Five-Seven");
		case 10: return crypt_str("G3SG1");
		case 11: return crypt_str("Galil-AR");
		case 12: return crypt_str("Glock");
		case 13: return crypt_str("M249");
		case 14: return crypt_str("M4A1-S");
		case 15: return crypt_str("M4A1");
		case 16: return crypt_str("Mac10");
		case 17: return crypt_str("Mag7");
		case 18: return crypt_str("Mp5-SD");
		case 19: return crypt_str("Mp7");
		case 20: return crypt_str("Mp9");
		case 21: return crypt_str("Negev");
		case 22: return crypt_str("Nova");
		case 23: return crypt_str("HPK2000");
		case 24: return crypt_str("P250");
		case 25: return crypt_str("P90");
		case 26: return crypt_str("Bizon");
		case 27: return crypt_str("Revolver");
		case 28: return crypt_str("Sawedoff");
		case 29: return crypt_str("Scar20");
		case 30: return crypt_str("Ssg08");
		case 31: return crypt_str("Sg556");
		case 32: return crypt_str("Tec9");
		case 33: return crypt_str("Ump45");
		case 34: return crypt_str("USP-S");
		case 35: return crypt_str("XM1014");
		default: return crypt_str("Unknown");
		}
	}
}

IDirect3DTexture9* get_skin_preview(const char* weapon_name, const std::string& skin_name, IDirect3DDevice9* device)
{
	IDirect3DTexture9* skin_image = nullptr;
	std::string vpk_path;

	if (strcmp(weapon_name, crypt_str("unknown")) && strcmp(weapon_name, crypt_str("knife")) && strcmp(weapon_name, crypt_str("gloves")))
	{
		if (skin_name.empty() || skin_name == crypt_str("default"))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");
		else if (std::string(weapon_name).find(crypt_str("brokenfang")) != std::string::npos) // strcmp(weapon_name, crypt_str("brokenfang"))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = crypt_str("resource/flash/econ/default_generated/") + std::string(weapon_name) + crypt_str("_") + std::string(skin_name) + crypt_str("_light_large.png");
	}
	else
	{
		if (!strcmp(weapon_name, crypt_str("knife")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_knife.png");
		else if (!strcmp(weapon_name, crypt_str("gloves")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else if (!strcmp(weapon_name, crypt_str("unknown")))
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/weapon_snowball.png");
	}

	const auto handle = m_basefilesystem()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));
	if (handle)
	{
		int file_len = m_basefilesystem()->Size(handle);
		char* image = new char[file_len];

		m_basefilesystem()->Read(image, file_len, handle);
		m_basefilesystem()->Close(handle);

		D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
		delete[] image;
	}

	if (!skin_image)
	{
		std::string vpk_path;

		if (strstr(weapon_name, crypt_str("bloodhound")) != NULL || strstr(weapon_name, crypt_str("hydra")) != NULL)
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/ct_gloves.png");
		else
			vpk_path = crypt_str("resource/flash/econ/weapons/base_weapons/") + std::string(weapon_name) + crypt_str(".png");

		const auto handle = m_basefilesystem()->Open(vpk_path.c_str(), crypt_str("r"), crypt_str("GAME"));

		if (handle)
		{
			int file_len = m_basefilesystem()->Size(handle);
			char* image = new char[file_len];

			m_basefilesystem()->Read(image, file_len, handle);
			m_basefilesystem()->Close(handle);

			D3DXCreateTextureFromFileInMemory(device, image, file_len, &skin_image);
			delete[] image;
		}
	}

	return skin_image;
}

void load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(configs.selected_config), false);
	c_lua::get().unload_all_scripts();

	for (auto& script : configs.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1;

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < configs.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	configs.scripts.scripts.clear();

	cfg_manager->load(cfg_manager->files.at(configs.selected_config), true);
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Loaded ") + files.at(configs.selected_config) + crypt_str(" config"), false);
}

void save_config()
{
	if (cfg_manager->files.empty())
		return;

	configs.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto& script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			configs.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(cfg_manager->files.at(configs.selected_config));
	cfg_manager->config_files();

	eventlogs::get().add(crypt_str("Saved ") + files.at(configs.selected_config) + crypt_str(" config"), false);
}

void remove_config()
{
	if (cfg_manager->files.empty())
		return;

	eventlogs::get().add(crypt_str("Removed ") + files.at(configs.selected_config) + crypt_str(" config"), false);

	cfg_manager->remove(cfg_manager->files.at(configs.selected_config));
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (configs.selected_config >= files.size())
		configs.selected_config = files.size() - 1;
}

void add_config()
{
	auto empty = true;

	for (auto current : configs.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		configs.new_config_name = crypt_str("config");

	eventlogs::get().add(crypt_str("Added ") + configs.new_config_name + crypt_str(" config"), false);

	if (configs.new_config_name.find(crypt_str(".cfg")) == std::string::npos)
		configs.new_config_name += crypt_str(".cfg");

	cfg_manager->save(configs.new_config_name);
	cfg_manager->config_files();

	configs.selected_config = cfg_manager->files.size() - 1;
	files = cfg_manager->files;
}

void draw_multicomboSkeet(std::string name, std::vector<int>& variable, std::vector<const char*> labels)
{
	if (variable.size() < labels.size())
		for (int i = variable.size(); i < labels.size(); i++)
			variable.push_back(false);
	else if (variable.size() > labels.size())
		for (int i = variable.size(); i > labels.size(); i--)
			variable.pop_back();

	static auto howmuchsel = [](std::vector<int> e) -> int {
		int s = 0;
		for (int i = 0; i < e.size(); i++)
			if (e[i])
				s++;

		return s;
	};

	if (ImGui::BeginComboSkeet(name.c_str(), (howmuchsel(variable) > 0 ? std::to_string(howmuchsel(variable)) + " Selected" : "-").c_str(), ImGuiComboFlags_HeightLarge, labels.size())) // draw start
	{
		ImGui::BeginGroup();
		for (auto i = 0; i < variable.size(); i++)
			ImGui::SelectableSkeet(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);

		ImGui::EndGroup();

		ImGui::EndComboSkeet();
	}
}

void menu::initialize(IDirect3DDevice9* pDevice) {
	if (this->m_bInitialized)
		return;

	ImGui::CreateContext();
	auto& io = ImGui::GetIO();
	auto style = &ImGui::GetStyle();

	style->WindowRounding = 0.f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->ScrollbarRounding = 0.f;
	style->ScrollbarSize = 0.f;
	style->WindowPadding = ImVec2(0, 0);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);

	this->m_bIsOpened = true;

	IDirect3DSwapChain9* pChain = nullptr;
	D3DPRESENT_PARAMETERS pp = {};
	D3DDEVICE_CREATION_PARAMETERS param = {};
	pDevice->GetCreationParameters(&param);
	pDevice->GetSwapChain(0, &pChain);

	if (pChain)
		pChain->GetPresentParameters(&pp);

	ImGui_ImplWin32_Init(param.hFocusWindow);
	ImGui_ImplDX9_Init(pDevice);

	this->m_bInitialized = true;
}

// resize current style sizes
void c_menu::dpi_resize(float scale_factor, ImGuiStyle& style)
{
	style.ScaleAllSizes(scale_factor);
}

void menu::set_menu_opened(bool v) {
	this->m_bIsOpened = v;
}

static int player;

float RenderText(const std::string& text, ImVec2 pos, float size, Color color, bool center, bool outline, ImFont* pFont)
{
	ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str());
	if (!pFont->ContainerAtlas) return 0.f;
	ImGui::GetDrawList()->PushTextureID(pFont->ContainerAtlas->TexID);

	if (center)
		pos.x -= textSize.x / 2.0f;

	if (outline) {
		ImGui::GetDrawList()->AddText(pFont, size, ImVec2(pos.x + 1, pos.y + 1), c_menu::get().GetU32(Color(0, 0, 0, color.a())), text.c_str());
		ImGui::GetDrawList()->AddText(pFont, size, ImVec2(pos.x - 1, pos.y - 1), c_menu::get().GetU32(Color(0, 0, 0, color.a())), text.c_str());
		ImGui::GetDrawList()->AddText(pFont, size, ImVec2(pos.x + 1, pos.y - 1), c_menu::get().GetU32(Color(0, 0, 0, color.a())), text.c_str());
		ImGui::GetDrawList()->AddText(pFont, size, ImVec2(pos.x - 1, pos.y + 1), c_menu::get().GetU32(Color(0, 0, 0, color.a())), text.c_str());
	}

	ImGui::GetDrawList()->AddText(pFont, size, pos, c_menu::get().GetU32(color), text.c_str());

	ImGui::GetDrawList()->PopTextureID();

	return pos.y + textSize.y;
}

float load_number = 0;
int text_alpha = 255;
int loading_alpha = 255;
int background_alpha = 165;
std::vector<const char*> loading =
{
	"Loading",
	"Loading.",
	"Loading..",
	"Loading...",
	"Loading..",
	"Loading.",
	"Loading"
};

void menu::draw()
{
	//pool inline
	auto& io = ImGui::GetIO();
	auto style = &ImGui::GetStyle();

	style->WindowRounding = 0.f;
	style->AntiAliasedLines = true;
	style->AntiAliasedFill = true;
	style->ScrollbarRounding = 0.f;
	style->ScrollbarSize = 0.f;
	style->WindowPadding = ImVec2(0, 0);
	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(45 / 255.f, 45 / 255.f, 45 / 255.f, 1.f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(65 / 255.f, 65 / 255.f, 65 / 255.f, 1.f);
	style->Colors[ImGuiCol_Skeet_text] = ImColor(100, 100, 100, 255);
	style->Colors[ImGuiCol_Skeet_shadow] = ImColor(2, 2, 2, 255);
	style->Colors[ImGuiCol_Skeet_hovered] = ImColor(185, 185, 185, 255);

	/*
	if (!this->m_bIsOpened && ImGui::GetStyle().Alpha > 0.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha - fc / 255.f, 0.f, 1.f);
	}

	if (this->m_bIsOpened && ImGui::GetStyle().Alpha < 1.f) {
		float fc = 255.f / 0.2f * ImGui::GetIO().DeltaTime;
		ImGui::GetStyle().Alpha = std::clamp(ImGui::GetStyle().Alpha + fc / 255.f, 0.f, 1.f);
	}
	*/
	//if (!this->m_bIsOpened && ImGui::GetStyle().Alpha == 0.f)
	//	return;

	//if (!c_menu::get().load_success)
	//{
	//	c_menu::get().load_success = true;
	//	Hooks::menu_open = true;
	//	return;
	//}

	if (!c_menu::get().load_success)
	{
		static int width, height;
		m_engine()->GetScreenSize(width, height);
		bool success = (load_number >= 50);
		ImGui::PushFont(skeet_menu.title);
		auto title_size = ImGui::CalcTextSize("Sakura.cc");
		ImGui::PopFont();
		ImGui::PushFont(skeet_menu.text);
		auto text_size = ImGui::CalcTextSize("Flower fall and people die");
		auto text_size2 = ImGui::CalcTextSize(success ? ("Welcome you! " + cheat.username).c_str() : loading.at(static_cast<int>(load_number) % 6));
		//auto free_size = ImGui::CalcTextSize("You are using free edition.");
		ImGui::PopFont();
		ImGui::GetBackgroundDrawList()->AddText(skeet_menu.title, 50, ImVec2((width / 2) - (title_size.x / 2), (height / 2) - title_size.y - 30), ImColor(255, 191, 192, text_alpha), "Sakura.cc");
		ImGui::GetBackgroundDrawList()->AddTextShadow(skeet_menu.text, 25, ImVec2((width / 2) - (text_size.x / 2), (height / 2) - text_size.y), ImColor(255, 191, 192, text_alpha), "Flower fall and people die");
		ImGui::GetBackgroundDrawList()->AddTextShadow(skeet_menu.text, 25, ImVec2((width / 2) - (text_size2.x / 2), (height / 2) - text_size2.y + 30), ImColor(255, 191, 192, loading_alpha), success ? ("Welcome you! " + cheat.username).c_str() : loading.at(static_cast<int>(load_number) % 6));
		ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(width, height), ImColor(255, 192, 203, background_alpha));
		if (success)
		{
			if (loading_alpha > 0)
				loading_alpha -= (loading_alpha < 30) ? 2 : 6;
			else
			{
				background_alpha--;
				text_alpha -= 2;
				if (background_alpha <= 0 && text_alpha <= 0)
				{
					c_menu::get().load_success = true;
					Hooks::menu_open = true;
				}
			}
		}
		else
			load_number += 0.05f;
		return;
	}

	if (!this->m_bIsOpened)
		return;

	static int width, height;

	m_engine()->GetScreenSize(width, height);
	if (loli_image)
		ImGui::GetBackgroundDrawList()->AddImage(loli_image, ImVec2(width - loliwidth, height - loliheight), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), ImColor(255, 255, 255, 255));
	else
		D3DXCreateTextureFromFileInMemory(c_menu::get().device, lolipng, sizeof(lolipng), &loli_image);

	ImGui::SetNextWindowSizeConstraints(ImVec2(760, 660), ImVec2(width, height));

	ImGui::BeginSkeet(cheat.cheatname.c_str(), NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar);

	/*
	static bool a = false;
	if (!a)
	{
		ImGui::SetWindowFontScale(1.5);
		ImGui::SetWindowSize(ImVec2(ImFloor(760 * 1.5), ImFloor(660 * 1.5)));
		style->ScaleAllSizes(1.5f);
		a = true;
	}
	*/

	ImVec2 vecWindowPosition = ImGui::GetWindowPos();

	ImGui::PushFont(tabs);
	ImGui::TabButtonSkeet("C", &this->m_nCurrentTab, 0, 7);
	ImGui::TabButtonSkeet("D", &this->m_nCurrentTab, 1, 7);
	ImGui::TabButtonSkeet("B", &this->m_nCurrentTab, 2, 7);
	ImGui::TabButtonSkeet("E", &this->m_nCurrentTab, 3, 7);
	ImGui::TabButtonSkeet("F", &this->m_nCurrentTab, 4, 7);
	ImGui::TabButtonSkeet("G", &this->m_nCurrentTab, 5, 7);
	ImGui::TabButtonSkeet("H", &this->m_nCurrentTab, 6, 7);
	ImGui::PopFont();

	static auto calculateChildWindowPosition = [](int num) -> ImVec2 {
		return ImVec2(ImGui::GetWindowPos().x + 120 + (ImGui::GetWindowSize().x / 3 - 31) * num + 20 * num, ImGui::GetWindowPos().y + 20);
	};

	static auto calculateChildWindowPositionDouble = [](int num) -> ImVec2 {
		return ImVec2(ImGui::GetWindowPos().x + 120 + (ImGui::GetWindowSize().x / 2 - 70) * num + 20 * num, ImGui::GetWindowPos().y + 20);
	};

	auto child_size = ImVec2(ImGui::GetWindowSize().x / 2 - 90, ImGui::GetWindowSize().y / 2 - 40);
	auto child_size_d = ImVec2(ImGui::GetWindowSize().x / 2 - 90, ImGui::GetWindowSize().y - 40);
	//ImVec2(760, 660)
	auto child_size_x = ImVec2(ImGui::GetWindowSize().x - 140, ImGui::GetWindowSize().y / 13.2 + 25);

	ImGui::PushFont(skeet_menu.f2);
	if (this->m_nCurrentTab == 0)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));
		ImGui::BeginChildSkeet("WEAPONS SECTION RAGE", child_size_x);
		{
			ImGui::PushFont(weapon_font);

			ImGui::Columns(10, nullptr, false);

			if (ImGui::TabButtonSkeetv2("p", &Hooks::rage_weapon, 0, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 0;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("A", &Hooks::rage_weapon, 1, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 1;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("J", &Hooks::rage_weapon, 2, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 2;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("C", &Hooks::rage_weapon, 3, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 3;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("N", &Hooks::rage_weapon, 4, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 4;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("W", &Hooks::rage_weapon, 5, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 5;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("X", &Hooks::rage_weapon, 6, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 6;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("a", &Hooks::rage_weapon, 7, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 7;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("Z", &Hooks::rage_weapon, 8, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 8;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("f", &Hooks::rage_weapon, 9, 10, ImVec2(ImGui::GetWindowSize().x / 10 - 3.5, ImGui::GetWindowSize().y - 25)))
				Hooks::rage_weapon = 9;

			ImGui::PopFont();
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size_x.y + 40));

		ImGui::BeginChildSkeet("RAGEBOT", ImVec2(child_size.x, child_size.y + 200));
		{
			ImGui::CheckboxSkeet(crypt_str("Enable"), &configs.ragebot.enabled);

			if (configs.ragebot.enabled)
				configs.legitbot.enabled = false;

			ImGui::SliderIntSkeet(crypt_str("Maximum FOV"), &configs.ragebot.field_of_view, 1, 360, crypt_str("%d\u00b0"));
			ImGui::CheckboxSkeet(crypt_str("Silent aim"), &configs.ragebot.silent_aim);
			ImGui::SingleSelectSkeet(crypt_str("Automatic wall"), &configs.ragebot.autowall, autowall_type);
			ImGui::CheckboxSkeet(crypt_str("Zeusbot"), &configs.ragebot.zeus_bot);
			if (configs.ragebot.zeus_bot)
				ImGui::SliderIntSkeet(crypt_str("Zeus Hitchance"), &configs.ragebot.zeus_hitchance, 1, 100, crypt_str("%.0f"));
			ImGui::CheckboxSkeet(crypt_str("Knifebot"), &configs.ragebot.knife_bot);
			ImGui::CheckboxSkeet(crypt_str("Automatic fire"), &configs.ragebot.autoshoot);
			ImGui::SingleSelectSkeet(crypt_str("Automatic scope"), &configs.ragebot.autoscope, autoscope_type);
			draw_multicomboSkeet(crypt_str("Anti-aim correction"), configs.ragebot.resolver, resolver_types);
			if (configs.ragebot.resolver[RESOLVER_YAW])
			{
				ImGui::SingleSelectSkeet(crypt_str("Correction mode"), &configs.ragebot.resolver_mode, resolver_mode);
				ImGui::KeybindSkeet("Correction override key", &configs.ragebot.resolver_override.key, &configs.ragebot.resolver_override.mode);
				ImGui::CheckboxSkeet("Adaptive correction", &configs.ragebot.adaptive_resolver);
				if (ImGui::IsItemHovered())
					ImGui::SetTooltip("If the target using jitter, it is not recommended to enable this.");
				if (configs.ragebot.adaptive_resolver)
					ImGui::CheckboxSkeet("Ignore body correction", &configs.ragebot.adaptive_resolver_ignore_body);
			}
			ImGui::CheckboxSkeet(crypt_str("Dormant"), &configs.ragebot.dormant);
			ImGui::CheckboxSkeet(crypt_str("Double tap"), &configs.ragebot.double_tap);
			if (configs.ragebot.double_tap)
			{
				ImGui::CheckboxSkeet(crypt_str("Double tap instant"), &configs.ragebot.double_tap_instant);
				ImGui::KeybindSkeet("Double tap key", &configs.ragebot.double_tap_key.key, &configs.ragebot.double_tap_key.mode);
			}
			ImGui::CheckboxSkeet(crypt_str("Hide shots"), &configs.ragebot.hide_shots);
			if (configs.ragebot.hide_shots)
				ImGui::KeybindSkeet("Hide shots key", &configs.ragebot.hide_shots_key.key, &configs.ragebot.hide_shots_key.mode);
			if (configs.ragebot.hide_shots || configs.ragebot.double_tap)
				ImGui::CheckboxSkeet(crypt_str("Disable teleport"), &configs.ragebot.disable_teleport);
			ImGui::KeybindSkeet("Force safe points", &configs.ragebot.safe_point_key.key, &configs.ragebot.safe_point_key.mode);
			ImGui::KeybindSkeet("Force body aim", &configs.ragebot.body_aim_key.key, &configs.ragebot.body_aim_key.mode);
			ImGui::KeybindSkeet("Damage override", &configs.ragebot.damage_override_key.key, &configs.ragebot.damage_override_key.mode);
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 160.2 + child_size.x, ImGui::GetWindowPos().y + 40.8 + child_size_x.y));

		ImGui::BeginChildSkeet("WEAPON SETTINGS RAGE", ImVec2(child_size.x, child_size.y + 200), 0);
		{
			if (Hooks::rage_weapon != 0)
				ImGui::CheckboxSkeet(crypt_str("Enabled"), &configs.ragebot.weapon[Hooks::rage_weapon].enabled);

			ImGui::SingleSelectSkeet(crypt_str("Target selection"), &configs.ragebot.weapon[Hooks::rage_weapon].selection_type, selection);

			draw_multicomboSkeet(crypt_str("Target hitboxes"), configs.ragebot.weapon[Hooks::rage_weapon].hitboxes, hitboxes);

			ImGui::CheckboxSkeet(crypt_str("Ignore limbs when moving"), &configs.ragebot.weapon[Hooks::rage_weapon].ignore_limbs);

			ImGui::SingleSelectSkeet(crypt_str("Scan point mode"), &configs.ragebot.weapon[Hooks::rage_weapon].scan_point_mode, scan_points);

			if (configs.ragebot.weapon[Hooks::rage_weapon].scan_point_mode == 2)
			{
				draw_multicomboSkeet(crypt_str("Multi-point"), configs.ragebot.weapon[Hooks::rage_weapon].multipoints_hitboxes, hitboxes);
				ImGui::SliderFloatSkeet(crypt_str("Multipoint scale"), &configs.ragebot.weapon[Hooks::rage_weapon].multipoints_scale, 0.0f, 1.0f, configs.ragebot.weapon[Hooks::rage_weapon].multipoints_scale ? crypt_str("%.2f") : crypt_str("None"));
				ImGui::CheckboxSkeet(crypt_str("Adaptive point scale"), &configs.ragebot.weapon[Hooks::rage_weapon].adaptive_point_scale);
			}
			else if (configs.ragebot.weapon[Hooks::rage_weapon].scan_point_mode)
			{
				ImGui::SliderFloatSkeet(crypt_str("Head scale"), &configs.ragebot.weapon[Hooks::rage_weapon].head_scale, 0.0f, 1.0f, configs.ragebot.weapon[Hooks::rage_weapon].head_scale ? crypt_str("%.2f") : crypt_str("None"));
				ImGui::SliderFloatSkeet(crypt_str("Body scale"), &configs.ragebot.weapon[Hooks::rage_weapon].body_scale, 0.0f, 1.0f, configs.ragebot.weapon[Hooks::rage_weapon].body_scale ? crypt_str("%.2f") : crypt_str("None"));
				ImGui::CheckboxSkeet(crypt_str("Adaptive point scale"), &configs.ragebot.weapon[Hooks::rage_weapon].adaptive_point_scale);
			}

			ImGui::CheckboxSkeet(crypt_str("Enable max misses"), &configs.ragebot.weapon[Hooks::rage_weapon].max_misses);

			if (configs.ragebot.weapon[Hooks::rage_weapon].max_misses)
				ImGui::SliderIntSkeet(crypt_str("Max misses"), &configs.ragebot.weapon[Hooks::rage_weapon].max_misses_amount, 0, 6, "%.0f");

			draw_multicomboSkeet(crypt_str("Safe points conditions"), configs.ragebot.weapon[Hooks::rage_weapon].safe_points_conditions, safe_points_conditions);

			draw_multicomboSkeet(crypt_str("Body aim conditions"), configs.ragebot.weapon[Hooks::rage_weapon].body_aim_conditions, body_aim_conditions);

			ImGui::CheckboxSkeet(crypt_str("Dynamic body aim"), &configs.ragebot.weapon[Hooks::rage_weapon].dynamic_body_aim_conditions);

			draw_multicomboSkeet(crypt_str("Body aim hitboxes"), configs.ragebot.weapon[Hooks::rage_weapon].body_aim_hitboxes, hitboxes_baim);

			draw_multicomboSkeet(crypt_str("Prefer head aim"), configs.ragebot.weapon[Hooks::rage_weapon].head_aim_conditions, head_aim_conditions);

			draw_multicomboSkeet(crypt_str("Only head aim"), configs.ragebot.weapon[Hooks::rage_weapon].head_aim_only_conditions, head_aim_only_conditions);

			ImGui::CheckboxSkeet(crypt_str("Accuracy boost"), &configs.ragebot.weapon[Hooks::rage_weapon].autostop);

			if (configs.ragebot.weapon[Hooks::rage_weapon].autostop)
				draw_multicomboSkeet(crypt_str("Modifiers"), configs.ragebot.weapon[Hooks::rage_weapon].autostop_modifiers, autostop_modifiers);

			ImGui::SliderIntSkeet(crypt_str("Hitchance"), &configs.ragebot.weapon[Hooks::rage_weapon].hitchance, 0, 100, "%.0f");

			if (configs.ragebot.double_tap && (Hooks::rage_weapon == 1 || (Hooks::rage_weapon > 2 && Hooks::rage_weapon < 7)))
			{
				ImGui::CheckboxSkeet(crypt_str("Double tap hitchance"), &configs.ragebot.weapon[Hooks::rage_weapon].double_tap_hitchance);

				if (configs.ragebot.weapon[Hooks::rage_weapon].double_tap_hitchance)
					ImGui::SliderIntSkeet(crypt_str("Double tap hitchance amount"), &configs.ragebot.weapon[Hooks::rage_weapon].double_tap_hitchance_amount, 0, 100, "%.0f");
			}

			ImGui::SliderIntSkeet(crypt_str("Minimum visible damage"), &configs.ragebot.weapon[Hooks::rage_weapon].minimum_visible_damage, 0, 150, "%.0f");

			if (configs.ragebot.autowall != 0)
				ImGui::SliderIntSkeet(crypt_str("Minimum wall damage"), &configs.ragebot.weapon[Hooks::rage_weapon].minimum_damage, 0, 150, "%.0f");

			if (configs.ragebot.damage_override_key.key > KEY_NONE && configs.ragebot.damage_override_key.key < KEY_MAX)
				ImGui::SliderIntSkeet(crypt_str("Minimum override damage"), &configs.ragebot.weapon[Hooks::rage_weapon].minimum_override_damage, 0, 150, "%.0f");
		}

		ImGui::EndChildSkeet();
	}

	if (this->m_nCurrentTab == 1)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));
		ImGui::BeginChildSkeet("WEAPONS SECTION LEGIT", child_size_x);
		{
			ImGui::PushFont(weapon_font);

			ImGui::Columns(6, nullptr, false);

			if (ImGui::TabButtonSkeetv2("J", &Hooks::legit_weapon, 0, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 0;
			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("C", &Hooks::legit_weapon, 1, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 1;
			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("W", &Hooks::legit_weapon, 2, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 2;
			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("N", &Hooks::legit_weapon, 3, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 3;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("Z", &Hooks::legit_weapon, 4, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 4;

			ImGui::NextColumn();
			if (ImGui::TabButtonSkeetv2("d", &Hooks::legit_weapon, 5, 6, ImVec2(ImGui::GetWindowSize().x / 10 + 24, ImGui::GetWindowSize().y - 25)))
				Hooks::legit_weapon = 5;

			ImGui::PopFont();
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size_x.y + 60));

		ImGui::BeginChildSkeet("LEGITBOT", ImVec2(child_size.x, child_size.y + 200));
		{
			ImGui::CheckboxSkeet(crypt_str("Enable Legit"), &configs.legitbot.enabled);

			if (configs.legitbot.enabled)
				configs.ragebot.enabled = false;

			ImGui::KeybindSkeet("Key", &configs.legitbot.key.key, &configs.legitbot.key.mode);

			ImGui::CheckboxSkeet(crypt_str("Friendly fire"), &configs.legitbot.friendly_fire);

			ImGui::CheckboxSkeet(crypt_str("Automatic pistol"), &configs.legitbot.autopistol);

			ImGui::CheckboxSkeet(crypt_str("Automatic wall"), &configs.legitbot.autowall);

			ImGui::CheckboxSkeet(crypt_str("Automatic scope"), &configs.legitbot.autoscope);

			ImGui::CheckboxSkeet(crypt_str("Scope only"), &configs.legitbot.sniper_scope_only);

			ImGui::CheckboxSkeet(crypt_str("Jump check"), &configs.legitbot.jump_check);

			ImGui::CheckboxSkeet(crypt_str("Ignore flash"), &configs.legitbot.ignore_flash);

			ImGui::CheckboxSkeet(crypt_str("Ignore smoke"), &configs.legitbot.ignore_smoke);

			ImGui::SingleSelectSkeet(crypt_str("Smooth mode"), &configs.legitbot.smooth_mode, { "None", "Normal", "Humanized" });

			ImGui::CheckboxSkeet(crypt_str("Enable Backtrack"), &configs.backtrack.enabled);

			ImGui::CheckboxSkeet(crypt_str("Backtrack Ignore smoke"), &configs.backtrack.ignoreSmoke);

			ImGui::CheckboxSkeet(crypt_str("Backtrack Ignore Flash"), &configs.backtrack.ignoreFlash);

			//ImGui::CheckboxSkeet(crypt_str("Recoil base fov"), &configs.backtrack.recoilBasedFov);

			//ImGui::SliderIntSkeet(crypt_str("Backtrack fov"), &configs.backtrack.fov, 0, 360);

			ImGui::SliderIntSkeet(crypt_str("Time limit"), &configs.backtrack.timeLimit, 0, 200);

			ImGui::KeybindSkeet(crypt_str("Ping spike"), &configs.backtrack.pingSpike.key, &configs.backtrack.pingSpike.mode);

			ImGui::SliderIntSkeet(crypt_str("Ping spike amount"), &configs.backtrack.pingSpikeAmount, 0, 1000);
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 160.2 + child_size.x, ImGui::GetWindowPos().y + 60.8 + child_size_x.y));

		ImGui::BeginChildSkeet("WEAPON SETTINGS", ImVec2(child_size.x, child_size.y + 200), 0);
		{
			ImGui::SingleSelectSkeet(crypt_str("Bone"), &configs.legitbot.weapon[Hooks::legit_weapon].hitboxes, { "Nearest", "Best damage", "Head", "Neck", "Sternum", "Chest", "Stomach", "Pelvis" });

			ImGui::CheckboxSkeet(crypt_str("Aimlock"), &configs.legitbot.weapon[Hooks::legit_weapon].aimlock);

			ImGui::CheckboxSkeet(crypt_str("Silent"), &configs.legitbot.weapon[Hooks::legit_weapon].silent);

			ImGui::SliderFloatSkeet(crypt_str("Field of view"), &configs.legitbot.weapon[Hooks::legit_weapon].fov, 0.f, 180.0f, crypt_str("%.2f"));

			if (configs.legitbot.smooth_mode)
				ImGui::SliderFloatSkeet(crypt_str("Smooth"), &configs.legitbot.weapon[Hooks::legit_weapon].smooth, 1.f, 100.f, crypt_str("%.2f"));

			ImGui::SliderIntSkeet(crypt_str("Inaccuracy"), &configs.legitbot.weapon[Hooks::legit_weapon].maxAimInaccuracy, 0, 100, crypt_str("%.0f"));

			ImGui::Text(crypt_str("Min damage"));
			ImGui::InputInt(crypt_str("##INPUT__MINDAMAGE"), &configs.legitbot.weapon[Hooks::legit_weapon].min_damage);

			configs.legitbot.weapon[Hooks::legit_weapon].min_damage = std::clamp(configs.legitbot.weapon[Hooks::legit_weapon].min_damage, 0, 150);

			ImGui::CheckboxSkeet("Between shots", &configs.legitbot.weapon[Hooks::legit_weapon].between_shots);

			ImGui::CheckboxSkeet("Recoil control system", &configs.legitbot.weapon[Hooks::legit_weapon].standaloneRCS);
			if (configs.legitbot.weapon[Hooks::legit_weapon].standaloneRCS) {
				ImGui::CheckboxSkeet("Random RCS factor", &configs.legitbot.weapon[Hooks::legit_weapon].randomRCS);
				ImGui::CheckboxSkeet("Silent RCS", &configs.legitbot.weapon[Hooks::legit_weapon].silentRCS);
				ImGui::SliderIntSkeet("RCS Ignore Shots", &configs.legitbot.weapon[Hooks::legit_weapon].shotsFired, 0, 150, "%d");
				if (configs.legitbot.weapon[Hooks::legit_weapon].randomRCS) {
					ImGui::SliderFloatSkeet("RCS Horizontal odds", &configs.legitbot.weapon[Hooks::legit_weapon].recoilControlHorizontal, 0.0f, 1.0f, "%.5f");
					ImGui::SliderFloatSkeet("RCS Vertical odds", &configs.legitbot.weapon[Hooks::legit_weapon].recoilControlVertical, 0.0f, 1.0f, "%.5f");
				}
				else
				{
					ImGui::SliderFloatSkeet("RCS Horizontal", &configs.legitbot.weapon[Hooks::legit_weapon].recoilControlHorizontal, 0.0f, 1.0f, "%.5f");
					ImGui::SliderFloatSkeet("RCS Vertical", &configs.legitbot.weapon[Hooks::legit_weapon].recoilControlVertical, 0.0f, 1.0f, "%.5f");
				}
			}
		}
		ImGui::EndChildSkeet();
	}

	if (this->m_nCurrentTab == 2)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));
		ImGui::BeginChildSkeet("ANTI-AIM", child_size_d);
		{
			static auto type = 0;

			ImGui::CheckboxSkeet(crypt_str("Enable"), &configs.antiaim.enabled);
			if (!configs.antiaim.antiaim_type)
			{
				ImGui::CheckboxSkeet(crypt_str("Jitter move"), &configs.antiaim.jitter_move);
				ImGui::CheckboxSkeet(crypt_str("Auto direction"), &configs.antiaim.auto_direction);
				ImGui::KeybindSkeet("Zero pitch on key", &configs.antiaim.zero_pitch.key, &configs.antiaim.zero_pitch.mode);
				ImGui::KeybindSkeet("180 FakeFlick on key", &configs.antiaim.fake_flick_180.key, &configs.antiaim.fake_flick_180.mode);
			}
			ImGui::SingleSelectSkeet(crypt_str("Anti-aim type"), &configs.antiaim.antiaim_type, antiaim_type);

			if (configs.antiaim.antiaim_type)
			{
				ImGui::SingleSelectSkeet(crypt_str("Desync"), &configs.antiaim.desync, desync);

				if (configs.antiaim.desync)
				{
					ImGui::SingleSelectSkeet(crypt_str("LBY type"), &configs.antiaim.legit_lby_type, lby_type);

					if (configs.antiaim.desync == 1)
						ImGui::KeybindSkeet("Invert desync key", &configs.antiaim.flip_desync.key, &configs.antiaim.flip_desync.mode);
				}
			}
			else
			{
				ImGui::SingleSelectSkeet(crypt_str("Movement type"), &type, movement_type);

				if (!configs.misc.anti_untrusted)
					ImGui::SliderIntSkeet(crypt_str("Roll"), &configs.antiaim.type[type].roll, -50, 50, "%0.f");

				ImGui::CheckboxSkeet(crypt_str("Fake Flick"), &configs.antiaim.type[type].fake_flick);

				ImGui::CheckboxSkeet(crypt_str("Fake Flick Disable With Manual"), &configs.antiaim.type[type].fake_flick_disable_with_manual);

				ImGui::SingleSelectSkeet(crypt_str("Pitch"), &configs.antiaim.type[type].pitch, pitch);

				if (configs.antiaim.type[type].pitch == 9)
					if (configs.misc.anti_untrusted)
						ImGui::SliderIntSkeet(crypt_str("Custom Pitch"), &configs.antiaim.type[type].custom_pitch, -89, 89, "%0.f");
					else
						ImGui::SliderIntSkeet(crypt_str("Custom Pitch"), &configs.antiaim.type[type].custom_pitch, -180, 180, "%0.f");

				ImGui::SingleSelectSkeet(crypt_str("Yaw"), &configs.antiaim.type[type].yaw, yaw);

				ImGui::SingleSelectSkeet(crypt_str("Base angle"), &configs.antiaim.type[type].base_angle, baseangle);

				if (configs.antiaim.type[type].yaw && configs.antiaim.type[type].yaw != 3)
				{
					ImGui::SliderIntSkeet(configs.antiaim.type[type].yaw == 1 ? crypt_str("Jitter range") : crypt_str("Spin range"), &configs.antiaim.type[type].range, 1, 180, "%0.f");

					if (configs.antiaim.type[type].yaw == 1)
					{
						ImGui::SingleSelectSkeet(crypt_str("Yaw jitter type"), &configs.antiaim.type[type].yaw_jitter_type, jitter_type);
						if (configs.antiaim.type[type].yaw_jitter_type == 1)
						{
							ImGui::SliderIntSkeet(crypt_str("Yaw jitter ticks max"), &configs.antiaim.type[type].yaw_jitter_ticks_max, 3, 25);
							ImGui::SliderIntSkeet(crypt_str("Yaw jitter ticks min"), &configs.antiaim.type[type].yaw_jitter_ticks_min, 3, 25);
							configs.antiaim.type[type].yaw_jitter_ticks_min = std::clamp(configs.antiaim.type[type].yaw_jitter_ticks_min, 3, configs.antiaim.type[type].yaw_jitter_ticks_max);
						}
					}

					if (configs.antiaim.type[type].yaw == 2)
						ImGui::SliderIntSkeet(crypt_str("Spin speed"), &configs.antiaim.type[type].speed, 1, 15, "%0.f");
				}

				if (configs.antiaim.type[type].yaw == 3)
					ImGui::SliderIntSkeet(crypt_str("Custom Yaw"), &configs.antiaim.type[type].custom_yaw, -180, 180, "%0.f");

				ImGui::SingleSelectSkeet(crypt_str("Desync"), &configs.antiaim.type[type].desync, desync);

				if (configs.antiaim.type[type].desync == 2)
				{
					ImGui::SingleSelectSkeet(crypt_str("Jitter type"), &configs.antiaim.type[type].jitter_type, jitter_type);
					if (configs.antiaim.type[type].jitter_type == 1)
					{
						ImGui::SliderIntSkeet(crypt_str("Jitter ticks max"), &configs.antiaim.type[type].jitter_ticks_max, 3, 25);
						ImGui::SliderIntSkeet(crypt_str("Jitter ticks min"), &configs.antiaim.type[type].jitter_ticks_min, 3, 25);
						configs.antiaim.type[type].jitter_ticks_min = std::clamp(configs.antiaim.type[type].jitter_ticks_min, 3, configs.antiaim.type[type].jitter_ticks_max);
					}
				}

				if (configs.antiaim.type[type].desync)
				{
					if (type == ANTIAIM_STAND)
						ImGui::SingleSelectSkeet(crypt_str("LBY type"), &configs.antiaim.lby_type, lby_type);

					if (type != ANTIAIM_STAND || !configs.antiaim.lby_type)
					{
						ImGui::SingleSelectSkeet(crypt_str("Desync type"), &configs.antiaim.type[type].desync_mode, desync_type);
						if (configs.antiaim.type[type].desync_mode)
						{
							ImGui::SliderIntSkeet(crypt_str("Max desync range"), &configs.antiaim.type[type].desync_range, 1, 60, "%0.f");
							ImGui::SliderIntSkeet(crypt_str("Min desync range"), &configs.antiaim.type[type].inverted_desync_range, 1, 60, "%0.f");
							configs.antiaim.type[type].inverted_desync_range = min(configs.antiaim.type[type].desync_range, configs.antiaim.type[type].inverted_desync_range);
						}
						else
						{
							ImGui::SliderIntSkeet(crypt_str("Desync range"), &configs.antiaim.type[type].desync_range, 1, 60, "%0.f");
							ImGui::SliderIntSkeet(crypt_str("Inverted desync range"), &configs.antiaim.type[type].inverted_desync_range, 1, 60, "%0.f");
						}

						//ImGui::SingleSelectSkeet(crypt_str("Body lean type"), &configs.antiaim.type[type].body_lean_mode, body_lean_type);
						//if (configs.antiaim.type[type].body_lean_mode)
						//{
						//	ImGui::SliderIntSkeet(crypt_str("Max body lean"), &configs.antiaim.type[type].body_lean, 0, 100, "%0.f");
						//	ImGui::SliderIntSkeet(crypt_str("Min body lean"), &configs.antiaim.type[type].inverted_body_lean, 0, 100, "%0.f");
						//	configs.antiaim.type[type].inverted_body_lean = min(configs.antiaim.type[type].body_lean, configs.antiaim.type[type].inverted_body_lean);
						//}
						//else
						//{
						//	ImGui::SliderIntSkeet(crypt_str("Body lean"), &configs.antiaim.type[type].body_lean, 0, 100, "%0.f");
						//	ImGui::SliderIntSkeet(crypt_str("Inverted body lean"), &configs.antiaim.type[type].inverted_body_lean, 0, 100, "%0.f");
						//}
					}

					if (configs.antiaim.type[type].desync == 1)
						ImGui::KeybindSkeet(crypt_str("Invert desync"), &configs.antiaim.flip_desync.key, &configs.antiaim.flip_desync.mode);
				}

				ImGui::KeybindSkeet(crypt_str("Manual back"), &configs.antiaim.manual_back.key, &configs.antiaim.manual_back.mode);
				ImGui::KeybindSkeet(crypt_str("Manual left"), &configs.antiaim.manual_left.key, &configs.antiaim.manual_left.mode);
				ImGui::KeybindSkeet(crypt_str("Manual right"), &configs.antiaim.manual_right.key, &configs.antiaim.manual_right.mode);

				if (configs.antiaim.manual_back.key > KEY_NONE && configs.antiaim.manual_back.key < KEY_MAX || configs.antiaim.manual_left.key > KEY_NONE && configs.antiaim.manual_left.key < KEY_MAX || configs.antiaim.manual_right.key > KEY_NONE && configs.antiaim.manual_right.key < KEY_MAX)
				{
					ImGui::CheckboxSkeet(crypt_str("Manuals indicator"), &configs.antiaim.flip_indicator);
					ImGui::ColorEdit4Fixed("##FLIPINDICATOR__COLOR", &configs.antiaim.flip_indicator_color, ImGuiWindowFlags_NoTitleBar);
				}
			}
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(1));
		ImGui::BeginChildSkeet("FAKELAGS", child_size_d);
		{
			ImGui::CheckboxSkeet(crypt_str("Fake lag"), &configs.fakelag.enabled);
			if (configs.fakelag.enabled)
			{
				ImGui::SingleSelectSkeet(crypt_str("Type"), &configs.fakelag.type, fakelags);
				if (configs.fakelag.type == 1)
					ImGui::SliderIntSkeet(crypt_str("Min-Limit"), &configs.fakelag.min_amount, 1, 16, "%0.f");
				ImGui::SliderIntSkeet(crypt_str("Limit"), &configs.fakelag.amount, 1, 16, "%0.f");
				configs.fakelag.min_amount = std::clamp(configs.fakelag.min_amount, 1, configs.fakelag.amount);

				draw_multicomboSkeet(crypt_str("Triggers"), configs.fakelag.enablers, lagstrigger);

				auto enabled_fakelag_triggers = false;

				for (auto i = 0; i < lagstrigger.size(); i++)
					if (configs.fakelag.enablers[i])
						enabled_fakelag_triggers = true;

				if (enabled_fakelag_triggers)
					ImGui::SliderIntSkeet(crypt_str("Triggers limit"), &configs.fakelag.triggers_amount, 1, 16, "%0.f");
			}

			ImGui::CheckboxSkeet(crypt_str("No duck cooldown"), &configs.misc.no_duck_cooldown);

			if (configs.misc.no_duck_cooldown)
				ImGui::KeybindSkeet(crypt_str("Fake duck"), &configs.misc.fake_duck_key.key, &configs.misc.fake_duck_key.mode);

			ImGui::KeybindSkeet(crypt_str("Slow motion"), &configs.misc.slowwalk_key.key, &configs.misc.slowwalk_key.mode);
			ImGui::KeybindSkeet(crypt_str("Quick peek"), &configs.misc.automatic_peek.key, &configs.misc.automatic_peek.mode);
			ImGui::Text(crypt_str("Quickpeek Color"));
			ImGui::ColorEdit4Fixed(crypt_str("##QUICKPEEK__COLOR"), &configs.misc.automatic_peek_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::KeybindSkeet(crypt_str("Edge jump"), &configs.misc.edge_jump.key, &configs.misc.edge_jump.mode);
			ImGui::KeybindSkeet(crypt_str("Jump bug"), &configs.misc.jump_bug.key, &configs.misc.jump_bug.mode);
			ImGui::KeybindSkeet(crypt_str("Long jump"), &configs.misc.long_jump.key, &configs.misc.long_jump.mode);
			ImGui::KeybindSkeet(crypt_str("Mini jump"), &configs.misc.mini_jump.key, &configs.misc.mini_jump.mode);
		}
		ImGui::EndChildSkeet();
	}

	if (this->m_nCurrentTab == 3)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));

		ImGui::BeginChildSkeet("PLAYER ESP", child_size);
		{
			ImGui::SingleSelectSkeet(crypt_str("Chose"), &player, { "Enemy", "Team", "Local" });
			ImGui::CheckboxSkeet(crypt_str("Bounding box"), &configs.player.type[player].box);
			ImGui::ColorEdit4Fixed(crypt_str("##BOUNDINGBOX__COLOR"), &configs.player.type[player].box_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::CheckboxSkeet(crypt_str("Name"), &configs.player.type[player].name);
			ImGui::ColorEdit4Fixed(crypt_str("##PLAYERNAME__COLOR"), &configs.player.type[player].name_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::CheckboxSkeet(crypt_str("Health bar"), &configs.player.type[player].health);
			ImGui::CheckboxSkeet(crypt_str("Health color"), &configs.player.type[player].custom_health_color);
			ImGui::ColorEdit4Fixed(crypt_str("##HEALTHBAR__COLOR"), &configs.player.type[player].health_color, ImGuiWindowFlags_NoTitleBar);

			draw_multicomboSkeet(crypt_str("Flags"), configs.player.type[player].flags, flags);
			draw_multicomboSkeet(crypt_str("Weapon"), configs.player.type[player].weapon, weaponplayer);

			if (configs.player.type[player].weapon[WEAPON_ICON] || configs.player.type[player].weapon[WEAPON_TEXT])
			{
				ImGui::Text(crypt_str("Color"));
				ImGui::ColorEdit4Fixed(crypt_str("##WEAPON__COLOR"), &configs.player.type[player].weapon_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Skeleton"), &configs.player.type[player].skeleton);

			ImGui::ColorEdit4Fixed(crypt_str("##SKELETON__COLOR"), &configs.player.type[player].skeleton_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Ammo bar"), &configs.player.type[player].ammo);

			ImGui::ColorEdit4Fixed(crypt_str("##AMMOBAR__COLOR"), &configs.player.type[player].ammobar_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Footsteps"), &configs.player.type[player].footsteps);

			ImGui::ColorEdit4Fixed(crypt_str("##FOOTSTEPS__COLOR"), &configs.player.type[player].footsteps_color, ImGuiWindowFlags_NoTitleBar);

			if (configs.player.type[player].footsteps)
			{
				ImGui::SliderIntSkeet(crypt_str("Thickness"), &configs.player.type[player].thickness, 1, 10, "%.0f");
				ImGui::SliderIntSkeet(crypt_str("Radius"), &configs.player.type[player].radius, 50, 500, "%.0f");
			}

			if (player == ENEMY || player == TEAM)
			{
				ImGui::CheckboxSkeet(crypt_str("Snap lines"), &configs.player.type[player].snap_lines);

				ImGui::ColorEdit4Fixed(crypt_str("##SNAPLINE__COLOR"), &configs.player.type[player].snap_lines_color, ImGuiWindowFlags_NoTitleBar);

				if (player == ENEMY)
				{
					//if (configs.ragebot.enabled)
					//{
					//	ImGui::CheckboxSkeet(crypt_str("Multi points"), &configs.player.show_multi_points);
					//	ImGui::ColorEdit4Fixed(crypt_str("##MULTIPOINTS__COLOR"), &configs.player.show_multi_points_color, ImGuiWindowFlags_NoTitleBar);
					//}

					ImGui::CheckboxSkeet(crypt_str("Lag hitboxes"), &configs.player.lag_hitbox);

					ImGui::ColorEdit4Fixed(crypt_str("##LAGHITBOXES__COLOR"), &configs.player.lag_hitbox_color, ImGuiWindowFlags_NoTitleBar);
				}
			}
			else
			{
				ImGui::SingleSelectSkeet(crypt_str("Player model T"), &configs.player.player_model_t, player_model_t);

				ImGui::SingleSelectSkeet(crypt_str("Player model CT"), &configs.player.player_model_ct, player_model_ct);
			}

			if (player == ENEMY)
			{
				ImGui::CheckboxSkeet(crypt_str("Offscreen arrows"), &configs.player.arrows);

				ImGui::ColorEdit4Fixed(crypt_str("##OOFSCREENARROWS__COLOR"), &configs.player.arrows_color, ImGuiWindowFlags_NoTitleBar);

				if (configs.player.arrows)
				{
					ImGui::SliderIntSkeet(crypt_str("Arrows distance"), &configs.player.distance, 1, 100, "%.0f");
					ImGui::SliderIntSkeet(crypt_str("Arrows size"), &configs.player.size, 1, 100, "%.0f");
				}
			}
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size.y + 40));

		ImGui::BeginChildSkeet("COLORED MODELS", child_size);
		{
			if (player == LOCAL)
				ImGui::SingleSelectSkeet(crypt_str("Type"), &configs.player.local_chams_type, local_chams_type);

			if (player != LOCAL || !configs.player.local_chams_type)
				draw_multicomboSkeet(crypt_str("Chams"), configs.player.type[player].chams, configs.player.type[player].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis);

			if (configs.player.type[player].chams[PLAYER_CHAMS_VISIBLE] || player == LOCAL && configs.player.local_chams_type)
			{
				if (player == LOCAL && configs.player.local_chams_type)
				{
					ImGui::CheckboxSkeet(crypt_str("Enable desync chams"), &configs.player.fake_chams_enabled);
					ImGui::CheckboxSkeet(crypt_str("Visualize lag"), &configs.player.visualize_lag);
					ImGui::CheckboxSkeet(crypt_str("Layered"), &configs.player.layered);

					ImGui::SingleSelectSkeet(crypt_str("Player chams material"), &configs.player.fake_chams_type, chamstype);

					ImGui::Text(crypt_str("Color"));

					ImGui::ColorEdit4Fixed(crypt_str("##PLAYERFAKECHAMS__COLOR"), &configs.player.fake_chams_color, ImGuiWindowFlags_NoTitleBar);

					if (configs.player.fake_chams_type != 6)
					{
						ImGui::CheckboxSkeet(crypt_str("Double material "), &configs.player.fake_double_material);

						ImGui::ColorEdit4Fixed(crypt_str("##LOCALDOUBLEMATERIAL__COLOR"), &configs.player.fake_double_material_color, ImGuiWindowFlags_NoTitleBar);
					}

					ImGui::CheckboxSkeet(crypt_str("Animated material "), &configs.player.fake_animated_material);

					ImGui::ColorEdit4Fixed(crypt_str("##LOCALANIMATEDMATERIAL__COLOR"), &configs.player.fake_animated_material_color, ImGuiWindowFlags_NoTitleBar);
				}
				else
				{
					ImGui::SingleSelectSkeet(crypt_str("Player chams material"), &configs.player.type[player].chams_type, chamstype);

					if (configs.player.type[player].chams[PLAYER_CHAMS_VISIBLE])
					{
						ImGui::Text(crypt_str("Visible"));

						ImGui::ColorEdit4Fixed(crypt_str("##PLAYERCHAMSVISIBLE__COLOR"), &configs.player.type[player].chams_color, ImGuiWindowFlags_NoTitleBar);
					}

					if (configs.player.type[player].chams[PLAYER_CHAMS_VISIBLE] && configs.player.type[player].chams[PLAYER_CHAMS_INVISIBLE])
					{
						ImGui::Text(crypt_str("Invisible"));

						ImGui::ColorEdit4Fixed(crypt_str("##PLAYERCHAMSINVISIBLE__COLOR"), &configs.player.type[player].xqz_color, ImGuiWindowFlags_NoTitleBar);
					}

					if (configs.player.type[player].chams_type != 6)
					{
						ImGui::CheckboxSkeet(crypt_str("Double material"), &configs.player.type[player].double_material);

						ImGui::ColorEdit4Fixed(crypt_str("##PLAYERXDOUBLEMATERIAL__COLOR"), &configs.player.type[player].double_material_color, ImGuiWindowFlags_NoTitleBar);
					}

					ImGui::CheckboxSkeet(crypt_str("Animated material"), &configs.player.type[player].animated_material);

					ImGui::ColorEdit4Fixed(crypt_str("##PLAYERXANIMATEDMATERIAL__COLOR"), &configs.player.type[player].animated_material_color, ImGuiWindowFlags_NoTitleBar);

					if (player == ENEMY)
					{
						ImGui::CheckboxSkeet(crypt_str("Backtrack chams"), &configs.player.backtrack_chams);

						if (configs.player.backtrack_chams)
						{
							ImGui::SingleSelectSkeet(crypt_str("Backtrack chams material"), &configs.player.backtrack_chams_material, chamstype);

							ImGui::Text(crypt_str("Color"));

							ImGui::ColorEdit4Fixed(crypt_str("##PLAYERBACKTRACK__COLOR"), &configs.player.backtrack_chams_color, ImGuiWindowFlags_NoTitleBar);
						}
					}
				}
			}

			if (player == ENEMY || player == TEAM)
			{
				ImGui::CheckboxSkeet(crypt_str("Ragdoll chams"), &configs.player.type[player].ragdoll_chams);

				if (configs.player.type[player].ragdoll_chams)
				{
					ImGui::SingleSelectSkeet(crypt_str("Ragdoll chams material"), &configs.player.type[player].ragdoll_chams_material, chamstype);

					ImGui::Text(crypt_str("Color"));

					ImGui::ColorEdit4Fixed(crypt_str("##PLAYERRAGDOLL__COLOR"), &configs.player.type[player].ragdoll_chams_color, ImGuiWindowFlags_NoTitleBar);
				}
			}
			else if (!configs.player.local_chams_type)
			{
				ImGui::CheckboxSkeet(crypt_str("Transparency in scope"), &configs.player.transparency_in_scope);

				if (configs.player.transparency_in_scope)
					ImGui::SliderFloatSkeet(crypt_str("Alpha"), &configs.player.transparency_in_scope_amount, 0.0f, 1.0f, "%.0f");
			}

			ImGui::CheckboxSkeet(crypt_str("Glow"), &configs.player.type[player].glow);

			if (configs.player.type[player].glow)
			{
				ImGui::SingleSelectSkeet(crypt_str("Glow type"), &configs.player.type[player].glow_type, glowtype);
				ImGui::Text(crypt_str("Color"));

				ImGui::ColorEdit4Fixed(crypt_str("##PLAYERGLOW__COLOR"), &configs.player.type[player].glow_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Arms chams"), &configs.esp.arms_chams);

			ImGui::ColorEdit4Fixed(crypt_str("##PLAYERARMS__COLOR"), &configs.esp.arms_chams_color, ImGuiWindowFlags_NoTitleBar);


			ImGui::SingleSelectSkeet(crypt_str("Arms chams material"), &configs.esp.arms_chams_type, chamstype);

			if (configs.esp.arms_chams_type != 6)
			{
				ImGui::CheckboxSkeet(crypt_str("Arms double material"), &configs.esp.arms_double_material);

				ImGui::ColorEdit4Fixed(crypt_str("##PLAYERARMSDOUBLEMATERIAL__COLOR"), &configs.esp.arms_double_material_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Arms animated material"), &configs.esp.arms_animated_material);

			ImGui::ColorEdit4Fixed(crypt_str("##PLAYERAMRSANIMATEDMATERIAL__COLOR"), &configs.esp.arms_animated_material_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Weapon chams"), &configs.esp.weapon_chams);

			ImGui::ColorEdit4Fixed(crypt_str("##PLAYERWEAPONCHAMS__COLOR"), &configs.esp.weapon_chams_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::SingleSelectSkeet(crypt_str("Weapon chams material"), &configs.esp.weapon_chams_type, chamstype);

			if (configs.esp.weapon_chams_type != 6)
			{
				ImGui::CheckboxSkeet(crypt_str("Weapon double material"), &configs.esp.weapon_double_material);

				ImGui::ColorEdit4Fixed(crypt_str("##PLAYERXARMSDOUBLEMATERIAL__COLOR"), &configs.esp.weapon_double_material_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Weapon animated material"), &configs.esp.weapon_animated_material);

			ImGui::ColorEdit4Fixed(crypt_str("##PLAYERXAMRSANIMATEDMATERIAL__COLOR"), &configs.esp.weapon_animated_material_color, ImGuiWindowFlags_NoTitleBar);
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(1));

		ImGui::BeginChildSkeet("OTHER", child_size);
		{
			ImGui::CheckboxSkeet(crypt_str("Force crosshair"), &configs.esp.force_crosshair);
			ImGui::CheckboxSkeet(crypt_str("Deagle spinner"), &configs.skins.deagleSpinner);
			ImGui::SliderIntSkeet(crypt_str("Viewmodel field of view"), &configs.esp.viewmodel_fov, 0, 89, "%.0f");
			ImGui::SliderIntSkeet(crypt_str("Viewmodel X"), &configs.esp.viewmodel_x, -50, 50, "%.0f");
			ImGui::SliderIntSkeet(crypt_str("Viewmodel Y"), &configs.esp.viewmodel_y, -50, 50, "%.0f");
			ImGui::SliderIntSkeet(crypt_str("Viewmodel Z"), &configs.esp.viewmodel_z, -50, 50, "%.0f");
			ImGui::SliderIntSkeet(crypt_str("Viewmodel roll"), &configs.esp.viewmodel_roll, -180, 180, "%.0f");

			ImGui::CheckboxSkeet(crypt_str("Rain"), &configs.esp.rain);
			ImGui::CheckboxSkeet(crypt_str("Full bright"), &configs.esp.bright);

			ImGui::SingleSelectSkeet(crypt_str("Skybox changer"), &configs.esp.skybox_changer, skybox);

			ImGui::Text(crypt_str("Color"));

			ImGui::ColorEdit4Fixed(crypt_str("##SKYBOX__COLOR"), &configs.esp.skybox_color, ImGuiWindowFlags_NoTitleBar);

			if (configs.esp.skybox_changer == 21)
			{
				static char sky_custom[64] = "\0";

				if (!configs.esp.custom_skybox.empty())
					strcpy_s(sky_custom, sizeof(sky_custom), configs.esp.custom_skybox.c_str());

				ImGui::Text(crypt_str("Name"));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

				if (ImGui::InputText(crypt_str("##CUSTOMSKY__INPUT"), sky_custom, sizeof(sky_custom)))
					configs.esp.custom_skybox = sky_custom;

				ImGui::PopStyleVar();
			}

			ImGui::CheckboxSkeet(crypt_str("Color modulation"), &configs.esp.nightmode);

			if (configs.esp.nightmode)
			{
				ImGui::Text(crypt_str("World color"));

				ImGui::ColorEdit4Fixed(crypt_str("##WORLD__COLOR"), &configs.esp.world_color, ImGuiWindowFlags_NoTitleBar);

				ImGui::Text(crypt_str("Props color"));

				ImGui::ColorEdit4Fixed(crypt_str("##PROPS__COLOR"), &configs.esp.props_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("World modulation"), &configs.esp.world_modulation);

			if (configs.esp.world_modulation)
			{
				ImGui::SliderFloatSkeet(crypt_str("Bloom"), &configs.esp.bloom, 0.0f, 750.0f, "%.0f");
				ImGui::SliderFloatSkeet(crypt_str("Exposure"), &configs.esp.exposure, 0.0f, 2000.0f, "%.0f");
				ImGui::SliderFloatSkeet(crypt_str("Ambient"), &configs.esp.ambient, 0.0f, 1500.0f, "%.0f");
			}

			ImGui::CheckboxSkeet(crypt_str("Fog modulation"), &configs.esp.fog);

			if (configs.esp.fog)
			{
				ImGui::SliderIntSkeet(crypt_str("Distance"), &configs.esp.fog_distance, 0, 2500, "%.0f");
				ImGui::SliderIntSkeet(crypt_str("Density"), &configs.esp.fog_density, 0, 100, "%.0f");

				ImGui::Text(crypt_str("Color"));

				ImGui::ColorEdit4Fixed(crypt_str("##FOG__COLOR"), &configs.esp.fog_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Motionblur modulation"), &configs.esp.motionblur);

			if (configs.esp.motionblur)
			{
				ImGui::CheckboxSkeet(crypt_str("Motionblur Forward"), &configs.esp.motionblur_forward);
				ImGui::SliderFloatSkeet(crypt_str("Falling min"), &configs.esp.motionblur_falling_min, 0.0f, 50.0f, "%.2f");
				ImGui::SliderFloatSkeet(crypt_str("Falling max"), &configs.esp.motionblur_falling_max, 0.0f, 50.0f, "%.2f");
				ImGui::SliderFloatSkeet(crypt_str("Falling intesity"), &configs.esp.motionblur_falling_intensity, 0.0f, 8.0f, "%.2f");
				ImGui::SliderFloatSkeet(crypt_str("Rotation intensity"), &configs.esp.motionblur_rotation_intensity, 0.0f, 8.0f, "%.2f");
				ImGui::SliderFloatSkeet(crypt_str("Motionblur Strength"), &configs.esp.motionblur_strength, 0.0f, 8.0f, "%.2f");
			}

			ImGui::CheckboxSkeet(crypt_str("Lightning on hit"), &configs.esp.lightning_on_hit);

			if (configs.esp.lightning_on_hit)
			{
				ImGui::CheckboxSkeet(crypt_str("Lightning kill only"), &configs.esp.lightning_on_hit_killonly);
				ImGui::SliderIntSkeet(crypt_str("Lightning width"), &configs.esp.lightning_on_hit_width, 0, 30, "%.0f");
				ImGui::SliderIntSkeet(crypt_str("Lightning radius"), &configs.esp.lightning_on_hit_radius, 0, 1000, "%.0f");
				ImGui::SliderIntSkeet(crypt_str("Lightning beams"), &configs.esp.lightning_on_hit_beams, 0, 100, "%.0f");
			}

			ImGui::CheckboxSkeet(crypt_str("Smoke Color"), &configs.esp.smoke_color_enabled);
			ImGui::ColorEdit4Fixed(crypt_str("##SMOKE_COLOR"), &configs.esp.smoke_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::CheckboxSkeet(crypt_str("Molotov Color"), &configs.esp.molotov_color_enabled);
			ImGui::ColorEdit4Fixed(crypt_str("##MOLOTOV_COLOR"), &configs.esp.molotov_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::CheckboxSkeet(crypt_str("Console Color"), &configs.esp.console_color_enabled);
			ImGui::ColorEdit4Fixed(crypt_str("##CONSOLE_COLOR"), &configs.esp.console_color, ImGuiWindowFlags_NoTitleBar);
		}
		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 160.2 + child_size.x, ImGui::GetWindowPos().y + 40.8 + child_size.y));

		ImGui::BeginChildSkeet("EFFECTS", child_size);
		{
			draw_multicomboSkeet(crypt_str("Indicators"), configs.esp.indicators, indicators);

			draw_multicomboSkeet(crypt_str("Removals"), configs.esp.removals, removals);

			if (configs.esp.removals[REMOVALS_ZOOM])
				ImGui::CheckboxSkeet(crypt_str("Fix zoom sensivity"), &configs.esp.fix_zoom_sensivity);

			ImGui::CheckboxSkeet(crypt_str("Grenade warning"), &configs.esp.grenade_warning);
			ImGui::CheckboxSkeet(crypt_str("Grenade prediction"), &configs.esp.grenade_prediction);

			ImGui::ColorEdit4Fixed(crypt_str("##GRENADEPREDICTION__COLOR"), &configs.esp.grenade_prediction_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::Text("GP Traceline Color");
			ImGui::ColorEdit4Fixed(crypt_str("##GRENADEPREDICTIONTRACER__COLOR"), &configs.esp.grenade_prediction_tracer_color, ImGuiWindowFlags_NoTitleBar);

			if (configs.esp.grenade_prediction)
				ImGui::CheckboxSkeet(crypt_str("Always Prediction"), &configs.esp.grenade_prediction_click_only);

			ImGui::CheckboxSkeet(crypt_str("Grenade projectiles"), &configs.esp.projectiles);

			if (configs.esp.projectiles)
				draw_multicomboSkeet(crypt_str("Grenade ESP"), configs.esp.grenade_esp, proj_combo);

			if (configs.esp.grenade_esp[GRENADE_ICON] || configs.esp.grenade_esp[GRENADE_TEXT])
			{
				ImGui::Text(crypt_str("Color"));

				ImGui::ColorEdit4Fixed(crypt_str("##PROJECTILES__COLOR"), &configs.esp.projectiles_color, ImGuiWindowFlags_NoTitleBar);
			}

			if (configs.esp.grenade_esp[GRENADE_BOX])
			{
				ImGui::Text(crypt_str("Box color"));

				ImGui::ColorEdit4Fixed(crypt_str("##GRENADEBOX__COLOR"), &configs.esp.grenade_box_color, ImGuiWindowFlags_NoTitleBar);
			}

			if (configs.esp.grenade_esp[GRENADE_GLOW])
			{
				ImGui::Text(crypt_str("Glow color"));

				ImGui::ColorEdit4Fixed(crypt_str("##GRENADEGLOW__COLOR"), &configs.esp.grenade_glow_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Molotov timer"), &configs.esp.molotov_timer);

			ImGui::ColorEdit4Fixed(crypt_str("##MOLOTOVTIMER__COLOR"), &configs.esp.molotov_timer_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Smoke timer"), &configs.esp.smoke_timer);

			ImGui::ColorEdit4Fixed(crypt_str("##SMOKETIMER__COLOR"), &configs.esp.smoke_timer_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Bomb indicator"), &configs.esp.bomb_timer);
			draw_multicomboSkeet(crypt_str("Weapon ESP"), configs.esp.weapon, weaponesp);

			if (configs.esp.weapon[WEAPON_ICON] || configs.esp.weapon[WEAPON_TEXT] || configs.esp.weapon[WEAPON_DISTANCE])
			{
				ImGui::Text(crypt_str("Color"));

				ImGui::ColorEdit4Fixed(crypt_str("##WEAPONESP__COLOR"), &configs.esp.weapon_color, ImGuiWindowFlags_NoTitleBar);
			}

			if (configs.esp.weapon[WEAPON_BOX])
			{
				ImGui::Text(crypt_str("Box color"));

				ImGui::ColorEdit4Fixed(crypt_str("##WEAPONESPBOX__COLOR"), &configs.esp.box_color, ImGuiWindowFlags_NoTitleBar);
			}

			if (configs.esp.weapon[WEAPON_GLOW])
			{
				ImGui::Text(crypt_str("Glow color"));

				ImGui::ColorEdit4Fixed(crypt_str("##WEAPONESPGLOW__COLOR"), &configs.esp.weapon_glow_color, ImGuiWindowFlags_NoTitleBar);
			}

			if (configs.esp.weapon[WEAPON_AMMO])
			{
				ImGui::Text(crypt_str("Ammo bar color"));

				ImGui::ColorEdit4Fixed(crypt_str("##WEAPONESPAMMO__COLOR"), &configs.esp.weapon_ammo_color, ImGuiWindowFlags_NoTitleBar);
			}

			ImGui::CheckboxSkeet(crypt_str("Client bullet impacts"), &configs.esp.client_bullet_impacts);

			ImGui::ColorEdit4Fixed(crypt_str("##CLIENTBULLET__COLOR"), &configs.esp.client_bullet_impacts_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Server bullet impacts"), &configs.esp.server_bullet_impacts);

			ImGui::ColorEdit4Fixed(crypt_str("##SERVERBULLET__COLOR"), &configs.esp.server_bullet_impacts_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Local bullet tracers"), &configs.esp.bullet_tracer);

			ImGui::ColorEdit4Fixed(crypt_str("##LOCALBULLET__COLOR"), &configs.esp.bullet_tracer_color, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Enemy bullet tracers"), &configs.esp.enemy_bullet_tracer);

			ImGui::ColorEdit4Fixed(crypt_str("##ENEMYBULLET__COLOR"), &configs.esp.enemy_bullet_tracer_color, ImGuiWindowFlags_NoTitleBar);
			draw_multicomboSkeet(crypt_str("Hit marker"), configs.esp.hitmarker, hitmarkers);
			ImGui::CheckboxSkeet(crypt_str("Damage marker"), &configs.esp.damage_marker);
			//ImGui::CheckboxSkeet(crypt_str("Kill effect"), &configs.esp.kill_effect);

			//if (configs.esp.kill_effect)
			//	ImGui::SliderFloatSkeet(crypt_str("Duration"), &configs.esp.kill_effect_duration, 0.01f, 3.0f, "%.0f");

			ImGui::KeybindSkeet(crypt_str("Thirdperson"), &configs.misc.thirdperson_toggle.key, &configs.misc.thirdperson_toggle.mode);

			ImGui::CheckboxSkeet(crypt_str("Thirdperson when spectating"), &configs.misc.thirdperson_when_spectating);
			ImGui::CheckboxSkeet(crypt_str("Smooth thirdperson"), &configs.misc.smooth_thirdperson);
			//ImGui::CheckboxSkeet(crypt_str("Keep thirdperson"), &configs.misc.keep_thirdperson);
			ImGui::SliderIntSkeet(crypt_str("Thirdperson distance"), &configs.misc.thirdperson_distance, 100, 300, "%.0f");

			ImGui::SliderIntSkeet(crypt_str("Field of view"), &configs.esp.fov, 0, 89, "%.0f");

			ImGui::CheckboxSkeet(crypt_str("Taser range"), &configs.esp.taser_range);
			ImGui::CheckboxSkeet(crypt_str("Show spread"), &configs.esp.show_spread);

			ImGui::ColorEdit4Fixed(crypt_str("##SPREAD__COLOR"), &configs.esp.show_spread_color, ImGuiWindowFlags_NoTitleBar);
			ImGui::CheckboxSkeet(crypt_str("Penetration crosshair"), &configs.esp.penetration_reticle);

		}
		ImGui::EndChildSkeet();
	}

	if (this->m_nCurrentTab == 4)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));

		ImGui::BeginChildSkeet("MISC MAIN", child_size);
		{
			ImGui::Text(crypt_str("Menu theme color"));
			ImGui::ColorEdit3Fixed(crypt_str("##MENUTHEMECOLOR__COLOR"), &configs.menu.menu_theme, ImGuiWindowFlags_NoTitleBar);

			ImGui::CheckboxSkeet(crypt_str("Anti untrusted"), &configs.misc.anti_untrusted);

			if (ImGui::ButtonSkeet("Unlock hidden cvars", ImVec2(226, 0)))
			{
				auto iterator = **reinterpret_cast<ConCommandBase***>(reinterpret_cast<DWORD>(m_cvar()) + 0x34);
				for (auto c = iterator->m_pNext; c != nullptr; c = c->m_pNext)
				{
					ConCommandBase* cmd = c;
					cmd->m_nFlags &= ~FCVAR_DEVELOPMENTONLY;
					cmd->m_nFlags &= ~FCVAR_HIDDEN;
				}
			}

			//ImGui::CheckboxSkeet(crypt_str("Draw server hitboxes"), &configs.misc.server_hitbox);

			ImGui::CheckboxSkeet(crypt_str("Reveal votes"), &configs.misc.reveal_votes);

			ImGui::CheckboxSkeet(crypt_str("Rank reveal"), &configs.misc.rank_reveal);

			ImGui::CheckboxSkeet(crypt_str("Inventory unlocker"), &configs.misc.inventory_access);

			ImGui::CheckboxSkeet(crypt_str("Preserve killfeed"), &configs.esp.preserve_killfeed);

			ImGui::CheckboxSkeet("Killfeed changer", &configs.killfeedChanger.enabled);
			ImGui::SameLine();

			ImGui::PushID("Killfeed changer");
			if (ImGui::ButtonSkeet("...", ImVec2(50, 10)))
				ImGui::OpenPopup("");

			if (ImGui::BeginPopup("")) {
				ImGui::CheckboxSkeet("Headshot", &configs.killfeedChanger.headshot);
				ImGui::CheckboxSkeet("Dominated", &configs.killfeedChanger.dominated);
				ImGui::CheckboxSkeet("Revenge", &configs.killfeedChanger.revenge);
				ImGui::CheckboxSkeet("Penetrated", &configs.killfeedChanger.penetrated);
				ImGui::CheckboxSkeet("Noscope", &configs.killfeedChanger.noscope);
				ImGui::CheckboxSkeet("Thrusmoke", &configs.killfeedChanger.thrusmoke);
				ImGui::CheckboxSkeet("Attackerblind", &configs.killfeedChanger.attackerblind);
				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::SliderFloatSkeet(crypt_str("Aspect ratio"), &configs.misc.aspect_ratio, 0.0f, 5.0f, "%.2f");

			ImGui::SingleSelectSkeet(crypt_str("Watermark"), &configs.menu.watermark, watermarks);

			if (configs.menu.watermark)
				ImGui::CheckboxSkeet(crypt_str("Music status"), &configs.menu.music_status);

			ImGui::CheckboxSkeet(crypt_str("Spectators list"), &configs.misc.spectators_list);

			if (ImGui::ButtonSkeet(crypt_str("Reset spectators position"), ImVec2(226, 0)))
			{
				configs.misc.spectatorlist_pos_x = 0;
				configs.misc.spectatorlist_pos_y = 0;
			}

			ImGui::CheckboxSkeet(crypt_str("Keybind list"), &configs.misc.keybind_list);

			if (ImGui::ButtonSkeet(crypt_str("Reset keybind position"), ImVec2(226, 0)))
			{
				configs.misc.keybinds_pos_x = 0;
				configs.misc.keybinds_pos_y = 0;
			}

			ImGui::CheckboxSkeet(crypt_str("InGame radar"), &configs.radar.enabled);
			ImGui::SameLine();

			ImGui::PushID("InGame radar");
			if (ImGui::ButtonSkeet("...", ImVec2(50, 10)))
				ImGui::OpenPopup("");

			if (ImGui::BeginPopup("")) {
				ImGui::CheckboxSkeet(crypt_str("Local"), &configs.radar.local);
				ImGui::CheckboxSkeet(crypt_str("Enemy"), &configs.radar.enemy);
				ImGui::CheckboxSkeet(crypt_str("Teammate"), &configs.radar.teammate);
				ImGui::CheckboxSkeet(crypt_str("Planted c4 radar"), &configs.radar.planted_c4);
				ImGui::CheckboxSkeet(crypt_str("Dropped c4 radar"), &configs.radar.dropped_c4);
				ImGui::CheckboxSkeet(crypt_str("Health"), &configs.radar.health);
				ImGui::EndPopup();
			}
			ImGui::PopID();

			ImGui::SingleSelectSkeet(crypt_str("Hitsound"), &configs.esp.hitsound, sounds);
			if (configs.esp.hitsound == 7)
			{
				static char hitsound_custom[64] = "\0";

				if (!configs.esp.custom_hitsound.empty())
					strcpy_s(hitsound_custom, sizeof(hitsound_custom), configs.esp.custom_hitsound.c_str());

				ImGui::Text(crypt_str("Sound file name"));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				if (ImGui::InputText(crypt_str("##CUSTOMHITSOUND__INPUT"), hitsound_custom, sizeof(hitsound_custom)))
					configs.esp.custom_hitsound = hitsound_custom;

				ImGui::PopStyleVar();
			}
			ImGui::SingleSelectSkeet(crypt_str("Killsound"), &configs.esp.killsound, sounds);
			if (configs.esp.killsound == 7)
			{
				static char killsound_custom[64] = "\0";

				if (!configs.esp.custom_killsound.empty())
					strcpy_s(killsound_custom, sizeof(killsound_custom), configs.esp.custom_killsound.c_str());

				ImGui::Text(crypt_str("Sound file name"));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				if (ImGui::InputText(crypt_str("##CUSTOMKILLSOUND__INPUT"), killsound_custom, sizeof(killsound_custom)))
					configs.esp.custom_killsound = killsound_custom;

				ImGui::PopStyleVar();
			}

			draw_multicomboSkeet(crypt_str("Logs"), configs.misc.events_to_log, events);

			draw_multicomboSkeet(crypt_str("Logs output"), configs.misc.log_output, events_output);

			if (configs.misc.events_to_log[EVENTLOG_HIT] || configs.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || configs.misc.events_to_log[EVENTLOG_BOMB])
			{
				ImGui::Text(crypt_str("Color"));

				ImGui::ColorEdit4Fixed(crypt_str("##LOG__COLOR"), &configs.misc.log_color, ImGuiWindowFlags_NoTitleBar);
			}

			//ImGui::CheckboxSkeet(crypt_str("Show CS:GO logs"), &configs.misc.show_default_log);
			ImGui::CheckboxSkeet(crypt_str("Gravity ragdolls"), &configs.misc.ragdolls);
			ImGui::CheckboxSkeet(crypt_str("sv_pure bypass"), &configs.misc.sv_pure_bypass);
			ImGui::CheckboxSkeet(crypt_str("Automatic jump"), &configs.misc.bunnyhop);
			ImGui::SingleSelectSkeet(crypt_str("Automatic strafes"), &configs.misc.airstrafe, strafes);
			ImGui::SingleSelectSkeet(crypt_str("Force relay cluster"), &configs.misc.force_relay_cluster, centersList);
			ImGui::CheckboxSkeet(crypt_str("Crouch in air"), &configs.misc.crouch_in_air);
			ImGui::CheckboxSkeet(crypt_str("Quick stop"), &configs.misc.fast_stop);
			ImGui::SingleSelectSkeet(crypt_str("Slide walk"), &configs.misc.slide_walk, legs);
			ImGui::CheckboxSkeet(crypt_str("Air static legs"), &configs.misc.air_static_legs);
			ImGui::CheckboxSkeet(crypt_str("Anti screenshot"), &configs.misc.anti_screenshot);
			ImGui::CheckboxSkeet(crypt_str("Clan tag"), &configs.misc.clan_tag);
			ImGui::CheckboxSkeet(crypt_str("Radar hack"), &configs.misc.radar_hack);
			ImGui::CheckboxSkeet(crypt_str("Reveal money"), &configs.misc.reveal_money);
			ImGui::CheckboxSkeet(crypt_str("Reveal suspect"), &configs.misc.reveal_suspect);
			ImGui::CheckboxSkeet(crypt_str("Auto accept matchmaking"), &configs.misc.auto_accept);
			ImGui::CheckboxSkeet(crypt_str("Anti server crasher"), &configs.misc.anti_crasher);
			ImGui::CheckboxSkeet(crypt_str("Anti exploit"), &configs.misc.anti_exploit);
			ImGui::CheckboxSkeet(crypt_str("Anti afk"), &configs.misc.anti_afk);
			ImGui::CheckboxSkeet(crypt_str("Remove serverside ads"), &configs.misc.ad_block);
			ImGui::CheckboxSkeet(crypt_str("Slider to input"), &configs.misc.slider_to_input);
			ImGui::CheckboxSkeet(crypt_str("Blockbot"), &configs.blockbot.enabled);
			if (configs.blockbot.enabled)
			{
				ImGui::KeybindSkeet(crypt_str("##BLOCKBOT__HOTKEY"), &configs.blockbot.key.key, &configs.blockbot.key.mode);
				ImGui::SingleSelectSkeet(crypt_str("Blockbot type"), &configs.blockbot.type, blockbot_type);
			}
			ImGui::CheckboxSkeet(crypt_str("Enable buybot"), &configs.misc.buybot_enabled);

			if (configs.misc.buybot_enabled)
			{
				ImGui::SingleSelectSkeet(crypt_str("Snipers"), &configs.misc.buybot1, mainwep);

				ImGui::SingleSelectSkeet(crypt_str("Pistols"), &configs.misc.buybot2, secwep);

				draw_multicomboSkeet(crypt_str("Other"), configs.misc.buybot3, grenades);
			}

			draw_multicomboSkeet(crypt_str("Walkbot"), configs.misc.walkbot, walkbot_type);
			ImGui::KeybindSkeet(crypt_str("Create point"), &configs.misc.walkbot_key.key, &configs.misc.walkbot_key.mode);

			static char addictname[64] = "\0";

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			ImGui::InputText(crypt_str("##ADDICTNAME__INPUT"), addictname, sizeof(addictname));
			ImGui::PopStyleVar();
			if (ImGui::ButtonSkeet(crypt_str("Save path"), ImVec2(226, 0)))
				walkbot::get().file(addictname, file_overwrite);
			if (ImGui::ButtonSkeet(crypt_str("Load path"), ImVec2(226, 0)))
				walkbot::get().file(addictname, file_load);
			if (ImGui::ButtonSkeet(crypt_str("Clear path"), ImVec2(226, 0)))
				walkbot::get().clear_dots();
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size.y + 40));

		ImGui::BeginChildSkeet("SCRIPTS LUA", child_size);
		{
			buttonPositionX = child_size.x - 64;
			static auto should_update = true;

			if (should_update)
			{
				should_update = false;
				scripts = c_lua::get().scripts;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			if (ImGui::ButtonSkeet(crypt_str("Open scripts folder"), ImVec2(buttonPositionX, 0)))
			{
				ShellExecuteA(NULL, crypt_str("open"), cfg_manager->get_folder_path(PathType::Lua).c_str(), NULL, NULL, SW_SHOWNORMAL);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

			if (scripts.empty())
				ImGui::ListBoxConfigArraySkeet(crypt_str("##LUAS"), &selected_script, scripts, 7);
			else
			{
				auto& backup_scripts = scripts;

				for (auto& script : scripts)
				{
					auto script_id = c_lua::get().get_script_id(script + crypt_str(".lua"));

					if (script_id == -1)
						continue;

					if (c_lua::get().loaded.at(script_id))
						scripts.at(script_id) += crypt_str(" [loaded]");
				}

				ImGui::ListBoxConfigArraySkeet(crypt_str("##LUAS"), &selected_script, scripts, 7);
				scripts = std::move(backup_scripts);
			}

			ImGui::PopStyleVar();

			if (ImGui::ButtonSkeet(crypt_str("Refresh scripts"), ImVec2(buttonPositionX, 0)))
			{
				c_lua::get().refresh_scripts();
				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			//if (ImGui::Button(crypt_str("Edit script"), ImVec2(buttonPositionX, 0)))
			//{
			//	loaded_editing_script = false;
			//	editing_script = scripts.at(selected_script);
			//}

			if (ImGui::ButtonSkeet(crypt_str("Load script"), ImVec2(buttonPositionX, 0)))
			{
				c_lua::get().load_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(crypt_str("Successed load ") + scripts.at(selected_script) + crypt_str(" script"), false);
			}

			if (ImGui::ButtonSkeet(crypt_str("Unload script"), ImVec2(buttonPositionX, 0)))
			{
				c_lua::get().unload_script(selected_script);
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}

				eventlogs::get().add(crypt_str("Successed unload ") + scripts.at(selected_script) + crypt_str(" script"), false);
			}

			if (ImGui::ButtonSkeet(crypt_str("Reload all scripts"), ImVec2(buttonPositionX, 0)))
			{
				c_lua::get().reload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}

			if (ImGui::ButtonSkeet(crypt_str("Unload all scripts"), ImVec2(buttonPositionX, 0)))
			{
				c_lua::get().unload_all_scripts();
				c_lua::get().refresh_scripts();

				scripts = c_lua::get().scripts;

				if (selected_script >= scripts.size())
					selected_script = scripts.size() - 1;

				for (auto& current : scripts)
				{
					if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
						current.erase(current.size() - 5, 5);
					else if (current.size() >= 4)
						current.erase(current.size() - 4, 4);
				}
			}
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(1));

		ImGui::BeginChildSkeet("LUA ELEMENTS", child_size);
		{
			ImGui::CheckboxSkeet(crypt_str("Developer mode"), &configs.scripts.developer_mode);
			ImGui::CheckboxSkeet(crypt_str("Allow unsafe scripts"), &configs.scripts.allow_unsafe_scripts);
			buttonPositionX = child_size.x - 64;

			auto previous_check_box = false;

			//for (auto& current : c_lua::get().scripts)
			if (selected_script > 0)
			{
				auto& current = c_lua::get().scripts.at(selected_script);
				auto& items = c_lua::get().items.at(c_lua::get().get_script_id(current));

				for (auto& item : items)
				{
					std::string item_name;

					auto first_point = false;
					auto item_str = false;

					for (auto& c : item.first)
					{
						if (c == '.')
						{
							if (first_point)
							{
								item_str = true;
								continue;
							}
							else
								first_point = true;
						}

						if (item_str)
							item_name.push_back(c);
					}

					switch (item.second.type)
					{
					case NEXT_LINE:
						previous_check_box = false;
						break;
					case CHECK_BOX:
						previous_check_box = true;
						ImGui::CheckboxSkeet(item_name.c_str(), &item.second.check_box_value);
						break;
					case COMBO_BOX:
						previous_check_box = false;
						ImGui::SingleSelectSkeet(item_name.c_str(), &item.second.combo_box_value, [](void* data, int idx, const char** out_text)
							{
								auto labels = (std::vector <std::string>*)data;
								*out_text = labels->at(idx).c_str();
								return true;
							}, item.second.combo_box_labels.size());
						break;
					case SLIDER_INT:
						previous_check_box = false;
						ImGui::SliderIntSkeet(item_name.c_str(), &item.second.slider_int_value, item.second.slider_int_min, item.second.slider_int_max);
						break;
					case SLIDER_FLOAT:
						previous_check_box = false;
						ImGui::SliderFloatSkeet(item_name.c_str(), &item.second.slider_float_value, item.second.slider_float_min, item.second.slider_float_max);
						break;
					case COLOR_PICKER:
						if (previous_check_box)
							previous_check_box = false;
						else
							ImGui::Text((item_name + ' ').c_str());

						ImGui::ColorEdit4Fixed((crypt_str("##") + item_name).c_str(), &item.second.color_picker_value, ImGuiWindowFlags_NoTitleBar);
						break;
					}
				}
			}
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 160.2 + child_size.x, ImGui::GetWindowPos().y + 40.8 + child_size.y));

		ImGui::BeginChildSkeet("CONFIGS", child_size, 0);
		{
			buttonPositionX = child_size.x - 64;
			static auto should_update = true;

			if (should_update)
			{
				should_update = false;

				cfg_manager->config_files();
				files = cfg_manager->files;
			}

			if (ImGui::ButtonSkeet(crypt_str("Open configs folder"), ImVec2(buttonPositionX, 0)))
			{
				ShellExecuteA(NULL, crypt_str("open"), cfg_manager->get_folder_path(PathType::Configs).c_str(), NULL, NULL, SW_SHOWNORMAL);
			}

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			ImGui::ListBoxConfigArraySkeet(crypt_str("##CONFIGS"), &configs.selected_config, files, 7);
			ImGui::PopStyleVar();

			if (ImGui::ButtonSkeet(crypt_str("Refresh configs"), ImVec2(buttonPositionX, 0)))
			{
				cfg_manager->config_files();
				files = cfg_manager->files;
			}

			static char config_name[64] = "\0";

			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
			ImGui::Text("Cfg name");
			ImGui::InputText(crypt_str("##CONFIGNAME__INPUT"), config_name, sizeof(config_name));
			ImGui::PopStyleVar();

			if (ImGui::ButtonSkeet(crypt_str("Create config"), ImVec2(buttonPositionX, 0)))
			{
				configs.new_config_name = config_name;
				add_config();
			}

			static auto next_save = false;
			static auto prenext_save = false;
			static auto clicked_sure = false;
			static auto save_time = m_globals()->m_realtime;
			static auto save_alpha = 1.0f;

			save_alpha = math::clamp(save_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_save ? 1.f : -1.f)), 0.01f, 1.f);
			//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, save_alpha * public_alpha * (1.f - preview_alpha));

			if (!next_save)
			{
				clicked_sure = false;

				if (prenext_save && save_alpha <= 0.01f)
					next_save = true;

				if (ImGui::ButtonSkeet(crypt_str("Save config"), ImVec2(buttonPositionX, 0)))
				{
					save_time = m_globals()->m_realtime;
					prenext_save = true;
				}
			}
			else
			{
				if (prenext_save && save_alpha <= 0.01f)
				{
					prenext_save = false;
					next_save = !clicked_sure;
				}

				if (ImGui::ButtonSkeet(crypt_str("Are you sure?"), ImVec2(buttonPositionX, 0)))
				{
					save_config();
					prenext_save = true;
					clicked_sure = true;
				}

				if (!clicked_sure && m_globals()->m_realtime > save_time + 1.5f)
				{
					prenext_save = true;
					clicked_sure = true;
				}
			}

			//ImGui::PopStyleVar();

			if (ImGui::ButtonSkeet(crypt_str("Load config"), ImVec2(buttonPositionX, 0)))
				load_config();

			static auto next_delete = false;
			static auto prenext_delete = false;
			static auto clicked_sure_del = false;
			static auto delete_time = m_globals()->m_realtime;
			static auto delete_alpha = 1.0f;

			delete_alpha = math::clamp(delete_alpha + (4.f * ImGui::GetIO().DeltaTime * (!prenext_delete ? 1.f : -1.f)), 0.01f, 1.f);
			//ImGui::PushStyleVar(ImGuiStyleVar_Alpha, delete_alpha * public_alpha * (1.f - preview_alpha));

			if (!next_delete)
			{
				clicked_sure_del = false;

				if (prenext_delete && delete_alpha <= 0.01f)
					next_delete = true;

				if (ImGui::ButtonSkeet(crypt_str("Remove config"), ImVec2(buttonPositionX, 0)))
				{
					delete_time = m_globals()->m_realtime;
					prenext_delete = true;
				}
			}
			else
			{
				if (prenext_delete && delete_alpha <= 0.01f)
				{
					prenext_delete = false;
					next_delete = !clicked_sure_del;
				}

				if (ImGui::ButtonSkeet(crypt_str("Are you sure?"), ImVec2(buttonPositionX, 0)))
				{
					remove_config();
					prenext_delete = true;
					clicked_sure_del = true;
				}

				if (!clicked_sure_del && m_globals()->m_realtime > delete_time + 1.5f)
				{
					prenext_delete = true;
					clicked_sure_del = true;
				}
			}

		}
		ImGui::EndChildSkeet();
	}

	if (this->m_nCurrentTab == 5)
	{
		static bool drugs = false;

		// some animation logic(switch)
		static bool active_animation = false;
		static bool preview_reverse = false;
		static float switch_alpha = 1.f;
		static int next_id = -1;
		if (active_animation)
		{
			if (preview_reverse)
			{
				if (switch_alpha == 1.f)
				{
					preview_reverse = false;
					active_animation = false;
				}

				switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
			else
			{
				if (switch_alpha == 0.01f)
				{
					preview_reverse = true;
				}

				switch_alpha = math::clamp(switch_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
			}
		}
		else
			switch_alpha = math::clamp(switch_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

		auto child_size_qw = ImVec2(ImGui::GetWindowSize().x - 140, ImGui::GetWindowSize().y - 60);

		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));
		ImGui::BeginChildSkeet("SKINS", child_size_qw);
		{
			// we need to count our items in 1 line
			auto same_line_counter = 0;

			// if we didnt choose any weapon
			if (c_menu::get().current_profile == -1)
			{
				for (auto i = 0; i < configs.skins.skinChanger.size(); i++)
				{
					// do we need update our preview for some reasons?
					if (!all_skins[i])
					{
						configs.skins.skinChanger.at(i).update();
						all_skins[i] = get_skin_preview(get_weapon(i, (i == 0 || i == 1) ? configs.skins.skinChanger.at(i).definition_override_vector_index : -1, i == 0).c_str(), configs.skins.skinChanger.at(i).skin_name, c_menu::get().device);
					}

					// we licked on weapon
					if (ImGui::ImageButton(all_skins[i], ImVec2(107, 76)))
					{
						next_id = i;
						active_animation = true;
					}

					// if our animation step is half from all - switch profile
					if (active_animation && preview_reverse)
					{
						ImGui::SetScrollY(0);
						c_menu::get().current_profile = next_id;
					}

					if (same_line_counter < 4) { // continue push same-line
						ImGui::SameLine();
						same_line_counter++;
					}
					else { // we have maximum elements in 1 line
						same_line_counter = 0;
					}
				}
			}
			else
			{
				// update skin preview bool
				static bool need_update[36];

				// we pressed crypt_str("Save & Close") button
				static bool leave;

				// update if we have nullptr texture or if we push force update
				if (!all_skins[c_menu::get().current_profile] || need_update[c_menu::get().current_profile])
				{
					all_skins[c_menu::get().current_profile] = get_skin_preview(get_weapon(c_menu::get().current_profile, (c_menu::get().current_profile == 0 || c_menu::get().current_profile == 1) ? configs.skins.skinChanger.at(c_menu::get().current_profile).definition_override_vector_index : -1, c_menu::get().current_profile == 0).c_str(), configs.skins.skinChanger.at(c_menu::get().current_profile).skin_name, c_menu::get().device);
					need_update[c_menu::get().current_profile] = false;
				}

				// get settings for selected weapon
				auto& selected_entry = configs.skins.skinChanger[c_menu::get().current_profile];
				selected_entry.itemIdIndex = c_menu::get().current_profile;

				// search input later
				static char search_skins[64] = "\0";
				static auto item_index = selected_entry.paint_kit_vector_index;

				if (!c_menu::get().current_profile)
				{
					if (ImGui::SingleSelectSkeet("Knife model", &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = game_data::knife_names[idx].name;
							return true;
						}, IM_ARRAYSIZE(game_data::knife_names)))
						need_update[c_menu::get().current_profile] = true; // push force update
				}
				else if (c_menu::get().current_profile == 1)
				{
					//ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 125 - 111);
					if (ImGui::SingleSelectSkeet(crypt_str("Glove model"), &selected_entry.definition_override_vector_index, [](void* data, int idx, const char** out_text)
						{
							*out_text = game_data::glove_names[idx].name;
							return true;
						}, IM_ARRAYSIZE(game_data::glove_names)))
					{
						item_index = 0; // set new generated paintkits element to 0;
						need_update[c_menu::get().current_profile] = true; // push force update
					}
				}
				else
					selected_entry.definition_override_vector_index = 0;

				if (c_menu::get().current_profile != 1)
				{
					ImGui::Text(crypt_str("Search"));
					ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
					ImGui::PushFont(unicodeFont.fonts.chinese10);

					if (ImGui::InputText(crypt_str("##SEARCH__INPUT"), search_skins, sizeof(search_skins)))
						item_index = -1;

					ImGui::PopFont();
					ImGui::PopStyleVar();
				}

				auto& main_kits = c_menu::get().current_profile == 1 ? SkinChanger::gloveKits : SkinChanger::skinKits;
				auto display_index = 0;

				SkinChanger::displayKits = main_kits;

				// we dont need custom gloves
				if (c_menu::get().current_profile == 1)
				{
					for (auto i = 0; i < main_kits.size(); i++)
					{
						auto& main_name = main_kits.at(i).name;

						for (auto i = 0; i < main_name.size(); i++)
							if (iswalpha((main_name.at(i))))
								main_name.at(i) = towlower(main_name.at(i));

						char search_name[64];

						if (!strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Broken Fang")) || !strcmp(game_data::glove_names[selected_entry.definition_override_vector_index].name, crypt_str("Hydra")))
							strcpy_s(search_name, sizeof(search_name), crypt_str("Bloodhound"));
						else
							strcpy_s(search_name, sizeof(search_name), game_data::glove_names[selected_entry.definition_override_vector_index].name);

						for (auto i = 0; i < sizeof(search_name); i++)
							if (iswalpha(search_name[i]))
								search_name[i] = towlower(search_name[i]);

						if (main_name.find(search_name) != std::string::npos)
						{
							SkinChanger::displayKits.at(display_index) = main_kits.at(i);
							display_index++;
						}
					}

					SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
				}
				else
				{
					if (strcmp(search_skins, crypt_str("")))
					{
						for (auto i = 0; i < main_kits.size(); i++)
						{
							auto& main_name = main_kits.at(i).name;

							for (auto i = 0; i < main_name.size(); i++)
								if (iswalpha(main_name.at(i)))
									main_name.at(i) = towlower(main_name.at(i));

							char search_name[64];
							strcpy_s(search_name, sizeof(search_name), search_skins);

							for (auto i = 0; i < sizeof(search_name); i++)
								if (iswalpha(search_name[i]))
									search_name[i] = towlower(search_name[i]);

							if (main_name.find(search_name) != std::string::npos)
							{
								SkinChanger::displayKits.at(display_index) = main_kits.at(i);
								display_index++;
							}
						}

						SkinChanger::displayKits.erase(SkinChanger::displayKits.begin() + display_index, SkinChanger::displayKits.end());
					}
					else
						item_index = selected_entry.paint_kit_vector_index;
				}

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				if (!SkinChanger::displayKits.empty())
				{
					ImGui::PushFont(unicodeFont.fonts.chinese10);
					if (ImGui::ListBoxSkeet(crypt_str("##PAINTKITS"), &item_index, [](void* data, int idx, const char** out_text)
						{
							while (SkinChanger::displayKits.at(idx).name.find(crypt_str("\u0451")) != std::string::npos)
								SkinChanger::displayKits.at(idx).name.replace(SkinChanger::displayKits.at(idx).name.find(crypt_str("\u0451")), 2, crypt_str("\u0435"));

							*out_text = SkinChanger::displayKits.at(idx).name.c_str();
							return true;
						}, nullptr, SkinChanger::displayKits.size(), SkinChanger::displayKits.size() > 9 ? 9 : SkinChanger::displayKits.size()) || !all_skins[c_menu::get().current_profile])
					{
						SkinChanger::scheduleHudUpdate();
						need_update[c_menu::get().current_profile] = true;

						auto i = 0;

						while (i < main_kits.size())
						{
							if (main_kits.at(i).id == SkinChanger::displayKits.at(item_index).id)
							{
								selected_entry.paint_kit_vector_index = i;
								break;
							}

							i++;
						}

					}
						ImGui::PopFont();
				}

				ImGui::PopStyleVar();

				ImGui::Text("Seed");

				if (ImGui::InputInt(crypt_str("##SEED__INPUT"), &selected_entry.seed, 1, 100))
					SkinChanger::scheduleHudUpdate();

				ImGui::Text("StatTrak");

				if (ImGui::InputInt(crypt_str("##STATTRAK__INPUT"), &selected_entry.stat_trak, 1, 15))
					SkinChanger::scheduleHudUpdate();

				if (ImGui::SliderFloatSkeet(crypt_str("Wear"), &selected_entry.wear, 0.0f, 1.0f, "%.2f"))
				{
					drugs = true;
				}
				else if (drugs)
				{
					SkinChanger::scheduleHudUpdate();
					drugs = false;
				}

				if (ImGui::SingleSelectSkeet(crypt_str("Quality"), &selected_entry.entity_quality_vector_index, [](void* data, int idx, const char** out_text)
					{
						*out_text = game_data::quality_names[idx].name;
						return true;
					}, IM_ARRAYSIZE(game_data::quality_names)))
					SkinChanger::scheduleHudUpdate();

					if (c_menu::get().current_profile != 1)
					{
						if (!configs.skins.custom_name_tag[c_menu::get().current_profile].empty())
							strcpy_s(selected_entry.custom_name, sizeof(selected_entry.custom_name), configs.skins.custom_name_tag[c_menu::get().current_profile].c_str());

						ImGui::Text(crypt_str("Name Tag"));
						ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

						if (ImGui::InputText(crypt_str("##NAMETAG__INPUT"), selected_entry.custom_name, sizeof(selected_entry.custom_name)))
						{
							configs.skins.custom_name_tag[c_menu::get().current_profile] = selected_entry.custom_name;
							SkinChanger::scheduleHudUpdate();
						}

						ImGui::PopStyleVar();
					}

					if (ImGui::ButtonSkeet(crypt_str("Close"), ImVec2(198, 26)))
					{
						active_animation = true;
						next_id = -1;
						leave = true;
					}

					// update element
					selected_entry.update();

					// we need to reset profile in the end to prevent render images with massive's index == -1
					if (leave && (preview_reverse || !active_animation))
					{
						ImGui::SetScrollY(0);
						c_menu::get().current_profile = next_id;
						leave = false;
					}
			}
		}
		ImGui::EndChildSkeet();
	}

	auto child_size_p = ImVec2(ImGui::GetWindowSize().x - 140, ImGui::GetWindowSize().y / 13.2 + 160);

	auto child_size_a = ImVec2(ImGui::GetWindowSize().x - 140, ImGui::GetWindowSize().y / 13.2 + 115);

	if (this->m_nCurrentTab == 6)
	{
		ImGui::SetNextWindowPos(calculateChildWindowPositionDouble(0));
		ImGui::BeginChildSkeet("PROFILE_CHANGER", child_size_p);
		{
			ImGui::CheckboxSkeet("Enabled", &configs.profileChanger.enabled);
			ImGui::SingleSelectSkeet("Rank", &configs.profileChanger.rank, match_rank);

			ImGui::Text("Wins");
			ImGui::InputInt("##WINS", &configs.profileChanger.wins);
			configs.profileChanger.wins = std::clamp(configs.profileChanger.wins, 0, configs.profileChanger.wins);

			ImGui::SingleSelectSkeet("Wingman_Rank", &configs.profileChanger.r_rank, match_rank);

			ImGui::Text("Wingman_Wins");
			ImGui::InputInt("##WINGMAN__INPUT", &configs.profileChanger.r_wins);
			configs.profileChanger.r_wins = std::clamp(configs.profileChanger.r_wins, 0, configs.profileChanger.r_wins);

			ImGui::SingleSelectSkeet("DangerZone_Rank", &configs.profileChanger.t_rank, dangerzone_rank);

			ImGui::Text("DangerZone_Wins");
			ImGui::InputInt("##DANGERZONE__INPUT", &configs.profileChanger.t_wins);
			configs.profileChanger.t_wins = std::clamp(configs.profileChanger.t_wins, 0, configs.profileChanger.t_wins);

			ImGui::SliderIntSkeet("Level", &configs.profileChanger.level, 0, 40);

			ImGui::Text("EXP");
			ImGui::InputInt("##EXP__INPUT", &configs.profileChanger.exp);
			configs.profileChanger.exp = std::clamp(configs.profileChanger.exp, 0, configs.profileChanger.exp);

			ImGui::Text("Friendly");
			ImGui::InputInt("##FRIENDLY__INPUT", &configs.profileChanger.friendly);
			configs.profileChanger.friendly = std::clamp(configs.profileChanger.friendly, 0, configs.profileChanger.friendly);

			ImGui::Text("Teacher");
			ImGui::InputInt("##TEACHER__INPUT", &configs.profileChanger.teacher);
			configs.profileChanger.teacher = std::clamp(configs.profileChanger.teacher, 0, configs.profileChanger.teacher);

			ImGui::Text("Leader");
			ImGui::InputInt("##LEADER__INPUT", &configs.profileChanger.leader);
			configs.profileChanger.leader = std::clamp(configs.profileChanger.leader, 0, configs.profileChanger.leader);

			ImGui::SingleSelectSkeet("Fake ban", &configs.profileChanger.ban_type, cooldowns);
			ImGui::SliderIntSkeet("Fake ban time", &configs.profileChanger.ban_time, 0, 1000, "Seconds: %d");

			if (ImGui::ButtonSkeet("ApplyMatchMaking", ImVec2(190, 30)) && configs.profileChanger.enabled)
			{
				write.sendClientHello();
				write.sendClientGcRankUpdateMatchmaking();
			}

			if (ImGui::ButtonSkeet("ApplyWingman", ImVec2(190, 30)) && configs.profileChanger.enabled)
			{
				write.sendClientHello();
				write.sendClientGcRankUpdateWingman();
			}

			if (ImGui::ButtonSkeet("ApplyZone", ImVec2(190, 30)) && configs.profileChanger.enabled)
			{
				write.sendClientHello();
				write.sendClientGcRankUpdateZone();
			}

			ImGui::CheckboxSkeet("Fake prime", &configs.profileChanger.fake_prime);
			ImGui::SingleSelectSkeet("Musickit", &configs.profileChanger.musickit, music_kits);
			ImGui::Text("StatTrak");
			ImGui::InputInt("##STATTRAK__INPUT", &configs.profileChanger.musickit_stattrak);
			configs.profileChanger.musickit_stattrak = std::clamp(configs.profileChanger.musickit_stattrak, -1, configs.profileChanger.musickit_stattrak);
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size_p.y + 40));
		ImGui::BeginChildSkeet("PLAYER LIST", child_size_p);
		{
			ImGui::Columns(2, nullptr, false);
			static std::vector<Player_list_data> players;

			if (!configs.player_list.refreshing)
			{
				players.clear();

				for (auto& player : configs.player_list.players)
					players.emplace_back(player);
			}

			static auto current_player = 0;

			if (!players.empty())
			{
				std::vector<std::string> player_names;

				for (auto& player : players)
					player_names.emplace_back(player.name);

				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);
				ImGui::PushFont(unicodeFont.fonts.chinese10);
				ImGui::ListBoxConfigArraySkeet(crypt_str("##PLAYERLIST"), &current_player, player_names, 10);
				ImGui::PopFont();
				ImGui::PopStyleVar();
			}

			ImGui::NextColumn();

			if (!players.empty())
			{
				if (current_player >= players.size())
					current_player = players.size() - 1;
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 16);
				ImGui::CheckboxSkeet(crypt_str("White list"), &configs.player_list.white_list[players.at(current_player).i]);

				if (!configs.player_list.white_list[players.at(current_player).i])
				{
					ImGui::CheckboxSkeet(crypt_str("High priority"), &configs.player_list.high_priority[players.at(current_player).i]);
					ImGui::CheckboxSkeet(crypt_str("Roll prediction"), &configs.player_list.roll_prediction[players.at(current_player).i]);
					ImGui::CheckboxSkeet(crypt_str("Force safe points"), &configs.player_list.force_safe_points[players.at(current_player).i]);
					ImGui::CheckboxSkeet(crypt_str("Force body aim"), &configs.player_list.force_body_aim[players.at(current_player).i]);
					ImGui::CheckboxSkeet(crypt_str("Disable resolver"), &configs.player_list.disable_resolver[players.at(current_player).i]);
				}
			}
		}

		ImGui::EndChildSkeet();

		ImGui::SetNextWindowPos(ImVec2(ImGui::GetWindowPos().x + 120, ImGui::GetWindowPos().y + child_size_p.y * 2 + 80 - 20));

		ImGui::BeginChildSkeet((cheat.cheatname + " Info").c_str(), child_size_a);
		{
			ImGui::Text((crypt_str("Username: ") + cheat.username).c_str());
			ImGui::Text((crypt_str("Remaining time: ") + (cheat.remainingTime != -1 ? (std::to_string(cheat.remainingTime) + " days") : "Lifetime")).c_str());
			ImGui::Text((crypt_str("Edition: ") + cheat.edition).c_str());
			ImGui::Text((crypt_str("Developer: ") + cheat.developer).c_str());
			ImGui::Text((crypt_str("Version: ") + cheat.getVersion()).c_str());
		}
		ImGui::EndChildSkeet();
	}

	ImGui::PopFont();
	ImGui::EndSkeet();

	return; // Ignore it, I think this is bad lmao. But I don't want to delete it

	if (this->m_nCurrentTab != 3)
		return;

	if (!g_DrawModel->GetTexture())
		return;

	ImGui::SetNextWindowSize(ImVec2(350, 445));
	ImGui::SetNextWindowPos(ImVec2(vecWindowPosition.x + 654 + 150, vecWindowPosition.y));
	ImGui::BeginSkeet("DrawModelTab", 0, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
	if (g_DrawModel->GetTexture())
	{
		ImGui::GetDrawList()->AddImage(
			g_DrawModel->GetTexture()->pTextureHandles[0]->lpRawTexture,
			ImVec2(vecWindowPosition.x + 610 + 150, vecWindowPosition.y - 130),
			ImVec2(vecWindowPosition.x + 610 + 150 + g_DrawModel->GetTexture()->GetActualWidth(), vecWindowPosition.y + g_DrawModel->GetTexture()->GetActualHeight() - 130),
			ImVec2(0, 0), ImVec2(1, 1),
			ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));

		Color aBox = Color(configs.player.type[0].box_color);
		if (configs.player.type[0].box)
		{
			ImGui::GetDrawList()->AddRect(ImVec2(vecWindowPosition.x + 694 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 886 + 170, vecWindowPosition.y + 386), ImColor(0, 0, 0, 255));
			ImGui::GetDrawList()->AddRect(ImVec2(vecWindowPosition.x + 696 + 150, vecWindowPosition.y + 41), ImVec2(vecWindowPosition.x + 884 + 170, vecWindowPosition.y + 384), ImColor(0, 0, 0, 255));
			ImGui::GetDrawList()->AddRect(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 885 + 170, vecWindowPosition.y + 385), ImColor(aBox.r(), aBox.g(), aBox.b(), aBox.a()));
		}
		Color aName = Color(configs.player.type[0].name_color);
		if (configs.player.type[0].name)
		{
			RenderText("Sakura", ImVec2(vecWindowPosition.x + 715 + 150 + 80 - 50 / 2, vecWindowPosition.y + 22), 15.f, aName, false, false, skeet_menu.f2);
		}
		Color aHealthBar = Color(configs.player.type[0].health_color);
		if (configs.player.type[0].health)
		{
			ImGui::GetDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 689 + 150, vecWindowPosition.y + 39), ImVec2(vecWindowPosition.x + 693 + 150, vecWindowPosition.y + 385), ImColor(0, 0, 0, 100));
			ImGui::GetDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 690 + 150, vecWindowPosition.y + 40), ImVec2(vecWindowPosition.x + 692 + 150, vecWindowPosition.y + 385), ImColor(aHealthBar.r(), aHealthBar.g(), aHealthBar.b(), aHealthBar.a()));
		}
		Color aAmmoBar = Color(configs.player.type[0].ammobar_color);
		if (configs.player.type[0].ammo)
		{
			ImGui::GetDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 387), ImVec2(vecWindowPosition.x + 906 + 150, vecWindowPosition.y + 391), ImColor(0, 0, 0, 100));
			ImGui::GetDrawList()->AddRectFilled(ImVec2(vecWindowPosition.x + 695 + 150, vecWindowPosition.y + 388), ImVec2(vecWindowPosition.x + 905 + 150, vecWindowPosition.y + 390), ImColor(aAmmoBar.r(), aAmmoBar.g(), aAmmoBar.b(), aAmmoBar.a()));
		}
	}
	ImGui::EndSkeet();
}

ImColor menu::get_theme_color(float alpha) {
	auto color = configs.menu.menu_theme.toImColor();
	color.Value.w = alpha;
	return color;
}

LPDIRECT3DTEXTURE9 menu::steam_image()
{
	return nullptr;
}

std::array<Vector4D, 6> aWhiteArray =
{
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
	Vector4D(0.4f, 0.4f, 0.4f, 1.0f),
};

__forceinline void AngleMatrix(const QAngle& angView, matrix3x4_t& matOutput, const Vector& vecOrigin)
{
	float sp, sy, sr, cp, cy, cr;

	DirectX::XMScalarSinCos(&sp, &cp, DEG2RAD(angView.pitch));
	DirectX::XMScalarSinCos(&sy, &cy, DEG2RAD(angView.yaw));
	DirectX::XMScalarSinCos(&sr, &cr, DEG2RAD(angView.roll));

	matOutput.SetForward(Vector(cp * cy, cp * sy, -sp));

	const float crcy = cr * cy;
	const float crsy = cr * sy;
	const float srcy = sr * cy;
	const float srsy = sr * sy;

	matOutput.SetLeft(Vector(sp * srcy - crsy, sp * srsy + crcy, sr * cp));
	matOutput.SetUp(Vector(sp * crcy + srsy, sp * crsy - srcy, cr * cp));
	matOutput.SetOrigin(vecOrigin);
}

NEWMaterial* fCreateMaterial(bool lit, const std::string& material_data)
{
	static auto created = 0;
	std::string type = lit ? crypt_str("VertexLitGeneric") : crypt_str("UnlitGeneric");

	auto matname = cheat.cheatnamelower + crypt_str("_") + std::to_string(created);
	++created;

	auto material = m_materialsystem2()->CreateMaterial(matname.c_str(), KeyValues::FromString(type.c_str(), material_data.c_str()));
	material->IncrementReferenceCount();

	return material;
}

void ApplyMaterial(int32_t iMaterial, bool bIgnoreZ, Color aColor, bool bCustom, bool bShotChams)
{
	NEWMaterial* pRenderMaterial = nullptr;

	static NEWMaterial* materials[] =
	{
		fCreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"0"
				"$wireframe"				"0"
			}
		)#")),
		fCreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{
				"$basetexture"				"vgui/white" 
				"$ignorez"					"0" 
				"$envmap"					"env_cubemap" 
				"$normalmapalphaenvmapmask" "1" 
				"$envmapcontrast"			"1" 
				"$nofog"					"1" 
				"$model"					"1" 
				"$nocull" 					"0" 
				"$selfillum" 				"1" 
				"$halflambert"				"1" 
				"$znearer" 					"0" 
				"$flat" 					"1"
		        "$wireframe"				"0"
			}
		)#")),
		fCreateMaterial(false, crypt_str(R"#("UnlitGeneric"
			{
				"$basetexture"				"vgui/white"
				"$ignorez"					"0"
				"$envmap"					" "
				"$nofog"					"1"
				"$model"					"1"
				"$nocull"					"0"
				"$selfillum"				"1"
				"$halflambert"				"1"
				"$znearer"					"0"
				"$flat"						"1"
				"$wireframe"				"0"
			}
		)#")),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/dogtags/dogtags_outline"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/trophy_majors/crystal_clear"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/cologne_prediction/cologne_prediction_glass"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("dev/glow_armsrace.vmt"), nullptr),
		m_materialsystem2()->FindMaterial(crypt_str("models/inventory_items/wildfire_gold/wildfire_gold_detail"), nullptr),
		fCreateMaterial(true, crypt_str(R"#("VertexLitGeneric" 
			{ 
				"$additive"					"1" 
				"$envmap"					"models/effects/cube_white" 
				"$envmaptint"				"[1 1 1]" 
				"$envmapfresnel"			"1" 
				"$envmapfresnelminmaxexp" 	"[0 1 2]" 
				"$alpha" 					"0.8" 
			}
		)#")),
		fCreateMaterial(true, crypt_str(R"#("VertexLitGeneric"
		    {
		        "$basetexture"				"dev/zone_warning"
		        "$additive"					"1"
		        "$envmap"					"editor/cube_vertigo"
		        "$envmaptint"				"[0 0.5 0.55]"
		        "$envmapfresnel"			"1"
		        "$envmapfresnelminmaxexp"   "[0.00005 0.6 6]"
		        "$alpha"					"1"
   
		        Proxies
		        {
		            TextureScroll
		            {
		                "texturescrollvar"			"$baseTextureTransform"
		                "texturescrollrate"			"0.25"
		                "texturescrollangle"		"270"
		            }
		            Sine
		            {
		                "sineperiod"				"2"
		                "sinemin"					"0.1"
		                "resultVar"					"$envmapfresnelminmaxexp[1]"
		            }
		        }
		    }
		)#"))
	};

	pRenderMaterial = materials[iMaterial];

	bool bHasBeenFound = false;
	if (!pRenderMaterial)
		return;

	NEWMaterialVar* pMaterialVar = pRenderMaterial->FindVar(crypt_str("$envmaptint"), &bHasBeenFound);
	if (!pMaterialVar)
		return;

	if (bHasBeenFound && iMaterial > 1)
		pMaterialVar->SetVectorValue(aColor.r() / 255.0f, aColor.g() / 255.0f, aColor.b() / 255.0f);

	const float aBlendColor[3] =
	{
		(float_t)(aColor.r()) / 255.0f,
		(float_t)(aColor.g()) / 255.0f,
		(float_t)(aColor.b()) / 255.0f
	};

	pRenderMaterial->SetMaterialVarFlag(MATERIAL_VAR_IGNOREZ, bIgnoreZ && !bCustom);
	if (bCustom)
	{
		pRenderMaterial->ColorModulate(aColor.r() / 255.0f, aColor.g() / 255.0f, aColor.b() / 255.0f);
		pRenderMaterial->AlphaModulate(aColor.a() / 255.0f);

		return m_studiorender()->ForcedMaterialOverride(pRenderMaterial);
	}

	pRenderMaterial->ColorModulate(1.0f, 1.0f, 1.0f);
	pRenderMaterial->AlphaModulate(1.0f);

	if (bShotChams)
	{
		m_renderview()->SetColorModulation(aBlendColor);
		m_renderview()->SetBlend(aColor.a() / 255.0f);
		return m_modelrender()->ForcedMaterialOverride(pRenderMaterial);
	}

	m_renderview()->SetColorModulation(aBlendColor);
	m_renderview()->SetBlend(aColor.a() / 255.0f);
	return m_modelrender()->ForcedMaterialOverride(pRenderMaterial);
}

void C_DrawModel::Instance()
{
	if (!Hooks::menu_open)
		return;

	if (!m_PreviewTexture)
	{
		m_materialsystem()->BeginRenderTargetAllocation();

		m_PreviewTexture = m_materialsystem()->CreateNamedRenderTargetTextureEx(
			crypt_str("Preview"),
			350, 575,
			RT_SIZE_NO_CHANGE,
			m_materialsystem()->GetBackBufferFormat(),
			MATERIAL_RT_DEPTH_SHARED,
			TEXTUREFLAGS_CLAMPS | TEXTUREFLAGS_CLAMPT,
			0x00000001
		);

		m_materialsystem()->FinishRenderTargetAllocation();
	}

	if (!m_CubemapTexture)
		m_CubemapTexture = m_materialsystem()->FindTexture(crypt_str("editor/cubemap.hdr"), TEXTURE_GROUP_CUBE_MAP);
	static auto m_CreateModel = util::FindSignature("client.dll", crypt_str("53 8B D9 56 57 8D 4B 04 C7 03 ? ? ? ? E8 ? ? ? ? 6A"));
	auto CreateModel = reinterpret_cast<void(__thiscall*)(void*)>(m_CreateModel);
	if (!m_PreviewModel)
	{
		m_PreviewModel = static_cast<C_MergedMDL*>(m_memalloc()->Alloc(0x75C));
		CreateModel(m_PreviewModel);
		m_PreviewModel->SetMDL(crypt_str("models/player/custom_player/uiplayer/animset_uiplayer_buymenu.mdl"));
		m_PreviewModel->SetMergedMDL(crypt_str("models/player/custom_player/legacy/ctm_fbi_variantb.mdl"));
		m_PreviewModel->SetMergedMDL(crypt_str("models/weapons/w_pist_deagle.mdl"));
		m_PreviewModel->SetSequence(32, false);
		m_PreviewModel->SetupBonesForAttachmentQueries();
	}

	m_PreviewModel->RootMDL.flTime += m_globals()->m_frametime / 2.0f;

	m_ViewSetup.iX = 0;
	m_ViewSetup.iY = 0;
	m_ViewSetup.iWidth = 350;
	m_ViewSetup.iHeight = 575;
	m_ViewSetup.bOrtho = false;
	m_ViewSetup.flFOV = 70.f;
	m_ViewSetup.vecOrigin = Vector(-65.0f, 2.0f, 50);
	m_ViewSetup.angView = QAngle(0, 0, 0);
	m_ViewSetup.flNearZ = 7.0f;
	m_ViewSetup.flFarZ = 1000.f;
	m_ViewSetup.bDoBloomAndToneMapping = true;

	CMatRenderContextPtr pRenderContext(m_materialsystem());

	pRenderContext->PushRenderTargetAndViewport();
	pRenderContext->SetRenderTarget(m_PreviewTexture);

	pRenderContext->BindLocalCubemap(m_CubemapTexture);
	pRenderContext->SetLightingOrigin(-65.0f, 2.0f, 50.0f);

	Frustum dummyFrustum;

	enum
	{
		VIEW_CLEAR_COLOR = 0x1,
		VIEW_CLEAR_DEPTH = 0x2,
		VIEW_CLEAR_FULL_TARGET = 0x4,
		VIEW_NO_DRAW = 0x8,
		VIEW_CLEAR_OBEY_STENCIL = 0x10,
		VIEW_CLEAR_STENCIL = 0x20,
	};

	m_renderview()->Push3DView(pRenderContext, m_ViewSetup, VIEW_CLEAR_COLOR | VIEW_CLEAR_DEPTH | VIEW_CLEAR_STENCIL, m_PreviewTexture, dummyFrustum);

	pRenderContext->ClearColor4ub(false, false, false, false);
	pRenderContext->ClearBuffers(true, true, true);
	pRenderContext->SetAmbientLightCube(aWhiteArray.data());

	m_studiorender()->SetAmbientLightColors(aWhiteArray.data());
	m_studiorender()->SetLocalLights(0, nullptr);

	matrix3x4_t matPlayerView = { };
	AngleMatrix(QAngle(0, -180, 0), matPlayerView, Vector(0, 0, 0));

	auto fix = configs.player.type[0].chams_type == 3 || configs.player.type[0].chams_type == 5 || configs.player.type[0].chams_type == 6 || configs.player.type[0].chams_type == 8;

	m_modelrender()->SuppressEngineLighting(true);
	if (configs.player.type[0].chams[PLAYER_CHAMS_VISIBLE] && !fix && !configs.player.type[0].double_material && !configs.player.type[0].animated_material)
		ApplyMaterial(configs.player.type[0].chams_type, false, Color(configs.player.type[0].chams_color), true, false);
	else if (configs.player.type[0].chams[PLAYER_CHAMS_VISIBLE] && (fix || configs.player.type[0].double_material || configs.player.type[0].animated_material))
		ApplyMaterial(0, false, Color(configs.player.type[0].chams_color.r(), configs.player.type[0].chams_color.g(), configs.player.type[0].chams_color.b(), configs.player.type[0].chams_color.a() - 1), true, false);

	m_PreviewModel->Draw(matPlayerView);

	if (fix)
	{
		ApplyMaterial(configs.player.type[0].chams_type, true, Color(configs.player.type[0].chams_color), true, true);
		m_PreviewModel->Draw(matPlayerView);
	}

	if (configs.player.type[0].double_material)
	{
		ApplyMaterial(6, true, Color(configs.player.type[0].double_material_color), true, true);
		m_PreviewModel->Draw(matPlayerView);
	}

	if (configs.player.type[0].animated_material)
	{
		ApplyMaterial(9, true, Color(configs.player.type[0].animated_material_color), true, true);
		m_PreviewModel->Draw(matPlayerView);
	}

	m_modelrender()->SuppressEngineLighting(false);

	m_renderview()->PopView(pRenderContext, dummyFrustum);
	pRenderContext->BindLocalCubemap(nullptr);

	pRenderContext->PopRenderTargetAndViewport();
	pRenderContext->Release();
}

void C_MergedMDL::SetupBonesForAttachmentQueries()
{
	static auto m_SetupBonesForAttachmentQueries = util::FindSignature("client.dll", crypt_str("55 8B EC 83 EC 14 83 3D ? ? ? ? ? 53"));
	return ((void(__thiscall*)(void*))(m_SetupBonesForAttachmentQueries))(this);
}

void C_MergedMDL::SetMergedMDL(const char* szModelPath, CCustomMaterialOwner* pCustomMaterialOwner, void* pProxyData)
{
	static auto m_SetMergedMDL = util::FindSignature("client.dll", crypt_str("55 8B EC 57 8B F9 8B 0D ? ? ? ? 85 C9 75"));
	return ((void(__thiscall*)(void*, const char*, CCustomMaterialOwner*, void*, bool))(m_SetMergedMDL))(this, szModelPath, pCustomMaterialOwner, pProxyData, false);
}
