#include "HTTPRequest.hpp"
#include "Clua.h"	
#include <ShlObj_core.h>
#include <Windows.h>
#include <any>
#include "..\features\visuals\other_esp.h"
#include "..\utils\csgo.hpp"
#include "..\features\misc\logs.h"
#include "..\includes.hpp"
#include "..\features\menu.h"

void lua_panic(sol::optional<std::string> message)
{
	if (!message)
		return;

	auto log = crypt_str("Lua error: ") + message.value_or(crypt_str("unknown"));
	eventlogs::get().add(crypt_str("ERROR"), log, false);
}

std::string get_current_script(sol::this_state s)
{
	sol::state_view lua_state(s);
	sol::table rs = lua_state["debug"]["getinfo"](2, crypt_str("S"));
	std::string source = rs["source"];
	std::string filename = std::filesystem::path(source.substr(1)).filename().string();

	return filename;
}

int get_current_script_id(sol::this_state s)
{
	return c_lua::get().get_script_id(get_current_script(s));
}

namespace ns_cheat
{
	void register_callback(sol::this_state s, std::string eventname, sol::protected_function func)
	{
		if (eventname != crypt_str("on_paint") && eventname != crypt_str("on_draw") && eventname != crypt_str("on_resolver") && eventname != crypt_str("on_preprediction") && eventname != crypt_str("on_prediction") && eventname != crypt_str("on_restoreprediction") && eventname != crypt_str("on_createmove") && eventname != crypt_str("on_framestage") && eventname != crypt_str("on_shot"))
		{
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: invalid callback \"") + eventname + '\"', false);
			c_lua::get().unload_script(get_current_script_id(s));
			return;
		}

		if (c_lua::get().loaded.at(get_current_script_id(s)))
			c_lua::get().hooks.registerHook(eventname, get_current_script_id(s), func);
	}

	void load_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().load_script(c_lua::get().get_script_id(name));
	}

	void unload_script(std::string name)
	{
		c_lua::get().refresh_scripts();
		c_lua::get().unload_script(c_lua::get().get_script_id(name));
	}

	void log(std::string text)
	{
		eventlogs::get().add(text, false);
	}

	bool is_chat_open()
	{
		return util::isChatOpen();
	}

	bool should_breaking_lby(CUserCmd* cmd)
	{
		return antiaim::get().should_break_lby(cmd);
	}

	bool is_breaking_lby()
	{
		return antiaim::get().breaking_lby;
	}

	void set_breaking_lby(bool breaking)
	{
		antiaim::get().breaking_lby = breaking;
	}

	std::string get_username()
	{
		return cheat.username;
	}

	std::string get_edition()
	{
		return cheat.edition;
	}
}

std::vector<std::pair<std::string, menu_item>>::iterator find_item(std::vector<std::pair<std::string, menu_item>>& items, const std::string& name)
{
	for (auto it = items.begin(); it != items.end(); ++it)
		if (it->first == name)
			return it;

	return items.end();
}

menu_item find_item(std::vector<std::vector<std::pair<std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
	{
		for (auto& item : script)
		{
			std::string item_name;

			auto first_point = false;
			auto second_point = false;

			for (auto& c : item.first)
			{
				if (c == '.')
				{
					if (first_point)
					{
						second_point = true;
						continue;
					}
					else
					{
						first_point = true;
						continue;
					}
				}

				if (!second_point)
					continue;

				item_name.push_back(c);
			}

			if (item_name == name)
				return item.second;
		}
	}

	return menu_item();
}

menu_item find_item_full_name(std::vector<std::vector<std::pair<std::string, menu_item>>>& scripts, const std::string& name)
{
	for (auto& script : scripts)
		for (auto& item : script)
			if (item.first == name)
				return item.second;

	return menu_item();
}

namespace ns_menu
{
	bool get_visible()
	{
		return Hooks::menu_open;
	}

	void set_visible(bool visible)
	{
		Hooks::menu_open = visible;
	}

	auto next_line_counter = 0;

	void next_line(sol::this_state s)
	{
		c_lua::get().items.at(get_current_script_id(s)).emplace_back(std::make_pair(crypt_str("next_line_") + std::to_string(next_line_counter), menu_item()));
		++next_line_counter;
	}

	std::string add_check_box(sol::this_state s, const std::string& name, bool value = false)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return crypt_str("");

		items.emplace_back(std::make_pair(full_name, menu_item(value)));
		return full_name;
	}

	std::string add_combo_box(sol::this_state s, std::string name, std::vector<std::string> labels, int value = 0)
	{
		if (labels.empty())
			return "";

		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return crypt_str("");

		items.emplace_back(std::make_pair(full_name, menu_item(labels, value)));
		return full_name;
	}

	std::string add_slider_int(sol::this_state s, const std::string& name, int min, int max, int value = 0)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return crypt_str("");

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, value)));
		return full_name;
	}

	std::string add_slider_float(sol::this_state s, const std::string& name, float min, float max, float value = 0.0f)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return crypt_str("");

		items.emplace_back(std::make_pair(full_name, menu_item(min, max, value)));
		return full_name;
	}

	std::string add_color_picker(sol::this_state s, const std::string& name, Color value = Color::White)
	{
		auto script = get_current_script(s);
		auto script_id = c_lua::get().get_script_id(script);

		auto& items = c_lua::get().items.at(script_id);
		auto full_name = script + '.' + name;

		if (find_item(items, full_name) != items.end())
			return crypt_str("");

		items.emplace_back(std::make_pair(full_name, menu_item(value)));
		return full_name;
	}

	static std::unordered_map<std::string, bool> first_update;
	static std::unordered_map<std::string, menu_item> stored_values;
	static std::unordered_map<std::string, void*> config_items;

	bool find_config_item(std::string name, std::string type)
	{
		if (config_items.find(name) == config_items.end())
		{
			auto found = false;

			for (auto item : cfg_manager->items)
			{
				if (item->name == name)
				{
					if (item->type != type)
					{
						eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: invalid config item type, must be ") + type, false);
						return false;
					}

					found = true;
					config_items[name] = item->pointer;
					break;
				}
			}

			if (!found)
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find config variable \"") + name + '\"', false);
				return false;
			}
		}

		return true;
	}

	bool get_bool(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].check_box_value;
			else if (config_items.find(name) != config_items.end())
				return *(bool*)config_items[name];
			else
				return false;
		}

		auto& it = find_item_full_name(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("bool")))
				return *(bool*)config_items[name];

			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return false;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.check_box_value;
	}

	int get_int(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].type == COMBO_BOX ? stored_values[name].combo_box_value : stored_values[name].slider_int_value;
			else if (config_items.find(name) != config_items.end())
				return *(int*)config_items[name];
			else
				return 0;
		}

		auto& it = find_item_full_name(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("int")))
				return *(int*)config_items[name];

			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.type == COMBO_BOX ? it.combo_box_value : it.slider_int_value;
	}

	float get_float(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].slider_float_value;
			else if (config_items.find(name) != config_items.end())
				return *(float*)config_items[name];
			else
				return 0.0f;
		}

		auto& it = find_item_full_name(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("float")))
				return *(float*)config_items[name];

			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return 0.0f;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.slider_float_value;
	}

	Color get_color(std::string name)
	{
		if (first_update.find(name) == first_update.end())
			first_update[name] = false;

		if (!Hooks::menu_open && first_update[name])
		{
			if (stored_values.find(name) != stored_values.end())
				return stored_values[name].color_picker_value;
			else if (config_items.find(name) != config_items.end())
				return *(Color*)config_items[name];
			else
				return Color::White;
		}

		auto& it = find_item_full_name(c_lua::get().items, name);

		if (it.type == NEXT_LINE)
		{
			if (find_config_item(name, crypt_str("Color")))
				return *(Color*)config_items[name];

			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find menu variable \"") + name + '\"', false);
			return Color::White;
		}

		first_update[name] = true;
		stored_values[name] = it;

		return it.color_picker_value;
	}

	void set_key_bind_state(int key_bind, bool state)
	{
		key_binds::get().set_key_bind_state(key_bind, state);
	}

	bool get_key_bind_state(int key_bind)
	{
		return key_binds::get().get_key_bind_state_lua(key_bind);
	}

	int get_key_bind_mode(int key_bind)
	{
		return key_binds::get().get_key_bind_mode(key_bind);
	}

	void set_bool(std::string name, bool value)
	{
		if (!find_config_item(name, crypt_str("bool")))
			return;

		*reinterpret_cast<bool*>(config_items[name]) = value;
	}

	void set_int(std::string name, int value)
	{
		if (!find_config_item(name, crypt_str("int")))
			return;

		*reinterpret_cast<int*>(config_items[name]) = value;
	}

	void set_float(std::string name, float value)
	{
		if (!find_config_item(name, crypt_str("float")))
			return;

		*reinterpret_cast<float*>(config_items[name]) = value;
	}

	void set_color(std::string name, Color value)
	{
		if (!find_config_item(name, crypt_str("Color")))
			return;

		*reinterpret_cast<Color*>(config_items[name]) = value;
	}
}

namespace ns_globals
{
	int get_framerate()
	{
		return cheat.globals.framerate;
	}

	int get_ping()
	{
		return cheat.globals.ping;
	}

	std::string get_ip()
	{
		if (!m_engine()->IsInGame())
			return "unknown";

		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return "unknown";

		auto server = nci->GetAddress();

		if (!strcmp(server, "loopback"))
			server = "Local server";
		else if (m_gamerules()->m_bIsValveDS())
			server = "Valve server";

		return server;
	}

	std::string get_time()
	{
		return cheat.globals.time;
	}

	float get_realtime()
	{
		return m_globals()->m_realtime;
	}

	float get_curtime()
	{
		return m_globals()->m_curtime;
	}

	float get_frametime()
	{
		return m_globals()->m_frametime;
	}

	int get_tickcount()
	{
		return m_globals()->m_tickcount;
	}

	int get_framecount()
	{
		return m_globals()->m_framecount;
	}

	float get_intervalpertick()
	{
		return m_globals()->m_intervalpertick;
	}

	int get_maxclients()
	{
		return m_globals()->m_maxclients;
	}
}

namespace ns_engine
{
	int get_screen_width()
	{
		return m_surface()->GetScreenSize2D().x;
	}

	int get_screen_height()
	{
		return m_surface()->GetScreenSize2D().y;
	}

	int get_mouse_x()
	{
		return m_surface()->GetMousePosition().x;
	}

	int get_mouse_y()
	{
		return m_surface()->GetMousePosition().y;
	}

	bool mouse_in_region(int x, int y, int width, int height)
	{
		return m_surface()->MouseInRegion(x, y, width, height);
	}

	player_info_t get_player_info(int i)
	{
		player_info_t player_info;
		m_engine()->GetPlayerInfo(i, &player_info);

		return player_info;
	}

	int get_player_for_user_id(int i)
	{
		return m_engine()->GetPlayerForUserID(i);
	}

	int get_local_player_index()
	{
		return m_engine()->GetLocalPlayer();
	}

	Vector get_view_angles()
	{
		Vector view_angles;
		m_engine()->GetViewAngles(view_angles);

		return view_angles;
	}

	void set_view_angles(Vector view_angles)
	{
		math::normalize_angles(view_angles);
		m_engine()->SetViewAngles(view_angles);
	}

	bool is_in_game()
	{
		return m_engine()->IsInGame();
	}

	bool is_connected()
	{
		return m_engine()->IsConnected();
	}

	std::string get_level_name()
	{
		return m_engine()->GetLevelName();
	}

	std::string get_level_name_short()
	{
		return m_engine()->GetLevelNameShort();
	}

	std::string get_map_group_name()
	{
		return m_engine()->GetMapGroupName();
	}

	bool is_playing_demo()
	{
		return m_engine()->IsPlayingDemo();
	}

	bool is_recording_demo()
	{
		return m_engine()->IsRecordingDemo();
	}

	bool is_paused()
	{
		return m_engine()->IsPaused();
	}

	bool is_taking_screenshot()
	{
		return m_engine()->IsTakingScreenshot();
	}

	bool is_hltv()
	{
		return m_engine()->IsHLTV();
	}
}

/*
namespace ns_render
{
	Vector world_to_screen(const Vector& source)
	{
		return math::world_to_screen(source);
	}

	int get_text_width(ImFont* font, const std::string& text)
	{
		return imRender->GetTextSize(font, text).x;
	}

	IDirect3DTexture9* create_image(const std::string& data)
	{
		IDirect3DTexture9* image = nullptr;
		D3DXCreateTextureFromFileInMemory(c_menu::get().device, data.c_str(), sizeof(data.c_str()), &image);
		return image;
	}

	void draw_image(IDirect3DTexture9* image, float x, float y, float width, float height)
	{
		ImGui::GetBackgroundDrawList()->AddImage(image, ImVec2(x, y), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

	// float weight, std::optional<bool> antialias, 
	FontInfo create_font(const std::string& name, float size, std::optional<bool> dropshadow, std::optional<bool> outline)
	{
		return FontInfo{ ImGui::GetIO().Fonts->AddFontFromFileTTF(name.c_str(), size), outline.value_or(false) };
	}

	void draw_text(FontInfo fontInfo, float x, float y, Color color, const std::string& text)
	{
		imRender->DrawString(x, y, color, fontInfo.outline ? IFONT_OUTLINE : IFONT_NONE, fontInfo.font, text);
	}

	void draw_text_centered(FontInfo fontInfo, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text)
	{
		DWORD centered_flags = IFONT_NONE;

		if (centered_x)
		{
			centered_flags &= ~IFONT_NONE;
			centered_flags |= IFONT_CENTERED_X;
		}

		if (centered_y)
		{
			centered_flags &= ~IFONT_NONE;
			centered_flags |= IFONT_CENTERED_Y;
		}

		if (fontInfo.outline)
		{
			centered_flags &= ~IFONT_NONE;
			centered_flags |= IFONT_OUTLINE;
		}

		imRender->DrawString(x, y, color, centered_flags, fontInfo.font, text);
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		imRender->DrawLine(x, y, x2, y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		imRender->Rect(x, y, w, h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		imRender->FilledRect(x, y, w, h, color);
	}

	//void draw_rect_filled_gradient(float x, float y, float w, float h, Color color, Color color2, int gradient_type)
	//{
	//	gradient_type = math::clamp(gradient_type, 0, 1);
	//	render::get().gradient((int)x, (int)y, (int)w, (int)h, color, color2, (GradientType)gradient_type);
	//}

	//float points, 
	void draw_circle(float x, float y, float radius, Color color)
	{
		imRender->Circle(x, y, radius, color, 50);
	}

	void draw_circle_filled(float x, float y, float points, float radius, Color color)
	{
		imRender->CircleFilled(x, y, radius, color, 50);
	}

	void draw_triangle(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		imRender->Triangle(x, y, x2, y2, x3, y3, color);
	}

	void draw_triangle_filled(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		imRender->TriangleFilled(x, y, x2, y2, x3, y3, color);
	}
}
*/

namespace ns_render
{
	Vector world_to_screen(const Vector& source)
	{
		Vector screen;

		if (!math::world_to_screen_old(source, screen))
			return ZERO;

		return screen;
	}

	int get_text_width(vgui::HFont font, const std::string& text)
	{
		return render::get().text_width(font, text.c_str());
	}

	IDirect3DTexture9* create_image(const std::string& data)
	{
		IDirect3DTexture9* image = nullptr;
		D3DXCreateTextureFromFileInMemory(c_menu::get().device, data.c_str(), sizeof(data.c_str()), &image);
		return image;
	}

	void draw_image(IDirect3DTexture9* image, float x, float y, float width, float height)
	{
		ImGui::GetBackgroundDrawList()->AddImage(image, ImVec2(x, y), ImVec2(width, height), ImVec2(0, 0), ImVec2(1, 1), ImGui::GetColorU32(ImVec4(1, 1, 1, 1)));
	}

	vgui::HFont create_font(const std::string& name, float size, float weight, std::optional <bool> antialias, std::optional <bool> dropshadow, std::optional <bool> outline)
	{
		DWORD flags = FONTFLAG_NONE;

		if (antialias.value_or(false))
			flags |= FONTFLAG_ANTIALIAS;

		if (dropshadow.value_or(false))
			flags |= FONTFLAG_DROPSHADOW;

		if (outline.value_or(false))
			flags |= FONTFLAG_OUTLINE;

		cheat.last_font_name = name;

		auto font = m_surface()->FontCreate();
		m_surface()->SetFontGlyphSet(font, name.c_str(), (int)size, (int)weight, 0, 0, flags);

		return font;
	}

	void draw_text(vgui::HFont font, float x, float y, Color color, const std::string& text)
	{
		render::get().text(font, (int)x, (int)y, color, HFONT_CENTERED_NONE, text.c_str());
	}

	void draw_text_centered(vgui::HFont font, float x, float y, Color color, bool centered_x, bool centered_y, const std::string& text)
	{
		DWORD centered_flags = HFONT_CENTERED_NONE;

		if (centered_x)
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_X;
		}

		if (centered_y)
		{
			centered_flags &= ~HFONT_CENTERED_NONE;
			centered_flags |= HFONT_CENTERED_Y;
		}

		render::get().text(font, (int)x, (int)y, color, centered_flags, text.c_str());
	}

	void draw_line(float x, float y, float x2, float y2, Color color)
	{
		render::get().line(x, y, x2, y2, color);
	}

	void draw_rect(float x, float y, float w, float h, Color color)
	{
		render::get().rect(x, y, w, h, color);
	}

	void draw_rect_filled(float x, float y, float w, float h, Color color)
	{
		render::get().rect_filled(x, y, w, h, color);
	}

	void draw_rect_filled_gradient(float x, float y, float w, float h, Color color, Color color2, int gradient_type)
	{
		gradient_type = math::clamp(gradient_type, 0, 1);
		render::get().gradient((int)x, (int)y, (int)w, (int)h, color, color2, (GradientType)gradient_type);
	}

	void draw_circle(float x, float y, float points, float radius, Color color)
	{
		render::get().circle((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_circle_filled(float x, float y, float points, float radius, Color color)
	{
		render::get().circle_filled((int)x, (int)y, (int)points, (int)radius, color);
	}

	void draw_triangle(float x, float y, float x2, float y2, float x3, float y3, Color color)
	{
		render::get().triangle(Vector2D(x, y), Vector2D(x2, y2), Vector2D(x3, y3), color);
	}
}

namespace ns_indicator
{
	void add_indicator(std::string& text, Color color)
	{
		if (!cheat.available() || !cheat.local()->is_alive())
			return;

		otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
	}

	void add_indicator_with_pos(std::string& text, Color color, int pos)
	{
		if (!cheat.available() || !cheat.local()->is_alive())
			return;

		if (otheresp::get().m_indicators.empty())
			otheresp::get().m_indicators.insert(otheresp::get().m_indicators.begin(), m_indicator(text, color));
		else
		{
			auto iter1 = otheresp::get().m_indicators.cbegin();
			otheresp::get().m_indicators.insert(iter1 + (pos - 1), m_indicator(text, color));
		}
	}
}

namespace ns_console
{
	void execute(std::string& command)
	{
		if (command.empty())
			return;

		m_engine()->ExecuteClientCmd(command.c_str());
	}

	bool is_visible()
	{
		return m_engine()->Con_IsVisible();
	}

	std::unordered_map<std::string, ConVar*> convars;

	bool get_bool(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return false;
			}
		}

		if (!convars[convar_name])
			return false;

		return convars[convar_name]->GetBool();
	}

	int get_int(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0;
			}
		}

		if (!convars[convar_name])
			return 0;

		return convars[convar_name]->GetInt();
	}

	float get_float(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return 0.0f;
			}
		}

		if (!convars[convar_name])
			return 0.0f;

		return convars[convar_name]->GetFloat();
	}

	std::string get_string(const std::string& convar_name)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return crypt_str("");
			}
		}

		if (!convars[convar_name])
			return "";

		return convars[convar_name]->GetString();
	}

	void set_bool(const std::string& convar_name, bool value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetBool() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_int(const std::string& convar_name, int value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetInt() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_float(const std::string& convar_name, float value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetFloat() != value)
			convars[convar_name]->SetValue(value);
	}

	void set_string(const std::string& convar_name, const std::string& value)
	{
		if (convars.find(convar_name) == convars.end())
		{
			convars[convar_name] = m_cvar()->FindVar(convar_name.c_str());

			if (!convars[convar_name])
			{
				eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: cannot find ConVar \"") + convar_name + '\"', false);
				return;
			}
		}

		if (!convars[convar_name])
			return;

		if (convars[convar_name]->GetString() != value)
			convars[convar_name]->SetValue(value.c_str());
	}
}

namespace ns_events
{
	void register_event(sol::this_state s, std::string event_name, sol::protected_function function)
	{
		if (std::find(cheat.globals.events.begin(), cheat.globals.events.end(), event_name) == cheat.globals.events.end())
		{
			m_eventmanager()->AddListener(&Hooks::hooked_events, event_name.c_str(), false);
			cheat.globals.events.emplace_back(event_name);
		}

		c_lua::get().events[get_current_script_id(s)][event_name] = function;
	}
}

namespace ns_entitylist
{
	sol::optional<player_t*> get_local_player()
	{
		if (!m_engine()->IsInGame())
			return sol::optional<player_t*>(sol::nullopt);

		return reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(m_engine()->GetLocalPlayer()));
	}

	sol::optional<player_t*> get_player_by_index(int i)
	{
		if (!m_engine()->IsInGame())
			return sol::optional<player_t*>(sol::nullopt);

		return reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
	}

	sol::optional<entity_t*> get_entity_by_class_id(int classID)
	{
		if (!m_engine()->IsInGame())
			return sol::optional<entity_t*>(sol::nullopt);

		for (auto i = 1; i < m_globals()->m_maxclients; i++)
		{
			auto e = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

			if (!e)
				continue;

			auto client_class = e->GetClientClass();

			if (!client_class)
				continue;

			if (client_class->m_ClassID == classID)
				return e;
		}

		return sol::optional<entity_t*>(sol::nullopt);
	}

	sol::optional<weapon_t*> get_weapon_by_player(sol::optional<player_t*> player)
	{
		if (!m_engine()->IsInGame())
			return sol::optional<weapon_t*>(sol::nullopt);

		if (!player.value())
			return sol::optional<weapon_t*>(sol::nullopt);

		return player.value()->m_hActiveWeapon().Get();
	}

}

namespace ns_cmd
{
	bool get_send_packet()
	{
		if (!cheat.get_command())
			return true;

		return cheat.send_packet;
	}

	void set_send_packet(bool send_packet)
	{
		if (!cheat.get_command())
			return;

		cheat.send_packet = send_packet;
	}

	int get_choke()
	{
		if (!cheat.get_command())
			return 0;

		return m_clientstate()->m_iChokedCommands;
	}

	bool get_button_state(int button)
	{
		if (!cheat.get_command())
			return false;

		return cheat.get_command()->m_buttons & button;
	}

	void set_button_state(int button, bool state)
	{
		if (!cheat.get_command())
			return;

		if (state)
			cheat.get_command()->m_buttons |= button;
		else
			cheat.get_command()->m_buttons &= ~button;
	}
}

namespace ns_memory
{
	uint64_t find_pattern(const std::string& module, const std::string& signature)
	{
		return util::FindSignature(module.c_str(), signature.c_str());
	}

	void* create_interface(sol::this_state s, const std::string& module, const std::string& name)
	{
		if (const auto createInterface = reinterpret_cast<std::add_pointer_t<void* __cdecl(const char* name, int* returnCode)>>(
			GetProcAddress(GetModuleHandleA(module.c_str()), "CreateInterface")
			)) {
			if (void* foundInterface = createInterface(name.c_str(), nullptr))
				return foundInterface;
		}
		eventlogs::get().add(crypt_str("ERROR"), crypt_str("Lua error: failed to create \"") + name + crypt_str("\" interface!"), false);
		c_lua::get().unload_script(get_current_script_id(s));
	}
}

namespace ns_http
{
	std::string get(sol::this_state s, std::string& link)
	{
		if (!configs.scripts.allow_unsafe_scripts)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("WARNING"), crypt_str("Please allow unsafe scripts to http requests"), false);
			return "";
		}

		try
		{
			http::Request request(link);

			const http::Response response = request.send(crypt_str("GET"));
			return std::string(response.body.begin(), response.body.end());
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}

	std::string post(sol::this_state s, std::string& link, std::string& params)
	{
		if (!configs.scripts.allow_unsafe_scripts)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("WARNING"), crypt_str("Please allow unsafe scripts to http requests"), false);
			return "";
		}

		try
		{
			http::Request request(link);
			const http::Response response = request.send(crypt_str("POST"), params, { crypt_str("Content-Type: application/x-www-form-urlencoded") });
			return std::string(response.body.begin(), response.body.end());
		}
		catch (const std::exception& e)
		{
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Request failed, error: ") + std::string(e.what()), false);
			return "";
		}
	}
}

namespace ns_file
{
	void append(sol::this_state s, std::string& path, std::string& data)
	{
		if (!configs.scripts.allow_unsafe_scripts)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("WARNING"), crypt_str("Please allow unsafe scripts to read or write files"), false);
			return;
		}

		std::ofstream out(path, std::ios::app | std::ios::binary);

		if (out.is_open())
			out << data;
		else
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Can't append to file: ") + path, false);

		out.close();
	}

	void write(sol::this_state s, std::string& path, std::string& data)
	{
		if (!configs.scripts.allow_unsafe_scripts)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("WARNING"), crypt_str("Please allow unsafe scripts to read or write files"), false);
			return;
		}

		std::ofstream out(path, std::ios::binary);

		if (out.is_open())
			out << data;

		else
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Can't write to file: ") + path, false);

		out.close();
	}

	std::string read(sol::this_state s, std::string& path)
	{
		if (!configs.scripts.allow_unsafe_scripts)
		{
			c_lua::get().unload_script(get_current_script_id(s));
			eventlogs::get().add(crypt_str("WARNING"), crypt_str("Please, allow unsafe scripts to read or write files"), false);
			return "";
		}

		std::ifstream file(path, std::ios::binary);

		if (file.is_open())
		{
			std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
			file.close();
			return content;
		}
		else
		{
			eventlogs::get().add(crypt_str("ERROR"), crypt_str("Can't read file: ") + path, false);
			file.close();
			return "";
		}
	}
}

sol::state lua;

void c_lua::initialize()
{
	lua = sol::state(sol::c_call<decltype(&lua_panic), &lua_panic>);
	lua.open_libraries(sol::lib::base, sol::lib::string, sol::lib::math, sol::lib::table, sol::lib::debug, sol::lib::package, sol::lib::bit32, sol::lib::io, sol::lib::os, sol::lib::ffi, sol::lib::jit, sol::lib::utf8);

	lua[crypt_str("collectgarbage")] = sol::nil;
	lua[crypt_str("dofilsse")] = sol::nil;
	lua[crypt_str("load")] = sol::nil;
	lua[crypt_str("loadfile")] = sol::nil;
	lua[crypt_str("pcall")] = sol::nil;
	lua[crypt_str("print")] = sol::nil;
	lua[crypt_str("xpcall")] = sol::nil;
	lua[crypt_str("getmetatable")] = sol::nil;
	lua[crypt_str("setmetatable")] = sol::nil;
	lua[crypt_str("__nil_callback")] = [](){};

	lua.new_enum(crypt_str("key_binds"),
		crypt_str("trigger_bot"), key_bind_features::TRIGGER_BOT,
		crypt_str("legit_bot"), key_bind_features::LEGIT_BOT,
		crypt_str("double_tap"), key_bind_features::DOUBLE_TAP,
		crypt_str("hide_shots"), key_bind_features::HIDE_SHOTS,
		crypt_str("safe_points"), key_bind_features::SAFE_POINT,
		crypt_str("damage_override"), key_bind_features::DAMAGE_OVERRIDE,
		crypt_str("manual_back"), key_bind_features::ANTIAIM_MANUAL_BACK,
		crypt_str("manual_left"), key_bind_features::ANTIAIM_MANUAL_LEFT,
		crypt_str("manual_right"), key_bind_features::ANTIAIM_MANUAL_RIGHT,
		crypt_str("flip_desync"), key_bind_features::ANTIAIM_DESYNC_FLIP,
		crypt_str("zero_pitch"), key_bind_features::ANTIAIM_ZERO_PITCH,
		crypt_str("fake_flick_180"), key_bind_features::ANTIAIM_FAKE_FLICK_180,
		crypt_str("third_person"), key_bind_features::THIRD_PERSON,
		crypt_str("quick_peek"), key_bind_features::AUTO_PEEK,
		crypt_str("edge_jump"), key_bind_features::EDGE_JUMP,
		crypt_str("jump_bug"), key_bind_features::JUMP_BUG,
		crypt_str("long_jump"), key_bind_features::LONG_JUMP,
		crypt_str("mini_jump"), key_bind_features::MINI_JUMP,
		crypt_str("fake_duck"), key_bind_features::FAKE_DUCK,
		crypt_str("slow_motion"), key_bind_features::SLOW_WALK,
		crypt_str("body_aim"), key_bind_features::BODY_AIM,
		crypt_str("resolver_override"), key_bind_features::RESOLVER_OVERRIDE,
		crypt_str("block_bot"), key_bind_features::BLOCK_BOT,
		crypt_str("walk_bot"), key_bind_features::WALK_BOT,
		crypt_str("ping_spike"), key_bind_features::PING_SPIKE
	);

	lua.new_enum(crypt_str("key_bind_mode"),
		crypt_str("hold"), key_bind_mode::HOLD,
		crypt_str("toggle"), key_bind_mode::TOGGLE,
		crypt_str("always"), key_bind_mode::ALWAYS
	);

	lua.new_usertype<entity_t>(crypt_str("entity"),
		(std::string)crypt_str("get_prop_int"), &entity_t::GetPropInt,
		(std::string)crypt_str("get_prop_float"), &entity_t::GetPropFloat,
		(std::string)crypt_str("get_prop_bool"), &entity_t::GetPropBool,
		(std::string)crypt_str("get_prop_string"), &entity_t::GetPropString,
		(std::string)crypt_str("set_prop_int"), &entity_t::SetPropInt,
		(std::string)crypt_str("set_prop_float"), &entity_t::SetPropFloat,
		(std::string)crypt_str("set_prop_bool"), &entity_t::SetPropBool,
		(std::string)crypt_str("get_origin"), &entity_t::GetAbsOrigin,
		(std::string)crypt_str("get_angles"), &entity_t::GetAbsAngles,
		(std::string)crypt_str("get_render_origin"), &entity_t::GetRenderOrigin,
		(std::string)crypt_str("get_render_angles"), &entity_t::GetRenderAngles,
		(std::string)crypt_str("set_origin"), &entity_t::set_abs_origin,
		(std::string)crypt_str("set_angles"), &entity_t::set_abs_angles
	);

	lua.new_usertype<Color>(crypt_str("color"), sol::constructors<Color(), Color(int, int, int), Color(int, int, int, int)>(),
		(std::string)crypt_str("r"), &Color::r,
		(std::string)crypt_str("g"), &Color::g,
		(std::string)crypt_str("b"), &Color::b,
		(std::string)crypt_str("a"), &Color::a,
		(std::string)crypt_str("red"), &Color::r,
		(std::string)crypt_str("green"), &Color::g,
		(std::string)crypt_str("blue"), &Color::b,
		(std::string)crypt_str("alpha"), &Color::a
	);

	lua.new_usertype<Vector>(crypt_str("vector"), sol::constructors<Vector(), Vector(float, float, float)>(),
		(std::string)crypt_str("x"), &Vector::x,
		(std::string)crypt_str("y"), &Vector::y,
		(std::string)crypt_str("z"), &Vector::z,
		(std::string)crypt_str("length"), &Vector::Length,
		(std::string)crypt_str("length_sqr"), &Vector::LengthSqr,
		(std::string)crypt_str("length_2d"), &Vector::Length2D,
		(std::string)crypt_str("length_2d_sqr"), &Vector::Length2DSqr,
		(std::string)crypt_str("is_zero"), &Vector::IsZero,
		(std::string)crypt_str("is_valid"), &Vector::IsValid,
		(std::string)crypt_str("zero"), &Vector::Zero,
		(std::string)crypt_str("dot"), &Vector::Dot,
		(std::string)crypt_str("dist_to"), &Vector::DistTo,
		(std::string)crypt_str("dist_to_sqr"), &Vector::DistToSqr,
		(std::string)crypt_str("cross_product"), &Vector::Cross,
		(std::string)crypt_str("normalize"), &Vector::Normalize
	);

	lua.new_usertype<QAngle>(crypt_str("angle"), sol::constructors<QAngle(), QAngle(float, float, float)>(),
		(std::string)crypt_str("pitch"), &QAngle::pitch,
		(std::string)crypt_str("yaw"), &QAngle::yaw,
		(std::string)crypt_str("roll"), &QAngle::roll,
		(std::string)crypt_str("length"), &QAngle::Length,
		(std::string)crypt_str("length_sqr"), &QAngle::LengthSqr,
		(std::string)crypt_str("is_zero"), &QAngle::IsZero,
		(std::string)crypt_str("is_valid"), &QAngle::IsValid,
		(std::string)crypt_str("normalize"), &QAngle::Normalize
	);

	lua.new_usertype<player_info_t>(crypt_str("player_info"),
		(std::string)crypt_str("bot"), &player_info_t::fakeplayer,
		(std::string)crypt_str("name"), &player_info_t::getUsername,
		(std::string)crypt_str("steam_id"), &player_info_t::getGUID
	);

	lua.new_usertype<IGameEvent>(crypt_str("game_event"),
		(std::string)crypt_str("get_bool"), &IGameEvent::GetBool,
		(std::string)crypt_str("get_int"), &IGameEvent::GetInt,
		(std::string)crypt_str("get_float"), &IGameEvent::GetFloat,
		(std::string)crypt_str("get_string"), &IGameEvent::GetString,
		(std::string)crypt_str("set_bool"), &IGameEvent::SetBool,
		(std::string)crypt_str("set_int"), &IGameEvent::SetInt,
		(std::string)crypt_str("set_float"), &IGameEvent::SetFloat,
		(std::string)crypt_str("set_string"), &IGameEvent::SetString
	);

	lua.new_enum(crypt_str("hitboxes"),
		(std::string)crypt_str("head"), HITBOX_HEAD,
		(std::string)crypt_str("neck"), HITBOX_NECK,
		(std::string)crypt_str("pelvis"), HITBOX_PELVIS,
		(std::string)crypt_str("stomach"), HITBOX_STOMACH,
		(std::string)crypt_str("lower_chest"), HITBOX_LOWER_CHEST,
		(std::string)crypt_str("chest"), HITBOX_CHEST,
		(std::string)crypt_str("upper_chest"), HITBOX_UPPER_CHEST,
		(std::string)crypt_str("right_thigh"), HITBOX_RIGHT_THIGH,
		(std::string)crypt_str("left_thigh"), HITBOX_LEFT_THIGH,
		(std::string)crypt_str("right_calf"), HITBOX_RIGHT_CALF,
		(std::string)crypt_str("left_calf"), HITBOX_LEFT_CALF,
		(std::string)crypt_str("right_foot"), HITBOX_RIGHT_FOOT,
		(std::string)crypt_str("left_foot"), HITBOX_LEFT_FOOT,
		(std::string)crypt_str("right_hand"), HITBOX_RIGHT_HAND,
		(std::string)crypt_str("left_hand"), HITBOX_LEFT_HAND,
		(std::string)crypt_str("right_upper_arm"), HITBOX_RIGHT_UPPER_ARM,
		(std::string)crypt_str("right_lower_arm"), HITBOX_RIGHT_LOWER_ARM,
		(std::string)crypt_str("left_upper_arm"), HITBOX_LEFT_UPPER_ARM,
		(std::string)crypt_str("left_lower_arm"), HITBOX_LEFT_LOWER_ARM
	);

	lua.new_enum(crypt_str("flags"),
		(std::string)crypt_str("on_ground"), FL_ONGROUND,
		(std::string)crypt_str("ducking"), FL_DUCKING,
		(std::string)crypt_str("water_jump"), FL_WATERJUMP,
		(std::string)crypt_str("on_train"), FL_ONTRAIN,
		(std::string)crypt_str("in_rain"), FL_INRAIN,
		(std::string)crypt_str("frozen"), FL_FROZEN,
		(std::string)crypt_str("at_controls"), FL_ATCONTROLS,
		(std::string)crypt_str("client"), FL_CLIENT,
		(std::string)crypt_str("fake_client"), FL_FAKECLIENT
	);

	lua.new_usertype<player_t>(crypt_str("player"), sol::base_classes, sol::bases<entity_t>(),
		(std::string)crypt_str("get_index"), &player_t::EntIndex,
		(std::string)crypt_str("is_dormant"), &player_t::IsDormant,
		(std::string)crypt_str("get_team"), &player_t::m_iTeamNum,
		(std::string)crypt_str("is_alive"), &player_t::is_alive,
		(std::string)crypt_str("get_velocity"), &player_t::m_vecVelocity,
		(std::string)crypt_str("get_eye_position"), &player_t::get_eye_position,
		(std::string)crypt_str("get_hitbox_position"), &player_t::hitbox_position,
		(std::string)crypt_str("has_helmet"), &player_t::m_bHasHelmet,
		(std::string)crypt_str("has_heavy_armor"), &player_t::m_bHasHeavyArmor,
		(std::string)crypt_str("is_scoped"), &player_t::m_bIsScoped,
		(std::string)crypt_str("get_health"), &player_t::m_iHealth,
		(std::string)crypt_str("get_spawn_time"), &player_t::m_flSpawnTime,
		(std::string)crypt_str("get_max_desync_delta"), &player_t::get_max_desync_delta,
		(std::string)crypt_str("get_min_desync_delta"), &player_t::get_min_desync_delta,
		(std::string)crypt_str("flags"), &player_t::m_fFlags,
		(std::string)crypt_str("flash_duration"), &player_t::m_flFlashDuration
	);

	lua.new_usertype<weapon_t>(crypt_str("weapon"), sol::base_classes, sol::bases<entity_t>(),
		(std::string)crypt_str("ammo"), &weapon_t::m_iClip,
		(std::string)crypt_str("can_fire"), &weapon_t::can_fire,
		(std::string)crypt_str("is_non_aim"), &weapon_t::is_non_aim,
		(std::string)crypt_str("can_double_tap"), &weapon_t::can_double_tap,
		(std::string)crypt_str("get_name"), &weapon_t::get_name,
		(std::string)crypt_str("get_inaccuracy"), &weapon_t::get_inaccuracy,
		(std::string)crypt_str("get_spread"), &weapon_t::get_spread
	);

	lua.new_enum(crypt_str("buttons"),
		(std::string)crypt_str("in_attack"), IN_ATTACK,
		(std::string)crypt_str("in_attack2"), IN_ATTACK2,
		(std::string)crypt_str("in_jump"), IN_JUMP,
		(std::string)crypt_str("in_duck"), IN_DUCK,
		(std::string)crypt_str("in_forward"), IN_FORWARD,
		(std::string)crypt_str("in_back"), IN_BACK,
		(std::string)crypt_str("in_use"), IN_USE,
		(std::string)crypt_str("in_cancel"), IN_CANCEL,
		(std::string)crypt_str("in_left"), IN_LEFT,
		(std::string)crypt_str("in_right"), IN_RIGHT,
		(std::string)crypt_str("in_moveleft"), IN_MOVELEFT,
		(std::string)crypt_str("in_moveright"), IN_MOVERIGHT,
		(std::string)crypt_str("in_run"), IN_RUN,
		(std::string)crypt_str("in_reload"), IN_RELOAD,
		(std::string)crypt_str("in_alt1"), IN_ALT1,
		(std::string)crypt_str("in_alt2"), IN_ALT2,
		(std::string)crypt_str("in_score"), IN_SCORE,
		(std::string)crypt_str("in_speed"), IN_SPEED,
		(std::string)crypt_str("in_walk"), IN_WALK,
		(std::string)crypt_str("in_zoom"), IN_ZOOM,
		(std::string)crypt_str("in_weapon1"), IN_WEAPON1,
		(std::string)crypt_str("in_weapon2"), IN_WEAPON2,
		(std::string)crypt_str("in_bullrush"), IN_BULLRUSH,
		(std::string)crypt_str("in_grenade1"), IN_GRENADE1,
		(std::string)crypt_str("in_grenade2"), IN_GRENADE2,
		(std::string)crypt_str("in_lookspin"), IN_LOOKSPIN
	);

	lua.new_usertype<shot_info>(crypt_str("shot_info"), sol::constructors<>(),
		(std::string)crypt_str("target_name"), &shot_info::target_name,
		(std::string)crypt_str("result"), &shot_info::result,
		(std::string)crypt_str("client_hitbox"), &shot_info::client_hitbox,
		(std::string)crypt_str("server_hitbox"), &shot_info::server_hitbox,
		(std::string)crypt_str("client_damage"), &shot_info::client_damage,
		(std::string)crypt_str("server_damage"), &shot_info::server_damage,
		(std::string)crypt_str("hitchance"), &shot_info::hitchance,
		(std::string)crypt_str("backtrack_ticks"), &shot_info::backtrack_ticks,
		(std::string)crypt_str("aim_point"), &shot_info::aim_point
	);

	lua.new_usertype<resolver_info>(crypt_str("resolver_info"), sol::constructors<>(),
		(std::string)crypt_str("target_index"), &resolver_info::target_index,
		(std::string)crypt_str("is_yaw"), &resolver_info::is_yaw,
		(std::string)crypt_str("before_animation"), &resolver_info::before_animation,
		(std::string)crypt_str("current_animation"), &resolver_info::current_animation,
		(std::string)crypt_str("misses"), &resolver_info::misses,
		(std::string)crypt_str("shooting"), &resolver_info::shooting
	);

	lua.new_usertype<CUserCmd>(crypt_str("cmd"), sol::constructors<>(),
		(std::string)crypt_str("command_number"), &CUserCmd::m_command_number,
		(std::string)crypt_str("tickcount"), &CUserCmd::m_tickcount,
		(std::string)crypt_str("viewangles"), &CUserCmd::m_viewangles,
		(std::string)crypt_str("aimdirection"), &CUserCmd::m_aimdirection,
		(std::string)crypt_str("forwardmove"), &CUserCmd::m_forwardmove,
		(std::string)crypt_str("sidemove"), &CUserCmd::m_sidemove,
		(std::string)crypt_str("upmove"), &CUserCmd::m_upmove,
		(std::string)crypt_str("buttons"), &CUserCmd::m_buttons,
		(std::string)crypt_str("impulse"), &CUserCmd::m_impulse,
		(std::string)crypt_str("weaponselect"), &CUserCmd::m_weaponselect,
		(std::string)crypt_str("mousedx"), &CUserCmd::m_mousedx,
		(std::string)crypt_str("mousedy"), &CUserCmd::m_mousedy,
		(std::string)crypt_str("predicted"), &CUserCmd::m_predicted
	);

	auto cheat = lua.create_table();
	cheat[crypt_str("register_callback")] = ns_cheat::register_callback;
	cheat[crypt_str("load_script")] = ns_cheat::load_script;
	cheat[crypt_str("unload_script")] = ns_cheat::unload_script;
	cheat[crypt_str("print_log")] = ns_cheat::log;
	cheat[crypt_str("is_chat_open")] = ns_cheat::is_chat_open;
	cheat[crypt_str("should_breaking_lby")] = ns_cheat::should_breaking_lby;
	cheat[crypt_str("is_breaking_lby")] = ns_cheat::is_breaking_lby;
	cheat[crypt_str("set_breaking_lby")] = ns_cheat::set_breaking_lby;
	cheat[crypt_str("get_username")] = ns_cheat::get_username;
	cheat[crypt_str("get_edition")] = ns_cheat::get_edition;

	auto menu = lua.create_table();
	menu[crypt_str("next_line")] = ns_menu::next_line;
	menu[crypt_str("add_check_box")] = ns_menu::add_check_box;
	menu[crypt_str("add_combo_box")] = ns_menu::add_combo_box;
	menu[crypt_str("add_slider_int")] = ns_menu::add_slider_int;
	menu[crypt_str("add_slider_float")] = ns_menu::add_slider_float;
	menu[crypt_str("add_color_picker")] = ns_menu::add_color_picker;
	menu[crypt_str("get_bool")] = ns_menu::get_bool;
	menu[crypt_str("get_int")] = ns_menu::get_int;
	menu[crypt_str("get_float")] = ns_menu::get_float;
	menu[crypt_str("get_color")] = ns_menu::get_color;
	menu[crypt_str("set_key_bind_state")] = ns_menu::set_key_bind_state;
	menu[crypt_str("get_key_bind_state")] = ns_menu::get_key_bind_state;
	menu[crypt_str("get_key_bind_mode")] = ns_menu::get_key_bind_mode;
	menu[crypt_str("set_bool")] = ns_menu::set_bool;
	menu[crypt_str("set_int")] = ns_menu::set_int;
	menu[crypt_str("set_float")] = ns_menu::set_float;
	menu[crypt_str("set_color")] = ns_menu::set_color;

	auto globals = lua.create_table();
	globals[crypt_str("get_framerate")] = ns_globals::get_framerate;
	globals[crypt_str("get_ping")] = ns_globals::get_ping;
	globals[crypt_str("get_ip")] = ns_globals::get_ip;
	globals[crypt_str("get_time")] = ns_globals::get_time;
	globals[crypt_str("get_realtime")] = ns_globals::get_realtime;
	globals[crypt_str("get_curtime")] = ns_globals::get_curtime;
	globals[crypt_str("get_frametime")] = ns_globals::get_frametime;
	globals[crypt_str("get_tickcount")] = ns_globals::get_tickcount;
	globals[crypt_str("get_framecount")] = ns_globals::get_framecount;
	globals[crypt_str("get_intervalpertick")] = ns_globals::get_intervalpertick;
	globals[crypt_str("get_maxclients")] = ns_globals::get_maxclients;

	auto engine = lua.create_table();
	engine[crypt_str("get_screen_width")] = ns_engine::get_screen_width;
	engine[crypt_str("get_screen_height")] = ns_engine::get_screen_height;
	engine[crypt_str("get_mouse_x")] = ns_engine::get_mouse_x;
	engine[crypt_str("get_mouse_y")] = ns_engine::get_mouse_y;
	engine[crypt_str("mouse_in_region")] = ns_engine::mouse_in_region;
	engine[crypt_str("get_level_name")] = ns_engine::get_level_name;
	engine[crypt_str("get_level_name_short")] = ns_engine::get_level_name_short;
	engine[crypt_str("get_local_player_index")] = ns_engine::get_local_player_index;
	engine[crypt_str("get_map_group_name")] = ns_engine::get_map_group_name;
	engine[crypt_str("get_player_for_user_id")] = ns_engine::get_player_for_user_id;
	engine[crypt_str("get_player_info")] = ns_engine::get_player_info;
	engine[crypt_str("get_view_angles")] = ns_engine::get_view_angles;
	engine[crypt_str("is_connected")] = ns_engine::is_connected;
	engine[crypt_str("is_hltv")] = ns_engine::is_hltv;
	engine[crypt_str("is_in_game")] = ns_engine::is_in_game;
	engine[crypt_str("is_paused")] = ns_engine::is_paused;
	engine[crypt_str("is_playing_demo")] = ns_engine::is_playing_demo;
	engine[crypt_str("is_recording_demo")] = ns_engine::is_recording_demo;
	engine[crypt_str("is_taking_screenshot")] = ns_engine::is_taking_screenshot;
	engine[crypt_str("set_view_angles")] = ns_engine::set_view_angles;

	auto render = lua.create_table();
	render[crypt_str("world_to_screen")] = ns_render::world_to_screen;
	render[crypt_str("get_text_width")] = ns_render::get_text_width;
	render[crypt_str("create_image")] = ns_render::create_image;
	render[crypt_str("draw_image")] = ns_render::draw_image;
	render[crypt_str("create_font")] = ns_render::create_font;
	render[crypt_str("draw_text")] = ns_render::draw_text;
	render[crypt_str("draw_text_centered")] = ns_render::draw_text_centered;
	render[crypt_str("draw_line")] = ns_render::draw_line;
	render[crypt_str("draw_rect")] = ns_render::draw_rect;
	render[crypt_str("draw_rect_filled")] = ns_render::draw_rect_filled;
	render[crypt_str("draw_rect_filled_gradient")] = ns_render::draw_rect_filled_gradient;
	render[crypt_str("draw_circle")] = ns_render::draw_circle;
	render[crypt_str("draw_circle_filled")] = ns_render::draw_circle_filled;
	render[crypt_str("draw_triangle")] = ns_render::draw_triangle;

	auto console = lua.create_table();
	console[crypt_str("execute")] = ns_console::execute;
	console[crypt_str("is_visible")] = ns_console::is_visible;
	console[crypt_str("get_int")] = ns_console::get_int;
	console[crypt_str("get_float")] = ns_console::get_float;
	console[crypt_str("get_string")] = ns_console::get_string;
	console[crypt_str("set_int")] = ns_console::set_int;
	console[crypt_str("set_float")] = ns_console::set_float;
	console[crypt_str("set_string")] = ns_console::set_string;

	auto events = lua.create_table();
	events[crypt_str("register_event")] = ns_events::register_event;

	auto entitylist = lua.create_table();
	entitylist[crypt_str("get_local_player")] = ns_entitylist::get_local_player;
	entitylist[crypt_str("get_player_by_index")] = ns_entitylist::get_player_by_index;
	entitylist[crypt_str("get_entity_by_class_id")] = ns_entitylist::get_entity_by_class_id;
	entitylist[crypt_str("get_weapon_by_player")] = ns_entitylist::get_weapon_by_player;

	auto cmd = lua.create_table();
	cmd[crypt_str("get_send_packet")] = ns_cmd::get_send_packet;
	cmd[crypt_str("set_send_packet")] = ns_cmd::set_send_packet;
	cmd[crypt_str("get_choke")] = ns_cmd::get_choke;
	cmd[crypt_str("get_button_state")] = ns_cmd::get_button_state;
	cmd[crypt_str("set_button_state")] = ns_cmd::set_button_state;

	auto memory = lua.create_table();
	memory[crypt_str("find_pattern")] = ns_memory::find_pattern;
	memory[crypt_str("create_interface")] = ns_memory::create_interface;

	auto indicators = lua.create_table();
	indicators[crypt_str("add")] = ns_indicator::add_indicator;
	indicators[crypt_str("add_with_position")] = ns_indicator::add_indicator_with_pos;

	auto http = lua.create_table();
	http[crypt_str("get")] = ns_http::get;
	http[crypt_str("post")] = ns_http::post;

	auto file = lua.create_table();
	file[crypt_str("append")] = ns_file::append;
	file[crypt_str("write")] = ns_file::write;
	file[crypt_str("read")] = ns_file::read;

	lua[crypt_str("cheat")] = cheat;
	lua[crypt_str("menu")] = menu;
	lua[crypt_str("globals")] = globals;
	lua[crypt_str("engine")] = engine;
	lua[crypt_str("render")] = render;
	lua[crypt_str("console")] = console;
	lua[crypt_str("events")] = events;
	lua[crypt_str("entitylist")] = entitylist;
	lua[crypt_str("cmd")] = cmd;
	lua[crypt_str("memory")] = memory;
	lua[crypt_str("indicators")] = indicators;
	lua[crypt_str("http")] = http;
	lua[crypt_str("file")] = file;

	refresh_scripts();
}

int c_lua::get_script_id(const std::string& name)
{
	for (auto i = 0; i < scripts.size(); i++)
		if (scripts.at(i) == name)
			return i;

	return -1;
}

int c_lua::get_script_id_by_path(const std::string& path)
{
	for (auto i = 0; i < pathes.size(); i++)
		if (pathes.at(i).string() == path)
			return i;

	return -1;
}

void c_lua::refresh_scripts()
{
	auto oldLoaded = loaded;
	auto oldScripts = scripts;

	loaded.clear();
	pathes.clear();
	scripts.clear();
	ns_console::convars.clear();

	std::string folder = cfg_manager->get_folder_path(PathType::Lua);

	auto i = 0;

	for (auto& entry : std::filesystem::directory_iterator(folder))
	{
		if (entry.path().extension() == crypt_str(".lua") || entry.path().extension() == crypt_str(".luac"))
		{
			auto path = entry.path();
			auto filename = path.filename().string();

			auto didPut = false;

			for (auto i = 0; i < oldScripts.size(); i++)
			{
				if (filename == oldScripts.at(i))
				{
					loaded.emplace_back(oldLoaded.at(i));
					didPut = true;
				}
			}

			if (!didPut)
				loaded.emplace_back(false);

			pathes.emplace_back(path);
			scripts.emplace_back(filename);

			items.emplace_back(std::vector<std::pair<std::string, menu_item>>());
			++i;
		}
	}
}

void c_lua::load_script(int id)
{
	if (id == -1)
		return;

	if (loaded.at(id))
		return;

	auto path = get_script_path(id);

	if (path == crypt_str(""))
		return;

	auto error_load = false;
	loaded.at(id) = true;
	lua.script_file(path,
		[&error_load](lua_State*, sol::protected_function_result result)
		{
			if (!result.valid())
			{
				sol::error error = result;
				auto log = crypt_str("Lua error: ") + (std::string)error.what();

				eventlogs::get().add(crypt_str("ERROR"), log, false);
				error_load = true;
			}

			return result;
		}
	);

	if (error_load || !loaded.at(id))
	{
		loaded.at(id) = false;
		return;
	}

	cheat.globals.loaded_script = true;
}

void c_lua::unload_script(int id)
{
	if (id == -1)
		return;

	if (!loaded.at(id))
		return;

	items.at(id).clear();

	if (c_lua::get().events.find(id) != c_lua::get().events.end())
		c_lua::get().events.at(id).clear();

	hooks.unregisterHooks(id);
	loaded.at(id) = false;
}

void c_lua::reload_all_scripts()
{
	for (auto& current : scripts)
	{
		if (!loaded.at(get_script_id(current)))
			continue;

		unload_script(get_script_id(current));
		load_script(get_script_id(current));
	}
}

void c_lua::unload_all_scripts()
{
	for (auto s : scripts)
		unload_script(get_script_id(s));
}

std::string c_lua::get_script_path(const std::string& name)
{
	return get_script_path(get_script_id(name));
}

std::string c_lua::get_script_path(int id)
{
	if (id == -1)
		return crypt_str("");

	return pathes.at(id).string();
}
