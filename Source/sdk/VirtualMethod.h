#pragma once

#include <cstddef>
#include "../utils/x86RetSpoof.h"
#include "Memory.h"

#define VIRTUAL_METHOD(returnType, name, idx, args, argsRaw) \
returnType name args noexcept \
{ \
    return VirtualMethod::call<returnType, idx>argsRaw; \
}

namespace VirtualMethod
{
    template <typename T, std::size_t Idx, typename ...Args>
    constexpr T call(void* classBase, Args... args) noexcept
    {
        return (*reinterpret_cast<T(__thiscall***)(void*, Args...)>(classBase))[Idx](classBase, args...);
        //return x86RetSpoof::invokeThiscall<T, Args...>(std::uintptr_t(classBase), (*reinterpret_cast<std::uintptr_t**>(classBase))[Idx], memory->jmpEbxGadgetInClient, args...);
    }
}
