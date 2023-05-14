#pragma once

#include <ShlObj_core.h>
#include <string>
#include <vector>

#include "..\function\js_functions.hpp"
#include "..\api\api.h"
#include "..\api\config.h"
#include "..\..\hooks\hooks.hpp"
#include "..\..\features\misc\logs.h"

enum exec_locations
{
	none,
	create_move_hk,
	frame_stage_notify_hk,
	paint_traverse_hk
};

class c_js_script
{
public:
	std::string name = "null";
	int execution_location = none; //this should be exec_locations, but imgui goes ham if it isnt
	bool is_enabled = false;
	int prev_len = 0;
	int current_len = 0;
	bool had_error = false;

	void execute()
	{
		std::string folder = cfg_manager->get_folder_path(PathType::JavaScript);

		const auto script_path = folder + this->name;

		auto script = js_funcs.read_script_from_file(script_path);
		this->current_len = script.length();
		if (this->current_len == 0)
			return;

		if (this->had_error && this->prev_len == this->current_len)
			return;

		this->prev_len = this->current_len;

		this->had_error = duk_peval_string(cheat.context_default, script.c_str()) != 0;
		if (this->had_error)
		{
			eventlogs::get().add("script error");
		}
	}
};

class c_js_loader
{
public:
	static std::vector<c_js_script> scripts;

	static bool convert_v_string(std::vector<std::string> script_names);

	static std::vector<c_js_script> refresh_scripts(std::vector<std::string> script_names);

	static void execute_scripts(exec_locations loc);
};