#pragma once

#include "..\includes.hpp"

class UnicodeFonts
{
public:
	struct {
		ImFont* normal15px = nullptr;
		ImFont* chinese = nullptr;
		ImFont* chinese10 = nullptr;
		ImFont* chinese14 = nullptr;
	} fonts;

	void init(ImGuiIO io);
};

inline UnicodeFonts unicodeFont;