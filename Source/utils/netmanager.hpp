#pragma once

#include "..\sdk\misc\Recv.hpp"
#include <unordered_map>

#define NETVAR(type, name, table, netvar)                                               \
[[nodiscard]] std::add_lvalue_reference_t<type> name() noexcept                         \
{                                                                                       \
    static const auto netvarOffset = netvars::get().get_offset(table, netvar);          \
    return *reinterpret_cast<std::add_pointer_t<type>>(uintptr_t(this) + netvarOffset); \
}

#define PNETVAR(type, name, table, netvar)                                              \
[[nodiscard]] auto name() noexcept                                                      \
{                                                                                       \
    static const auto netvarOffset = netvars::get().get_offset(table, netvar);          \
    return reinterpret_cast<std::add_pointer_t<type>>(uintptr_t(this) + netvarOffset);  \
}

#define POFFSET(type, name, offset)                                                      \
[[nodiscard]] std::add_lvalue_reference_t<type> name() noexcept                         \
{                                                                                       \
    return reinterpret_cast<std::add_pointer_t<type>>(uintptr_t(this) + offset);        \
}

#define OFFSET(type, name, offset)                                                      \
[[nodiscard]] std::add_lvalue_reference_t<type> name() noexcept                         \
{                                                                                       \
    return *reinterpret_cast<std::add_pointer_t<type>>(uintptr_t(this) + offset);       \
}

class netvars
{
public:
	std::unordered_map<std::string, RecvTable*> tables;

	int get_offset(const char* tableName, const char* propName); 
	int get_prop(const char* tableName, const char* propName, RecvProp** prop = nullptr);
	int get_prop(RecvTable* recvTable, const char* propName, RecvProp** prop = nullptr);

	static netvars& get()
	{
		static netvars instance;
		return instance;
	}

private:
	RecvTable* get_table(const char* tableName);
};
