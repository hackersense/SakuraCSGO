#include "js_functions.hpp"

#include <fstream>

#include "..\api\config.h"
#include "..\api\api.h"
#include "..\loader\js_loader.h"
#include "..\..\features\misc\logs.h"
#include "..\..\includes.hpp"

c_js_funcs js_funcs;
#define non_stat duk_ret_t

non_stat c_js_funcs::add_log(duk_ctx)
{
	eventlogs::get().add(duk_to_string(ctx, 0));
	return 1;
}

non_stat c_js_funcs::add_rectfilled(duk_ctx)
{
	render::get().rect_filled(duk_to_int(ctx, 0), duk_to_int(ctx, 1), duk_to_int(ctx, 2), duk_to_int(ctx, 3), Color(duk_to_int(ctx, 4), duk_to_int(ctx, 5), duk_to_int(ctx, 6), duk_to_int(ctx, 7)));
	return 1;
}

non_stat c_js_funcs::add_to_hook(duk_ctx)
{
	for (auto f : c_js_loader::scripts)
	{
		f.execution_location = duk_to_int(ctx, 0);
	}
	return 0;
}

void c_js_funcs::setup_js()
{
	const auto add_function = [&](const duk_c_function fn, const int args, const std::string& fn_name) -> void
	{
		duk_push_c_function(cheat.context_default, fn, args);
		duk_put_global_string(cheat.context_default, fn_name.c_str());
	};

	add_function(add_log, 1, "add_log"); 
	add_function(add_rectfilled, 8, "add_rect_filled");
}

std::string c_js_funcs::read_script_from_file(std::string path)
{
	std::fstream script_file;
	script_file.open(path, std::ios::in); //open a file to perform read operation using file object
	std::string script;
	if (script_file.is_open()) {   //checking whether the file is open
		std::string tp;
		while (std::getline(script_file, tp)) {  //read data from file object and put it into string.
			script += tp;  //print the data of the string
		}
		script_file.close();   //close the file object.
	}
	return script;
}

std::vector<std::string> c_js_funcs::get_js_files()
{
	std::vector<std::string> scripts;
	WIN32_FIND_DATAA ffd;
	std::string folder = cfg_manager->get_folder_path(PathType::JavaScript);

	auto directory = folder + "*";
	auto hFind = FindFirstFileA(directory.c_str(), &ffd);
	while (FindNextFileA(hFind, &ffd))
	{
		if (!(ffd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
		{
			std::string file_name = ffd.cFileName;
			scripts.push_back(file_name);
		}
	}
	return scripts;
}
