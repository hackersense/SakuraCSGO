#pragma once

#include <deque>

#include "..\..\includes.hpp"

class eventlogs : public singleton<eventlogs> 
{
public:
	void render(ImDrawList* draw);
	void events(IGameEvent* event);
	void add(std::string text, bool full_display = true);
	void add(std::string pre, std::string text, bool full_display = true);

	bool last_log = false;
private:
	struct notify_t
	{
		float life_ime_local;
		float life_ime;
		std::string type;
		std::string message;
		Color c_type;
		Color c_message;
		float x;
		float y = -15;
		float max_x;
	};

	std::deque<notify_t> notifications;
};