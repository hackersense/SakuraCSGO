#include "platform.hpp"
#include <vector>
#include <algorithm>

// Platform tools for windows. Maybe I'll make linux ones too

#include <Windows.h>
#include <psapi.h>

std::pair<std::uintptr_t, std::size_t> platform::get_module_info(const char* module_name)
{
	const auto module = GetModuleHandleA(module_name);
	if (!module)
		return std::make_pair(0, 0);
	MODULEINFO module_info;
	K32GetModuleInformation(GetCurrentProcess(), module, &module_info, sizeof(MODULEINFO));
	return std::make_pair(std::uintptr_t(module_info.lpBaseOfDll), module_info.SizeOfImage);
}

auto platform::is_code_ptr(void* ptr) -> bool
{
	constexpr const DWORD protect_flags = PAGE_EXECUTE | PAGE_EXECUTE_READ | PAGE_EXECUTE_READWRITE | PAGE_EXECUTE_WRITECOPY;

	MEMORY_BASIC_INFORMATION out;
	VirtualQuery(ptr, &out, sizeof out);

	return out.Type
		&& !(out.Protect & (PAGE_GUARD | PAGE_NOACCESS))
		&& out.Protect & protect_flags;
}
