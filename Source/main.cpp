#include <ShlObj.h>
#include <shlobj_core.h>
#include <DbgHelp.h>
#include "sdk\interfaces\IInputInternal.hpp"
#include "utils\cheat.hpp"
#include "utils\recv.h"
#include "utils\fnv.h"
#include "utils\HWIDUtils.h"
#include "utils\ConsoleUtils.h"
#include "utils\StringUtils.h"
#include "utils\SomeUtils.h"
#include "utils\AntiDetection.h"
#include "sdk\SteamInputAPI.h"
#include "lua\Lua51.h"
#include "utils\HTTPUtils.h"
#include "utils\WindowUtils.h"
#include "profileChanger\Protobuffs.h"
#include "skinChanger\SkinChanger.h"
#include "features\menu.h"
#include "features\legitbot\backtrack.h"
#include "features\visuals\world_esp.h"
#include "features\misc\misc.h"
#include "features\serverside\local_animations.h"
#include "features\imgs\skeet_tab.h"
#include "hooks\libs\MinHook.h"
#include "sdk\interfaces\CLMsgMove.h"
#include "sdk\BBData.h"
#include "DiscordRpc.h"

__forceinline void setup_netvars()
{
    netvars::get().tables.clear();
    auto client = m_client()->GetAllClasses();

    if (!client)
        return;

    while (client)
    {
        auto recvTable = client->m_pRecvTable;

        if (recvTable)
            netvars::get().tables.emplace(std::string(client->m_pNetworkName), recvTable);

        client = client->m_pNext;
    }
}

int buildTransformsIndex;
Vector correctAngle;

using GetEyeAngles = Vector * (__thiscall*)(void*);
using BuildTransformations = Vector * (__thiscall*)(void*, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, void* computed);

static Vector* __fastcall getEyeAngles(void* ecx, void* edx) noexcept
{
    static auto original = Hooks::player_hook->getOriginal<GetEyeAngles>(170);

    const auto entity = reinterpret_cast<player_t*>(ecx);
    if (!cheat.local() || entity != cheat.local())
        return original(ecx);

    if (std::uintptr_t(_ReturnAddress()) != memory->eyePositionAndVectors)
        return original(ecx);

    if (buildTransformsIndex == -1 || buildTransformsIndex != entity->EntIndex())
        return original(ecx);

    buildTransformsIndex = -1;

    return &correctAngle;
}

static void __fastcall buildTransformations(void* ecx, void* edx, CStudioHdr* hdr, Vector* pos, Quaternion* q, matrix3x4_t* cameraTransform, int bonemask, void* computed) noexcept
{
    static auto original = Hooks::player_hook->getOriginal<BuildTransformations>(190);

    const auto entity = reinterpret_cast<player_t*>(ecx);
    if (entity && entity->is_alive() && cheat.local() && cheat.local() == entity && entity->get_animation_state())
    {
        buildTransformsIndex = entity->EntIndex();
        correctAngle = Vector(entity->get_animation_state()->m_flEyePitch, entity->get_animation_state()->m_flEyeYaw, local_animations::get().local_data.fake_angles.z);
        entity->m_flCollisionChangeTime() = 0.0f;
        //const auto animstate = reinterpret_cast<player_t*>(ecx)->get_animation_state();
        //vecBuildTransformationsAngles = Vector(animstate->m_flEyePitch, animstate->m_flEyeYaw, local_animations::get().local_data.fake_angles.z);
        //auto& renderAngles = cheat.local()->GetRenderAngles();
        //vecBuildTransformationsAngles = Vector(renderAngles.x, renderAngles.y, local_animations::get().local_data.fake_angles.z);
        //cheat.local()->m_flCollisionChangeTime() = 0.0f;
    }

    /*
    const CUtlVector<int> backupFlags = hdr->boneFlags;

    for (int i = 0; i < hdr->boneFlags.m_Size; i++)
    {
        if (configs.visuals.disable_jiggle_bones)
            hdr->boneFlags.m_pElements[i] &= ~0x04;
        else
            hdr->boneFlags.m_pElements[i] |= 0x04;
    }
    */

    original(ecx, hdr, pos, q, cameraTransform, bonemask, computed);

    //hdr->boneFlags = backupFlags;
}

static Vector* __fastcall eyeAnglesHook(void* thisPointer, void* edx) noexcept
{
    static auto original = Hooks::eyeangles_hook.getOriginal<Vector*>();

    const auto entity = reinterpret_cast<player_t*>(thisPointer);
    if (!cheat.local() || entity != cheat.local())
        return original(thisPointer);

    if (std::uintptr_t(_ReturnAddress()) != memory->eyePositionAndVectors)
        return original(thisPointer);

    if (buildTransformsIndex == -1 || buildTransformsIndex != entity->EntIndex())
        return original(thisPointer);

    buildTransformsIndex = -1;

    return &correctAngle;
}

static void __fastcall buildTransformationsHook(void* thisPointer, void* edx, CStudioHdr* hdr, void* pos, void* q, matrix3x4_t* cameraTransform, int boneMask, void* boneComputed) noexcept
{
    const auto entity = reinterpret_cast<player_t*>(thisPointer);
    if (entity && entity->is_alive() && cheat.local() && cheat.local() == entity && entity->get_animation_state())
    {
        buildTransformsIndex = entity->EntIndex();
        correctAngle = Vector(entity->get_animation_state()->m_flEyePitch, entity->get_animation_state()->m_flEyeYaw, local_animations::get().local_data.fake_angles.z);
        entity->m_flCollisionChangeTime() = 0.0f;
    }

    Hooks::buildtransformations_hook.callOriginal<void>(thisPointer, hdr, pos, q, cameraTransform, boneMask, boneComputed);
}

static void* __stdcall allocKeyValuesMemory(int size) noexcept
{
    if (const auto returnAddress = std::uintptr_t(_ReturnAddress()); size > 0 && (returnAddress == memory->keyValuesAllocEngine || returnAddress == memory->keyValuesAllocClient))
        return nullptr;

    return Hooks::keyvaluessystem_hook.callOriginal<void*, 2>(size);
}

static char __fastcall returnAddressBypass(void* ecx, void* edx, const char* moduleName) noexcept
{
    return true;
}

static const DemoPlaybackParameters* __stdcall getDemoPlaybackParameters() noexcept
{
    const auto params = Hooks::engine_hook.callOriginal<const DemoPlaybackParameters*, 218>();

    if (params && configs.misc.reveal_suspect && std::uintptr_t(_ReturnAddress()) != memory->demoFileEndReached) {
        static DemoPlaybackParameters customParams;
        customParams = *params;
        customParams.anonymousPlayerIdentity = false;
        return &customParams;
    }

    return params;
}

using GCRetrieveMessage = SteamResult(__thiscall*)(void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize);
static SteamResult __fastcall retrieveMessage(void* ecx, void*, uint32_t* punMsgType, void* pubDest, uint32_t cubDest, uint32_t* pcubMsgSize) noexcept
{
    static auto retrieveMessage = Hooks::steam_hook->getOriginal<GCRetrieveMessage>(2);
    const auto status = retrieveMessage(ecx, punMsgType, pubDest, cubDest, pcubMsgSize);

    if (configs.profileChanger.enabled && status == SteamResult::OK)
    {
        void* thisPtr = nullptr;
        __asm mov thisPtr, ebx;
        auto oldEBP = *reinterpret_cast<void**>(uint32_t(_AddressOfReturnAddress()) - 4);

        uint32_t messageType = *punMsgType & 0x7FFFFFFF;
        write.receiveMessage(thisPtr, oldEBP, messageType, pubDest, cubDest, pcubMsgSize);
    }

    return status;
}

static bool __stdcall dispatchUserMessage(ECstrike15UserMessages type, int passthroughFlags, int size, const void* data) noexcept
{
    if (type == ECstrike15UserMessages::CS_UM_TextMsg || type == ECstrike15UserMessages::CS_UM_HudMsg || type == ECstrike15UserMessages::CS_UM_SayText)
    {
        if (configs.misc.ad_block && !m_gamerules()->m_bIsValveDS())
            return true;
    }
    else if (type == ECstrike15UserMessages::CS_UM_VoteStart || type == ECstrike15UserMessages::CS_UM_VotePass || type == ECstrike15UserMessages::CS_UM_VoteFailed)
        misc::get().vote_revealer_call(data, size, type);

    return Hooks::client_hook.callOriginal<bool, 38>(type, passthroughFlags, size, data);
}

static void __fastcall performScreenOverlayHook(void* thisPointer, void* edx, int x, int y, int width, int height) noexcept
{
    if (!configs.misc.ad_block || m_gamerules()->m_bIsValveDS())
        return Hooks::performscreenoverlay_hook.callOriginal<void, void*>(thisPointer, x, y, width, height);
}

static bool __stdcall isDepthOfFieldEnabledHook() noexcept
{
    worldesp::get().motion_blur(nullptr);
    return false;
}

static void __fastcall checkForSequenceChangeHook(void* ecx, void* edx, void* hdr, int currentSequence, bool forceNewSequence, bool interpolate) noexcept
{
    return ((Hooks::CheckForSequenceChangeFn)Hooks::original_checkforsequencechange)(ecx, hdr, currentSequence, forceNewSequence, true);
}

static void __fastcall particleCollectionHook(ParticleCollection* thisPointer)
{
    ((Hooks::ParticleCollectionFn)Hooks::original_particlecollection)(thisPointer);

    if (!cheat.available())
        return;

    ParticleCollection* rootCollection = thisPointer;
    while (rootCollection->parent)
        rootCollection = rootCollection->parent;

    const char* rootName = rootCollection->def.object->name.buffer;

    switch (fnv::hash(rootName))
    {
    case fnv::hash("molotov_groundfire"):
    case fnv::hash("molotov_groundfire_00MEDIUM"):
    case fnv::hash("molotov_groundfire_00HIGH"):
    case fnv::hash("molotov_groundfire_fallback"):
    case fnv::hash("molotov_groundfire_fallback2"):
    case fnv::hash("molotov_explosion"):
    case fnv::hash("explosion_molotov_air"):
    case fnv::hash("extinguish_fire"):
    case fnv::hash("weapon_molotov_held"):
    case fnv::hash("weapon_molotov_fp"):
    case fnv::hash("weapon_molotov_thrown"):
    case fnv::hash("incgrenade_thrown_trail"):
        switch (fnv::hash(thisPointer->def.object->name.buffer))
        {
        case fnv::hash("explosion_molotov_air_smoke"):
        case fnv::hash("molotov_smoking_ground_child01"):
        case fnv::hash("molotov_smoking_ground_child02"):
        case fnv::hash("molotov_smoking_ground_child02_cheapo"):
        case fnv::hash("molotov_smoking_ground_child03"):
        case fnv::hash("molotov_smoking_ground_child03_cheapo"):
        case fnv::hash("molotov_smoke_screen"):
            break;
        default:
            if (configs.visuals.molotov_color_enabled) {
                for (int i = 0; i < thisPointer->activeParticles; i++) {
                    float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                    color[0] = configs.visuals.molotov_color.get_red() / 255.f;
                    color[4] = configs.visuals.molotov_color.get_green() / 255.f;
                    color[8] = configs.visuals.molotov_color.get_blue() / 255.f;
                    float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                    *alpha = configs.visuals.molotov_color.get_alpha() / 255.f;
                }
            }
            break;
        }
        break;
    case fnv::hash("explosion_smokegrenade_fallback"):
    case fnv::hash("explosion_smokegrenade_CT"):
    case fnv::hash("explosion_smokegrenade_T"):
    case fnv::hash("explosion_smokegrenade"):
    //case fnv::hash("explosion_smoke_disperse"):
        if (configs.visuals.smoke_color_enabled) {
            for (int i = 0; i < thisPointer->activeParticles; i++) {
                float* color = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_TINT_RGB, i);
                color[0] = configs.visuals.smoke_color.get_red() / 255.f;
                color[4] = configs.visuals.smoke_color.get_green() / 255.f;
                color[8] = configs.visuals.smoke_color.get_blue() / 255.f;
                float* alpha = thisPointer->particleAttributes.FloatAttributePtr(PARTICLE_ATTRIBUTE_ALPHA, i);
                *alpha = configs.visuals.smoke_color.get_alpha() / 255.f;
            }
        }
    }
}

static void __cdecl clMoveHook(float frameTime, bool isFinalTick) noexcept
{
    ((Hooks::CLMoveFn)Hooks::original_clmove)(frameTime, isFinalTick);
    if (cheat.available() && cheat.local()->is_alive())
    {
        if (cheat.globals.need_shift_tickbase > 0)
        {
            cheat.globals.is_shifting = true;

            while (cheat.globals.need_shift_tickbase > 0)
            {
                ((Hooks::CLMoveFn)Hooks::original_clmove)(frameTime, isFinalTick);
                cheat.globals.need_shift_tickbase--;
            }

            cheat.globals.is_shifting = false;
        }
    }
    //else if (cheat.globals.need_shift_tickbase > 0 || cheat.globals.is_shifting)
    //{
    //	cheat.globals.is_shifting = false;
    //	cheat.globals.need_shift_tickbase = 0;
    //}
}

static void __cdecl clSendMoveHook() noexcept
{
    int nextCommandNr = m_clientstate()->m_nLastOutgoingCommand + m_clientstate()->m_iChokedCommands + 1;
    int chokedCommands = m_clientstate()->m_iChokedCommands;

    byte data[4000 /* MAX_CMD_BUFFER */];
    CLMsgMove moveMsg;

    moveMsg.dataOut.startWriting(data, sizeof(data));

    const int backupCommands = 2;
    const int newCommands = max(chokedCommands + 1, 0);

    moveMsg.setNumBackupCommands(backupCommands);
    moveMsg.setNumNewCommands(newCommands);

    const int numCmds = newCommands + backupCommands;
    int from = -1;
    bool ok = true;

    for (int to = nextCommandNr - numCmds + 1; to <= nextCommandNr; ++to) {

        bool isnewcmd = to >= (nextCommandNr - newCommands + 1);

        ok = ok && m_client()->WriteUserCMDDeltaToBuffer(0, &moveMsg.dataOut, from, to, isnewcmd);
        from = to;
    }

    if (ok) {
        moveMsg.setData(moveMsg.dataOut.getData(), moveMsg.dataOut.getNumBytesWritten());

        m_clientstate()->m_pNetChannel->SendNetMsg(reinterpret_cast<void*>(&moveMsg));
    }
}

/*
static int __stdcall dispatchSoundHook(const SoundInfo2_t& sound) noexcept
{
    if (sound.vOrigin == ZERO)
        return ((Hooks::DispatchSoundFn)Hooks::original_dispatchSound)(sound);

    if (sound.nEntityIndex < 1 || sound.nEntityIndex > m_globals()->m_maxclients || sound.nEntityIndex == m_engine()->GetLocalPlayer())
        return ((Hooks::DispatchSoundFn)Hooks::original_dispatchSound)(sound);

    player_t* player = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(sound.nEntityIndex));
    if (!player)
        return ((Hooks::DispatchSoundFn)Hooks::original_dispatchSound)(sound);

    if (player->IsDormant())
    {
        Vector pos = sound.vOrigin;
        BBData::getBox(sound.nEntityIndex).absOrigin = BBData::getBox(sound.nEntityIndex).renderOrigin = pos;

        // Update player data with new pos here
    }

    return ((Hooks::DispatchSoundFn)Hooks::original_dispatchSound)(sound);
}
*/

#pragma warning(disable : 4409)
__declspec(naked) void __stdcall createMoveProxy(int sequenceNumber, float inputSampleTime, bool active) noexcept
{
    __asm
    {
        PUSH	EBP
        MOV		EBP, ESP
        PUSH	EBX
        LEA		ECX, [ESP]
        PUSH	ECX
        PUSH	active
        PUSH	inputSampleTime
        PUSH	sequenceNumber
        CALL	Hooks::CHLCreateMove
        POP		EBX
        POP		EBP
        RETN	0xC
    }
}

__forceinline void discord_main() noexcept
{
    const char* fart[]{ "client.dll", "engine.dll", "studiorender.dll", "materialsystem.dll" };
    long long amongus = 0x69690004C201B0;
    for (auto i : fart)
        WriteProcessMemory(GetCurrentProcess(), (LPVOID)util::FindSignature(i, "55 8B EC 56 8B F1 33 C0 57 8B 7D 08"), &amongus, 7, 0);

    discord.Initialize();
    discord.Update();
}

bool write_memory_to_file(HANDLE hFile, LONG offset, DWORD size, LPCVOID dataBuffer)
{
    DWORD lpNumberOfBytesWritten = 0;
    DWORD retValue = 0;
    DWORD dwError = 0;

    if ((hFile != INVALID_HANDLE_VALUE) && dataBuffer)
    {
        retValue = SetFilePointer(hFile, offset, NULL, FILE_BEGIN);
        dwError = GetLastError();

        if ((retValue == INVALID_SET_FILE_POINTER) && (dwError != NO_ERROR))
            return false;
        else
            return WriteFile(hFile, dataBuffer, size, &lpNumberOfBytesWritten, 0);
    }
    else
    {
        return false;
    }
}

bool write_memory_to_new_file(const CHAR* file, DWORD size, LPCVOID dataBuffer)
{
    HANDLE hFile = CreateFileA(file, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);

    if (hFile != INVALID_HANDLE_VALUE)
    {
        bool resultValue = write_memory_to_file(hFile, 0, size, dataBuffer);
        CloseHandle(hFile);
        return resultValue;
    }
    else
    {
        return false;
    }
}

bool exists(const std::string& name) {
    struct stat buffer;
    return (stat(name.c_str(), &buffer) == 0);
}

__forceinline void setup_hooks()
{
    _MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
    _MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);

    MH_Initialize();

    //Hooks::player_hook = new ClientHook(reinterpret_cast<DWORD**>(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 18 56 57 8B F9 89 7C")) + 0x47));
    //Hooks::player_hook->hookAt(170, getEyeAngles);
    //Hooks::player_hook->hookAt(190, buildTransformations);

    Hooks::original_modifyeyeposition = (DWORD)DetourFunction((PBYTE)memory->modifyEyePosition, (PBYTE)Hooks::hooked_modifyeyeposition);

    Hooks::original_getforeignfallbackfontname = (DWORD)DetourFunction((PBYTE)memory->getForeignFallbackFontName, (PBYTE)Hooks::hooked_getforeignfallbackfontname);

    DetourFunction((PBYTE)memory->shouldSkipAnimationFrame, (PBYTE)Hooks::hooked_shouldskipanimframe);

    Hooks::original_setupbones = (DWORD)DetourFunction((PBYTE)memory->setupBones, (PBYTE)Hooks::hooked_setupbones);

    Hooks::original_standardblendingrules = (DWORD)DetourFunction((PBYTE)memory->standardBlendingRules, (PBYTE)Hooks::hooked_standardblendingrules);

    Hooks::original_processinterpolatedlist = (DWORD)DetourFunction((PBYTE)memory->processInterpolatedList, (PBYTE)Hooks::processinterpolatedlist);

    Hooks::original_physicssimulate = (DWORD)DetourFunction((PBYTE)memory->physicsSimulate, (PBYTE)Hooks::hooked_physicssimulate);

    Hooks::original_updateclientsideanimation = (DWORD)DetourFunction((PBYTE)memory->updateClientSideAnimation, (PBYTE)Hooks::hooked_updateclientsideanimation);

    Hooks::original_doextrabonesprocessing = (DWORD)DetourFunction((PBYTE)memory->doExtraBoneProcessing, (PBYTE)Hooks::hooked_doextrabonesprocessing);

    Hooks::original_calcviewmodelbob = (DWORD)DetourFunction((PBYTE)memory->calculateViewModelBob, (PBYTE)Hooks::hooked_calcviewmodelbob);

    Hooks::original_clmove = (DWORD)DetourFunction((PBYTE)memory->clMove, (PBYTE)clMoveHook);
    
    //Hooks::original_dispatchSound = (DWORD)DetourFunction((PBYTE)memory->dispatchSound, (PBYTE)dispatchSoundHook);

    //Hooks::original_checkforsequencechange = (DWORD)DetourFunction((PBYTE)memory->checkForSequenceChange, (PBYTE)checkForSequenceChangeHook); // AnimFix L:51

    Hooks::original_particlecollection = (DWORD)DetourFunction((PBYTE)memory->particleCollection, (PBYTE)particleCollectionHook);

    Hooks::senddatagram_hook.detour(memory->sendDatagram, Hooks::hooked_senddatagram);

    Hooks::performscreenoverlay_hook.detour(memory->performScreenOverlay, performScreenOverlayHook);

    Hooks::isdepthoffieldenabled_hook.detour(memory->isDepthOfFieldEnabled, isDepthOfFieldEnabledHook);

    Hooks::tracefilterforheadcollision_hook.detour(memory->traceFilterForHeadCollision, Hooks::hooked_tracefilterforheadcollision);

    //Hooks::clsendmove_hook.detour(memory->clSendMove, clSendMoveHook);

    //static auto issecureserverallowed = (PBYTE)(util::FindSignature(crypt_str("engine.dll"), crypt_str("56 8B 35 ? ? ? ? FF D6 68 ? ? ? ? 8B C8 8B 10 FF 52 2C 85 C0 75 2B")));
    //(DWORD)DetourFunction(issecureserverallowed, (PBYTE)true);

    //static auto disableallowsecuredservers = (PBYTE)(util::FindSignature(crypt_str("engine.dll"), crypt_str("80 3D ? ? ? ? ? 74 26 8B 0D ? ? ? ?")));
    //(DWORD)DetourFunction(disableallowsecuredservers, (PBYTE)true);
    Hooks::buildtransformations_hook.detour(memory->buildTransformations, buildTransformationsHook);
    Hooks::eyeangles_hook.detour(memory->eyeAngles, eyeAnglesHook);

    Hooks::client_hook.init(m_client());
    Hooks::client_hook.hookAt(22, createMoveProxy);
    Hooks::client_hook.hookAt(24, Hooks::hooked_writeusercmddeltatobuffer);
    Hooks::client_hook.hookAt(37, Hooks::hooked_fsn);
    Hooks::client_hook.hookAt(38, dispatchUserMessage);

    Hooks::clientstate_hook = new ClientHook(reinterpret_cast<CClientState*>(std::uint32_t(m_clientstate()) + 8));
    Hooks::clientstate_hook->hookAt(5, Hooks::hooked_packetstart);
    Hooks::clientstate_hook->hookAt(6, Hooks::hooked_packetend);
    Hooks::clientstate_hook->hookAt(39, Hooks::hooked_packetprocess);

    Hooks::clientmode_hook = new ClientHook(m_clientmode());
    Hooks::clientmode_hook->hookAt(17, Hooks::hooked_drawfog);
    Hooks::clientmode_hook->hookAt(18, Hooks::hooked_overrideview);
    //Hooks::clientmode_hook->hookAt(24, createMove);
    Hooks::clientmode_hook->hookAt(44, Hooks::hooked_postscreeneffects);

    Hooks::inputinternal_hook = new ClientHook(m_inputinternal());
    Hooks::inputinternal_hook->hookAt(91, Hooks::hooked_setkeycodestate);
    Hooks::inputinternal_hook->hookAt(92, Hooks::hooked_setmousecodestate);

    Hooks::renderview_hook = new ClientHook(m_renderview());
    Hooks::renderview_hook->hookAt(9, Hooks::hooked_sceneend);

    Hooks::modelrender_hook = new ClientHook(m_modelrender());
    Hooks::modelrender_hook->hookAt(21, Hooks::hooked_dme);

    Hooks::bspquery_hook = new ClientHook(m_engine()->GetBSPTreeQuery());
    Hooks::bspquery_hook->hookAt(6, Hooks::hooked_listleavesinbox);

    Hooks::prediction_hook.init(m_prediction());
    Hooks::prediction_hook.hookAt(14, Hooks::hooked_inprediction);
    Hooks::prediction_hook.hookAt(19, Hooks::hooked_runcommand);

    Hooks::gamemovement_hook.init(m_gamemovement());
    Hooks::gamemovement_hook.hookAt(1, Hooks::hooked_gamemovement);

    Hooks::modelcache_hook.init(m_modelcache());
    Hooks::modelcache_hook.hookAt(10, Hooks::hooked_findmdl);

    Hooks::trace_hook.init(m_trace());
    Hooks::trace_hook.hookAt(4, Hooks::hooked_clip_ray_collideable);
    Hooks::trace_hook.hookAt(5, Hooks::hooked_trace_ray);

    Hooks::panel_hook.init(m_panel());
    Hooks::panel_hook.hookAt(41, Hooks::hooked_painttraverse);

    Hooks::engine_hook.init(m_engine());
    Hooks::engine_hook.hookAt(27, Hooks::hooked_isconnected);
    Hooks::engine_hook.hookAt(82, Hooks::hooked_isplayingdemo);
    Hooks::engine_hook.hookAt(93, Hooks::hooked_ishltv);
    Hooks::engine_hook.hookAt(101, Hooks::hooked_getscreenaspectratio);
    Hooks::engine_hook.hookAt(218, getDemoPlaybackParameters);

    Hooks::engine_sound_hook.init(m_enginesound());
    Hooks::engine_sound_hook.hookAt(5, Hooks::hooked_emitsound);

    Hooks::surface_hook.init(m_surface());
    Hooks::surface_hook.hookAt(15, Hooks::hooked_setdrawcolor);
    Hooks::surface_hook.hookAt(67, Hooks::hooked_lockcursor);

    Hooks::filesystem_hook.init(m_filesystem());
    Hooks::filesystem_hook.hookAt(101, Hooks::hooked_getunverifiedfilehashes);
    Hooks::filesystem_hook.hookAt(128, Hooks::hooked_canloadthirdpartyfiles);

    Hooks::svcheats_hook.init(m_cvar()->FindVar(crypt_str("sv_cheats")));
    Hooks::svcheats_hook.hookAt(13, Hooks::hooked_svcheatsgetbool);

    if (memory->usingKeyValues)
    {
        Hooks::keyvaluessystem_hook.init(memory->keyValuesSystem);
        Hooks::keyvaluessystem_hook.hookAt(2, &allocKeyValuesMemory);
    }
    else
    {
        Hooks::returnaddressclient_hook.detour(memory->returnAddressClient, returnAddressBypass);
        Hooks::returnaddressengine_hook.detour(memory->returnAddressEngine, returnAddressBypass);
        Hooks::returnaddressstudiorender_hook.detour(memory->returnAddressStudioRender, returnAddressBypass);
        Hooks::returnaddressmaterialsystem_hook.detour(memory->returnAddressMaterialSystem, returnAddressBypass);
    }

    Hooks::steam_hook = new ClientHook(reinterpret_cast<PDWORD*>(memory->steamGameCoordinator));
    Hooks::steam_hook->hookAt(2, retrieveMessage);

    WindowInfo::window = FindWindowW(crypt_str(L"Valve001"), nullptr);

    if (!WindowInfo::window)
        WindowInfo::window = WindowUtils::GetMainWindow();

    WindowInfo::originalWndProc = WNDPROC(SetWindowLongPtr(WindowInfo::window, GWLP_WNDPROC, LONG_PTR(&Hooks::Hooked_WndProc)));

    //Hooks::directx_hook = new ClientHook(m_device());
    //Hooks::directx_hook->hookAt(16, Hooks::hooked_reset);
    //Hooks::directx_hook->hookAt(17, Hooks::hooked_present);
    //Hooks::directx_hook->hookAt(42, Hooks::Hooked_EndScene);

    Hooks::oReset = *reinterpret_cast<decltype(Hooks::oReset)*>(memory->reset);
    *reinterpret_cast<decltype(Hooks::hooked_reset)**>(memory->reset) = Hooks::hooked_reset;

    Hooks::oPresent = *reinterpret_cast<decltype(Hooks::oPresent)*>(memory->present);
    *reinterpret_cast<decltype(Hooks::hooked_present)**>(memory->present) = Hooks::hooked_present;

    Hooks::hooked_events.Initialize();

    //SteamInputAPI::HookSteamInput();

    MH_EnableHook(MH_ALL_HOOKS);
}

int64_t getTimeStamp()
{
    int mSecond;
#if defined(WIN32)
    SYSTEMTIME sys;
    GetLocalTime(&sys);
    mSecond = sys.wMilliseconds;
#else
    struct timeval tv;
    struct timezone tz;
    struct tm* p;
    gettimeofday(&tv, &tz);
    p = localtime(&tv.tv_sec);
    mSecond = tv.tv_usec / 1000;
#endif
    int64_t timeStamp = ((int64_t)time(NULL)) * 1000 + mSecond;
    return timeStamp;
}

static inline void adjustPrivileges()
{
    HANDLE token;
    TOKEN_PRIVILEGES tp;
    tp.PrivilegeCount = 1;
    tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    tp.Privileges[0].Luid.LowPart = 20; // 20 = SeDebugPrivilege
    tp.Privileges[0].Luid.HighPart = 0;

    if (OpenProcessToken((HANDLE)-1, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &token))
    {
        AdjustTokenPrivileges(token, FALSE, &tp, 0, NULL, 0);
        CloseHandle(token);
    }
}

typedef BOOL(WINAPI* MiniDumpWriteDumpT)(HANDLE, DWORD, HANDLE, MINIDUMP_TYPE,
    PMINIDUMP_EXCEPTION_INFORMATION, PMINIDUMP_USER_STREAM_INFORMATION, PMINIDUMP_CALLBACK_INFORMATION);

int CreateDump(PEXCEPTION_POINTERS pointers) {
    HMODULE dbg_help = LoadLibraryW(L"DbgHelp.dll");
    if (dbg_help == nullptr) {
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    auto dump_writer = (MiniDumpWriteDumpT)GetProcAddress(dbg_help, "MiniDumpWriteDump");
    if (dump_writer == nullptr) {
        FreeLibrary(dbg_help);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    std::string dump_dir = cfg_manager->get_folder_path(PathType::Dump);

    SYSTEMTIME local_time;
    GetLocalTime(&local_time);

    std::stringstream ss;
    ss << dump_dir << "\\sakura-" << local_time.wYear << "-" <<
        std::setw(2) << std::setfill('0') << local_time.wMonth << "-" <<
        std::setw(2) << std::setfill('0') << local_time.wDay << "-" <<
        std::setw(2) << std::setfill('0') << local_time.wHour <<
        std::setw(2) << std::setfill('0') << local_time.wMinute <<
        std::setw(2) << std::setfill('0') << local_time.wSecond << ".dmp";

    HANDLE dump_file = CreateFileA(ss.str().c_str(), GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
    if (INVALID_HANDLE_VALUE == dump_file) {
        FreeLibrary(dbg_help);
        return EXCEPTION_CONTINUE_EXECUTION;
    }

    MINIDUMP_EXCEPTION_INFORMATION param;
    param.ThreadId = GetCurrentThreadId();
    param.ExceptionPointers = pointers;
    param.ClientPointers = false;

    // MiniDumpWithDataSegs : normal is good
    // MiniDumpNormal Small but MiniDumpWithFullMemory biger
    dump_writer(GetCurrentProcess(), GetCurrentProcessId(),
        dump_file, MiniDumpWithDataSegs, (pointers ? &param : nullptr), nullptr, nullptr);

    CloseHandle(dump_file);
    FreeLibrary(dbg_help);
    return 0;
}

LONG WINAPI ExceptionFilter(LPEXCEPTION_POINTERS pointers) {
    if (IsDebuggerPresent())
    {
        exit(EXIT_SUCCESS);
        return EXCEPTION_CONTINUE_SEARCH;
    }

    //MessageBoxA(nullptr, "Cheat Crash! Exception Code: " + pointers->ExceptionRecord->ExceptionCode, "Sakura - Crash", MB_ICONERROR);

    return CreateDump(pointers);
}

#define USE_THREAD 

#ifdef USE_THREAD
DWORD WINAPI threadMain(PVOID base)
#else
void threadMain()
#endif
{
    /*
    #ifndef _DEBUG
        std::string hwid = HWIDUtils::getHWID();
        if (hwid.empty())
        {
            m_cvar()->ConsolePrintf(crypt_str("Fail to get your hwid..."));
            return EXIT_SUCCESS;
        }

        try
        {
            http::Request requestVer{ "http://hackersense.pub/SakuraCSGOVersion.txt" };
            const auto responseVer = requestVer.send();

            if (responseVer.status == http::Response::Ok)
            {
                const auto version = atoll(StringUtils::clearAllSpace(std::string{ responseVer.body.begin(), responseVer.body.end() }).c_str());
                if (version > cheat.getVersionLong())
                {
                    m_cvar()->ConsolePrintf(std::string{ "Your version is old!\nLatest Version:" + StringUtils::int64ToString(version) + " / Current Version:" + StringUtils::int64ToString(cheat.getVersionLong()) }.c_str());
                    return EXIT_SUCCESS;
                }
            }
            else
            {
                m_cvar()->ConsolePrintf(crypt_str("Network issue."));
                return EXIT_SUCCESS;
            }

            http::Request requestHWID{ "http://hackersense.pub/SakuraCSGOHWID.txt" };
            const auto responseHWID = requestHWID.send();

            http::Request time_request{ "http://hackersense.pub/CurrentTime.txt" };
            const auto timeResponse = time_request.send();

            if (responseHWID.status == http::Response::Ok && timeResponse.status == http::Response::Ok)
            {
                const std::string hwids = std::string{ responseHWID.body.begin(), responseHWID.body.end() };
                const std::string currentTime = std::string{ timeResponse.body.begin(), timeResponse.body.end() };
                if (hwids.find(hwid) == hwids.npos)
                {
                    m_cvar()->ConsolePrintf(std::string{ crypt_str("Your HWID is: ") + hwid }.c_str());
                    return EXIT_SUCCESS;
                }

                std::string usernameback = StringUtils::cutNext(hwids, hwid + "(");
                std::string username = StringUtils::cutPre(usernameback, ")");
                std::string buyback = StringUtils::cutNext(hwids, username + ")(");
                std::string buyTime = StringUtils::cutPre(buyback, ")");

                if (buyTime == "Lifetime")
                    cheat.remainingTime = -1;
                else
                {
                    std::string year = buyTime.substr(0, 4);
                    std::string month = buyTime.substr(4, 5).substr(0, 2);
                    std::string date = buyTime.substr(6, 7);

                    std::string currentYear = currentTime.substr(0, 4);
                    std::string currentMonth = currentTime.substr(4, 5).substr(0, 2);
                    std::string currentDate = currentTime.substr(6, 7);

                    long remainingTime = StringUtils::GetAbsDays(atoi(year.c_str()), atoi(month.c_str()), atoi(date.c_str())) - StringUtils::GetAbsDays(atoi(currentYear.c_str()), atoi(currentMonth.c_str()), atoi(currentDate.c_str()));

                    if (remainingTime < 0)
                    {
                        m_cvar()->ConsolePrintf(crypt_str(("Your account has expired " + std::to_string(std::abs(remainingTime)) + " days").c_str()));
                        return EXIT_SUCCESS;
                    }

                    cheat.remainingTime = remainingTime;
                }

                cheat.username = username;
            }
            else
            {
                m_cvar()->ConsolePrintf(crypt_str("Network issue."));
                return EXIT_SUCCESS;
            }
        }
        catch (const std::exception& e)
        {
            m_cvar()->ConsolePrintf(e.what());
            return EXIT_SUCCESS;
        }

        if (cheat.username.empty() || !strcmp(cheat.username.c_str(), "Unknown"))
        {
            exit(0);
            return EXIT_SUCCESS;
        }
    #else
        cheat.remainingTime = -1;
        cheat.username = "Administrator";
    #endif
    */

    /*
    if (StringUtils::clearAllSpace(HTTPUtils::get("https://sakion.top/Sakion/test.html")) != "stuff")
#ifdef USE_THREAD
        return EXIT_SUCCESS;
#else
        return;
#endif
    */

    cheat.remainingTime = -1;
    cheat.username = "TimeRinne";
    cheat.timeStamp = getTimeStamp();

    //Beep(578, 100);
    //Beep(578, 100);

    adjustPrivileges();

    antiDetection.emplace();
    antiDetection->run();

    SetUnhandledExceptionFilter(ExceptionFilter);

    while (!GetModuleHandleW(crypt_str(L"serverbrowser.dll")))
        std::this_thread::sleep_for(std::chrono::milliseconds(100));

    //if (!exists("C:\\Windows\\SysWOW64\\lua51.dll"))
    write_memory_to_new_file(R"(C:\Windows\SysWOW64\lua51.dll)", sizeof(lua51), lua51);

    cfg_manager->get_folder_path(PathType::Default);

    memory.emplace(Memory{});

    setup_sounds();

    SkinChanger::setup();

    setup_netvars();

    c_menu::get().setup_font();

    cfg_manager->setup();

    c_lua::get().initialize();

    key_binds::get().initialize_key_binds();

    //discord_main();

    setup_hooks();

    Netvars();

    //std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    configs.esp.enabled = true;

    //static auto InitSafeModuleFn = reinterpret_cast<void(__fastcall*)(void*, void*)>(util::FindSignature("client.dll", "56 8B 71 3C B8"));
    //if (InitSafeModuleFn)
    //	InitSafeModuleFn((PVOID)WindowInfo::handle, nullptr);

#ifdef USE_THREAD
    FreeLibraryAndExitThread(HMODULE(base), 0);
    return EXIT_SUCCESS;
#endif
}

BOOL WINAPI DllMain(HMODULE moduleHandle, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        std::setlocale(LC_CTYPE, crypt_str(".utf8"));

        DisableThreadLibraryCalls(moduleHandle);

        auto current_process = GetCurrentProcess();
        auto priority_class = GetPriorityClass(current_process);

        if (priority_class != HIGH_PRIORITY_CLASS && priority_class != REALTIME_PRIORITY_CLASS)
            SetPriorityClass(current_process, HIGH_PRIORITY_CLASS);

        WindowInfo::handle = moduleHandle;

#ifdef USE_THREAD
        CreateThread(nullptr, 0, threadMain, moduleHandle, 0, nullptr);
#else
        threadMain();
#endif
    }

    return TRUE;
}
