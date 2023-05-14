#pragma once

#include "..\includes.hpp"

namespace ConsoleUtils
{
	std::uintptr_t create_convar(const char* pName, const char* pDefaultValue, int flags = 0,
		const char* pHelpString = NULL, bool bMin = false, float fMin = 0.0,
		bool bMax = false, float fMax = false, FnChangeCallback_t callback = NULL) noexcept;
}
