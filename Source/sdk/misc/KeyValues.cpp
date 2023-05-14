#include "KeyValues.hpp"
#include "IKeyValuesSystem.hpp"
#include "../Memory.h"

KeyValues* KeyValues::FromString(char const* szName, char const* szStringVal, char const** ppEndOfParse)
{
    const auto keyValuesFromString = memory->keyValuesFromString;
    KeyValues* keyValues;
    __asm {
        push 0
        mov edx, szStringVal
        mov ecx, szName
        call keyValuesFromString
        add esp, 4
        mov keyValues, eax
    }
    return keyValues;
}

KeyValues* KeyValues::FindKey(const char* keyName, bool bCreate)
{
    return memory->keyValuesFindKey(this, keyName, bCreate);
}

void KeyValues::SetString(const char* keyName, const char* value)
{
    if (const auto key = FindKey(keyName, true))
        memory->keyValuesSetString(key, value);
}

const char* KeyValues::GetName() const
{
    return GetKeyValuesSystem()->GetStringForSymbol(*reinterpret_cast<std::uint8_t*>(std::uintptr_t(this) + 3) | (*reinterpret_cast<std::uint16_t*>(std::uintptr_t(this) + 18) << 8));
}
