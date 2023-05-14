#pragma once

#include <string>
#include <vector>
#include <fstream>

#include "..\ImGui\imgui.h"
#define IMGUI_DEFINE_MATH_OPERATORS
#include "..\ImGui\imgui_internal.h"

namespace FontUtils
{
	bool decodeVFONT(std::vector<char>& buffer) noexcept;
	std::vector<char> loadBinaryFile(const std::string& path) noexcept;
	ImWchar* getFontGlyphRanges() noexcept;
}