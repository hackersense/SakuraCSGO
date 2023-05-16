#pragma once

#include <array>
#include <type_traits>
#include <cstdint>
#include <optional>

#include <Windows.h>
#include <Psapi.h>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "ItemSchema.h"
#include "math\Vector.hpp"
#include "misc\IKeyValuesSystem.hpp"
#include "SteamAPI.h"
#include "misc\UtlMap.hpp"

class IClientEntity;
class MemAlloc;
class KeyValues;
struct IPanoramaEventRegistration;
struct Ray_t;
struct mstudiobbox_t;
class matrix3x4_t;
class CGameTrace;

struct tesla_info_t {
    Vector m_pos;
    Vector m_ang;
    int m_entindex;
    const char* m_spritename;
    float m_flbeamwidth;
    int m_nbeams;
    Vector m_color;
    float m_fltimevis;
    float m_flradius;
};

class Memory {
public:
    Memory() noexcept;

    std::add_pointer_t<void __fastcall(const char*)> loadSky;
    CUtlMap<short, IPanoramaEventRegistration>* registeredPanoramaEvents;
    std::uintptr_t present;
    std::uintptr_t reset;
    std::uintptr_t hud;
    std::uintptr_t demoFileEndReached;
    std::uintptr_t demoOrHLTV;
    std::uintptr_t money;
    std::uintptr_t inventoryAccess;
    std::uintptr_t sendDatagram;
    float(__thiscall* getFirstSequenceAnimTag)(void*, int, int, int);
    float(__thiscall* sequenceDuration)(void*, int);
    int* (__thiscall* findHudElement)(std::uintptr_t, const char*);
    int(__thiscall* clearHudWeapon)(int*, int);
    std::add_pointer_t<ItemSystem* __cdecl()> itemSystem;
    void(__thiscall* setAbsOrigin)(void*, const Vector&);
    void(__thiscall* setAbsAngles)(void*, const Vector&);
    std::uintptr_t insertIntoTree;
    std::uintptr_t scopeDust;
    std::uintptr_t scopeArc;
    int(__thiscall* equipWearable)(void* wearable, void* player);
    int(__thiscall* seqdesc)(void*, int);
    std::add_pointer_t<bool __cdecl(Vector, Vector, short)> lineGoesThroughSmoke;
    std::uint8_t* fakePrime;
    std::uintptr_t keyValuesFromString;
    KeyValues*(__thiscall* keyValuesFindKey)(KeyValues* keyValues, const char* keyName, bool create);
    void(__thiscall* keyValuesSetString)(KeyValues* keyValues, const char* value);
    std::add_pointer_t<bool __fastcall(IClientEntity*)> isBreakableEntityFn;
    int(__fastcall* clipRayToHixbox)(const Ray_t&, mstudiobbox_t*, matrix3x4_t&, CGameTrace&);
    std::uintptr_t clSendMove;
    void(__thiscall* clMsgMoveSetData)(void*, unsigned char*, std::size_t);
    void(__thiscall* clMsgMoveDescontructor)(void*);
    std::uintptr_t clMove;
    std::uintptr_t chokeLimit;
    std::uintptr_t traceFilterForHeadCollision;
    std::uintptr_t modifyEyePosition;
    std::uintptr_t calculateView;
    std::uintptr_t checkForSequenceChange;
    std::uintptr_t particleCollection;
    std::uintptr_t physicsSimulate;
    std::uintptr_t getForeignFallbackFontName;
    std::uintptr_t processInterpolatedList;
    std::uintptr_t standardBlendingRules;
    std::uintptr_t doExtraBoneProcessing;
    std::uintptr_t shouldSkipAnimationFrame;
    std::uintptr_t updateClientSideAnimation;
    std::uintptr_t eyePositionAndVectors;
    std::uintptr_t buildTransformations;
    std::uintptr_t eyeAngles;
    std::uintptr_t calculateViewModelBob;
    std::uintptr_t updateState;
    std::uintptr_t createState;
    std::uintptr_t resetState;
    std::uintptr_t invalidateBoneCache;
    std::string* relayCluster;
    int(__thiscall* lookUpBone)(void*, const char*);
    std::uintptr_t setupBones;
    void(__stdcall* restoreEntityToPredictedFrame)(int, int);
    bool(__thiscall* physicsRunThink)(void*, int);
    void(__thiscall* checkHasThinkFunction)(void*, bool);
    bool(__thiscall* postThinkVPhysics)(void*);
    void(__thiscall* simulatePlayerSimulatedEntities)(void*);
    void(__fastcall* fs_tesla)(tesla_info_t&);
    int* predictionRandomSeed;
    int* predictionPlayer;
    std::uintptr_t traceToExit;
    std::uintptr_t drawScreenEffectMaterial;
    std::uintptr_t isDepthOfFieldEnabled;
    std::uintptr_t cameraThink;
    bool usingKeyValues = true;
    class IKeyValuesSystem* keyValuesSystem;
    std::uintptr_t keyValuesAllocEngine;
    std::uintptr_t keyValuesAllocClient;
    std::uintptr_t returnAddressClient;
    std::uintptr_t returnAddressEngine;
    std::uintptr_t returnAddressStudioRender;
    std::uintptr_t returnAddressMaterialSystem;
    std::uintptr_t jmpEbxGadgetInClient;
    std::uintptr_t shouldDrawFogReturnAddress;
    std::uintptr_t performScreenOverlay;
    MemAlloc* memalloc;
    SteamGameCoordinator* steamGameCoordinator;

    void* (__cdecl* createSimpleThread)(void*, void*, unsigned long);
    int(__cdecl* releaseThreadHandle)(void*);

    void setClanTag(const char* tag) const noexcept
    {
        setClanTagFn(tag, tag);
    }

    short makePanoramaSymbol(const char* name) const noexcept
    {
        short symbol;
        makePanoramaSymbolFn(&symbol, name);
        return symbol;
    }

    bool submitReport(const char* xuid, const char* report) const noexcept
    {
        return reinterpret_cast<bool(__stdcall*)(const char*, const char*)>(submitReportFunction)(xuid, report);
    }

    struct ModuleInfo {
        void* base;
        std::size_t size;
        std::string origin_name;
    };

    static ModuleInfo getModuleInformation(const char* name) noexcept
    {
        if (HMODULE handle = GetModuleHandleA(name)) {
            if (MODULEINFO moduleInfo; GetModuleInformation(GetCurrentProcess(), handle, &moduleInfo, sizeof(moduleInfo)))
                return ModuleInfo{ moduleInfo.lpBaseOfDll, moduleInfo.SizeOfImage, name };
        }
        return {};
    }

    [[nodiscard]] static auto generateBadCharTable(std::string_view pattern) noexcept
    {
        assert(!pattern.empty());

        std::array<std::size_t, (std::numeric_limits<std::uint8_t>::max)() + 1> table;

        auto lastWildcard = pattern.rfind('?');
        if (lastWildcard == std::string_view::npos)
            lastWildcard = 0;

        const auto defaultShift = (std::max)(std::size_t(1), pattern.length() - 1 - lastWildcard);
        table.fill(defaultShift);

        for (auto i = lastWildcard; i < pattern.length() - 1; ++i)
            table[static_cast<std::uint8_t>(pattern[i])] = pattern.length() - 1 - i;

        return table;
    }

    [[nodiscard]] static std::string patternToString(std::string_view pattern)
    {
        std::ostringstream os;

        bool printedFirst = false;
        for (auto byte : pattern) {
            if (printedFirst)
                os << ' ';

            if (byte != '?')
                os << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<unsigned>(static_cast<unsigned char>(byte));
            else
                os << '?';

            printedFirst = true;
        }

        return os.str();
    }

    template <bool ReportNotFound = true>
    static std::uintptr_t findPattern(ModuleInfo moduleInfo, std::string_view pattern) noexcept
    {
        static auto id = 0;
        ++id;

        if (moduleInfo.base && moduleInfo.size) {
            const auto lastIdx = pattern.length() - 1;
            const auto badCharTable = generateBadCharTable(pattern);

            auto start = static_cast<const char*>(moduleInfo.base);
            const auto end = start + moduleInfo.size - pattern.length();

            while (start <= end) {
                int i = lastIdx;
                while (i >= 0 && (pattern[i] == '?' || start[i] == pattern[i]))
                    --i;

                if (i < 0)
                    return reinterpret_cast<std::uintptr_t>(start);

                start += badCharTable[static_cast<std::uint8_t>(start[lastIdx])];
            }
        }

        assert(false);
#ifdef _DEBUG
        if constexpr (ReportNotFound)
            MessageBoxA(nullptr, ("Failed to find pattern:\n" + patternToString(pattern)).c_str(), ("Sakura - " + moduleInfo.origin_name).c_str(), MB_OK | MB_ICONWARNING);
#endif
        return 0;
    }

    template <bool ReportNotFound = true>
    static std::uintptr_t findPattern(const char* moduleName, std::string_view pattern) noexcept
    {
        return findPattern<ReportNotFound>(getModuleInformation(moduleName), pattern);
    }
private:
    void(__thiscall* makePanoramaSymbolFn)(short* symbol, const char* name);
    std::uintptr_t submitReportFunction;
    std::add_pointer_t<void __fastcall(const char*, const char*)> setClanTagFn;
};

inline std::optional<const Memory> memory;