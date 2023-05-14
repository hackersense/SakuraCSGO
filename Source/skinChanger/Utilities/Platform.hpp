#pragma once
#include <cstdint>
#include <utility>
#include <array>

namespace platform
{
    std::pair<std::uintptr_t, std::size_t> get_module_info(const char* module_name);
	auto is_code_ptr(void* ptr) -> bool;
}
