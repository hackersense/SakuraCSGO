#include "Memory.h"
#include "..\includes.hpp"

template <typename T>
static constexpr auto relativeToAbsolute(uintptr_t address) noexcept
{
    return (T)(address + 4 + *reinterpret_cast<std::int32_t*>(address));
}

Memory::Memory() noexcept
{
    //if (!GetModuleHandleW(crypt_str(L"discordhook")))
    //{
    reset = *reinterpret_cast<std::uintptr_t*>(findPattern(crypt_str("gameoverlayrenderer"), "\x57\x53\xC7\x45") + 11);
    present = reset + 4;
    //}
    //else
    //{
    //    present = *reinterpret_cast<std::uintptr_t*>(findPattern(crypt_str("discordhook"), "\x83\xC4\x0C\xFF\x75\x18") + 14);
    //    reset = present + 60;
    //}

    auto temp = reinterpret_cast<std::uintptr_t*>(findPattern(crypt_str("client"), "\xB9????\xE8????\x8B\x5D\x08") + 1);
    hud = *temp;
    findHudElement = relativeToAbsolute<decltype(findHudElement)>(reinterpret_cast<uintptr_t>(temp) + 5);
    loadSky = relativeToAbsolute<decltype(loadSky)>(findPattern(crypt_str("engine"), "\xE8????\x84\xC0\x74\x2D\xA1") + 1);
    registeredPanoramaEvents = reinterpret_cast<decltype(registeredPanoramaEvents)>(*reinterpret_cast<std::uintptr_t*>(findPattern(crypt_str("client"), "\xE8????\xA1????\xA8\x01\x75\x21") + 6) - 36);
    makePanoramaSymbolFn = relativeToAbsolute<decltype(makePanoramaSymbolFn)>(findPattern(crypt_str("client"), "\xE8????\x0F\xB7\x45\x0E\x8D\x4D\x0E") + 1);
    submitReportFunction = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x28\x8B\x4D\x08");
    demoFileEndReached = findPattern(crypt_str("client"), "\x8B\xC8\x85\xC9\x74\x1F\x80\x79\x10");
    demoOrHLTV = findPattern(crypt_str("client"), "\x84\xC0\x75\x09\x38\x05");
    money = findPattern(crypt_str("client"), "\x84\xC0\x75\x0C\x5B");
    inventoryAccess = findPattern(crypt_str("client"), "\x84\xC0\x75\x05\xB0\x01\x5F");
    clearHudWeapon = relativeToAbsolute<decltype(clearHudWeapon)>(findPattern(crypt_str("client"), crypt_str("\xE8????\x8B\xF0\xC6\x44\x24??\xC6\x44\x24")) + 1);
    itemSystem = relativeToAbsolute<decltype(itemSystem)>(findPattern(crypt_str("client"), crypt_str("\xE8????\x0F\xB7\x0F")) + 1);
    setAbsOrigin = relativeToAbsolute<decltype(setAbsOrigin)>(findPattern(crypt_str("client"), "\xE8????\xEB\x19\x8B\x07") + 1);
    setAbsAngles = reinterpret_cast<decltype(setAbsAngles)>(reinterpret_cast<DWORD*>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x64\x53\x56\x57\x8B\xF1")));
    insertIntoTree = findPattern(crypt_str("client"), "\x56\x52\xFF\x50\x18") + 5;
    scopeDust = findPattern(crypt_str("client"), "\xFF\x50\x3C\x8B\x4C\x24\x20") + 3;
    scopeArc = findPattern(crypt_str("client"), "\x8B\x0D????\xFF\xB7????\x8B\x01\xFF\x90????\x8B\x7C\x24\x1C");
    equipWearable = reinterpret_cast<decltype(equipWearable)>(findPattern(crypt_str("client"), crypt_str("\x55\x8B\xEC\x83\xEC\x10\x53\x8B\x5D\x08\x57\x8B\xF9")));
    setClanTagFn = reinterpret_cast<decltype(setClanTagFn)>(findPattern(crypt_str("engine"), "\x53\x56\x57\x8B\xDA\x8B\xF9\xFF\x15"));
    seqdesc = relativeToAbsolute<decltype(seqdesc)>(findPattern(crypt_str("client"), "\xE8????\x03\x40\x04") + 1);
    lineGoesThroughSmoke = relativeToAbsolute<decltype(lineGoesThroughSmoke)>(findPattern(crypt_str("client"), "\xE8????\x8B\x4C\x24\x30\x33\xD2") + 1);
    isBreakableEntityFn = reinterpret_cast<decltype(isBreakableEntityFn)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x51\x56\x8B\xF1\x85\xF6\x74\x68"));
    //clipRayToHixbox = reinterpret_cast<decltype(clipRayToHixbox)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\xF3????\x81?????\x0F"));
    clipRayToHixbox = reinterpret_cast<decltype(clipRayToHixbox)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\xF3\x0F\x10\x42"));
    predictionRandomSeed = *reinterpret_cast<int**>(findPattern(crypt_str("client"), "\x8B\x0D????\xBA????\xE8????\x83\xC4\x04") + 2);
    predictionPlayer = *reinterpret_cast<int**>(findPattern(crypt_str("client"), "\x89\x35????\xF3\x0F\x10\x48") + 2);
    traceToExit = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xEC\x4C\xF3\x0F\x10\x75");
    drawScreenEffectMaterial = relativeToAbsolute<uintptr_t>(findPattern(crypt_str("client"), "\xE8????\x83\xC4\x0C\x8D\x4D\xF8") + 1);
    isDepthOfFieldEnabled = findPattern(crypt_str("client"), "\x8B\x0D????\x56\x8B\x01\xFF\x50\x34\x8B\xF0\x85\xF6\x75\x04");
    cameraThink = findPattern(crypt_str("client"), "\x85\xC0\x75\x30\x38\x87");
    sendDatagram = findPattern(crypt_str("engine"), "\x55\x8B\xEC\x83\xE4\xF0\xB8????\xE8????\x56\x57\x8B\xF9\x89\x7C\x24\x14");
    getFirstSequenceAnimTag = relativeToAbsolute<decltype(getFirstSequenceAnimTag)>(findPattern(crypt_str("client"), "\xE8????\xF3\x0F\x11\x86????\x0F\x57\xDB") + 1);
    fakePrime = reinterpret_cast<std::uint8_t*>(findPattern(crypt_str("client"), "??????\x85\xC9\x75\x04\x33\xC0\xEB\x1E"));
    sequenceDuration = relativeToAbsolute<decltype(sequenceDuration)>(findPattern(crypt_str("client"), "\xE8????\xF3\x0F\x11\x86????\x51") + 1);
    clSendMove = findPattern(crypt_str("engine"), "\x55\x8B\xEC\x8B\x4D\x04\x81\xEC????");
    clMsgMoveSetData = relativeToAbsolute<decltype(clMsgMoveSetData)>(findPattern(crypt_str("engine"), "\xE8????\x8D\x7E\x18") + 1);
    clMsgMoveDescontructor = relativeToAbsolute<decltype(clMsgMoveDescontructor)>(findPattern(crypt_str("engine"), "\xE8????\x5F\x5E\x5B\x8B\xE5\x5D\xC3\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\xCC\x55\x8B\xEC\x81\xEC????") + 1);
    clMove = findPattern(crypt_str("engine"), "\x55\x8B\xEC\x81\xEC????\x53\x56\x8A\xF9");
    chokeLimit = findPattern(crypt_str("engine"), "\xB8????\x3B\xF0\x0F\x4F\xF0\x89\x5D\xFC") + 1;
    //traceFilterForHeadCollision = findPattern(crypt_str("client"), "\x55\x8B\xEC\x56\x8B\x75\x0C\x57\x8B\xF9\xF7\xC6????");
    //modifyEyePosition = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x70\x56\x57\x8B\xF9\x89\x7C\x24\x14\x83\x7F\x60");
    modifyEyePosition = relativeToAbsolute<decltype(modifyEyePosition)>(findPattern(crypt_str("client"), "\xE8????\x8B\x06\x8B\xCE\xFF\x90????\x85\xC0\x74\x50") + 1);
    calculateView = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xEC\x14\x53\x56\x57\xFF\x75\x18");
    checkForSequenceChange = findPattern(crypt_str("client"), "\x55\x8B\xEC\x51\x53\x8B\x5D\x08\x56\x8B\xF1\x57\x85");
    particleCollection = relativeToAbsolute<decltype(particleCollection)>(findPattern(crypt_str("client"), "\xE8????\x8B\x0E\x83\xC1\x10") + 1);
    physicsSimulate = findPattern(crypt_str("client"), "\x56\x8B\xF1\x8B?????\x83\xF9\xFF\x74\x23");
    getForeignFallbackFontName = findPattern(crypt_str("vguimatsurface"), "\x80\x3D?????\x74\x06\xB8");
    processInterpolatedList = findPattern(crypt_str("client"), "\x53\x0F\xB7\x1D????\x56");
    dispatchSound = findPattern(crypt_str("engine"), "\x55\x8B\xEC\x81\xEC????\x56\x8B\xF1\x8D\x4D\x98\xE8");
    //standardBlendingRules = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF0\xB8????\xE8????\x56\x8B\x75\x08\x57\x8B\xF9\x85\xF6");
    standardBlendingRules = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF0\xB8\xF8\x10");
    doExtraBoneProcessing = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x81\xEC????\x53\x56\x8B\xF1\x57\x89\x74\x24\x1C\x80");
    shouldSkipAnimationFrame = findPattern(crypt_str("client"), "\x57\x8B\xF9\x8B\x07\x8B\x80????\xFF\xD0\x84\xC0\x75\x02");
    updateClientSideAnimation = findPattern(crypt_str("client"), "\x55\x8B\xEC\x51\x56\x8B\xF1\x80\xBE?????\x74\x36");
    eyePositionAndVectors = findPattern(crypt_str("client"), "\x8B\x55\x0C\x8B\xC8\xE8????\x83\xC4\x08\x5E\x8B\xE5");
    buildTransformations = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF0\x81\xEC????\x56\x57\x8B\xF9\x8B\x0D????\x89\x7C\x24\x28\x8B");
    eyeAngles = findPattern(crypt_str("client"), "\x56\x8B\xF1\x85\xF6\x74\x32");
    calculateViewModelBob = findPattern(crypt_str("client"), "\x55\x8B\xEC\xA1????\x83\xEC\x10\x56\x8B\xF1\xB9");
    createState = findPattern(crypt_str("client"), "\x55\x8B\xEC\x56\x8B\xF1\xB9????\xC7");
    updateState = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x18\x56\x57\x8B\xF9\xF3");
    resetState = findPattern(crypt_str("client"), "\x56\x6A\x01\x68????\x8B\xF1");
    invalidateBoneCache = findPattern(crypt_str("client"), "\x80\x3D?????\x74\x16\xA1????\x48\xC7\x81");
    relayCluster = *reinterpret_cast<std::string**>(findPattern(crypt_str("steamnetworkingsockets"), "\xB8????\xB9????\x0F\x43") + 1);
    lookUpBone = reinterpret_cast<decltype(lookUpBone)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x53\x56\x8B\xF1\x57\x83\xBE?????\x75\x14"));
    setupBones = findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF0\xB8\xD8");
    restoreEntityToPredictedFrame = reinterpret_cast<decltype(restoreEntityToPredictedFrame)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x8B\x4D?\x56\xE8????\x8B\x75"));
    physicsRunThink = reinterpret_cast<decltype(physicsRunThink)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xEC\x10\x53\x56\x57\x8B\xF9\x8B\x87"));
    checkHasThinkFunction = reinterpret_cast<decltype(checkHasThinkFunction)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x56\x57\x8B\xF9\x8B\xB7????\x8B\xC6\xC1\xE8"));
    postThinkVPhysics = reinterpret_cast<decltype(postThinkVPhysics)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x83\xE4\xF8\x81\xEC????\x53\x8B\xD9\x56\x57\x83\xBB?????\x0F\x84"));
    simulatePlayerSimulatedEntities = reinterpret_cast<decltype(simulatePlayerSimulatedEntities)>(findPattern(crypt_str("client"), "\x56\x8B\xF1\x57\x8B\xBE????\x83\xEF\x01\x78\x74"));
    fs_tesla = reinterpret_cast<decltype(fs_tesla)>(findPattern(crypt_str("client"), "\x55\x8B\xEC\x81\xEC????\x56\x57\x8B\xF9\x8B\x47\x18"));
    keyValuesFromString = relativeToAbsolute<decltype(keyValuesFromString)>(findPattern(crypt_str("client"), "\xE8????\x83\xC4\x04\x89\x45\xD8") + 1);
    keyValuesFindKey = relativeToAbsolute<decltype(keyValuesFindKey)>(findPattern(crypt_str("client"), "\xE8????\xF7\x45") + 1);
    keyValuesSetString = relativeToAbsolute<decltype(keyValuesSetString)>(findPattern(crypt_str("client"), "\xE8????\x89\x77\x38") + 1);
    keyValuesSystem = reinterpret_cast<IKeyValuesSystem*(__stdcall*)()>(GetProcAddress(GetModuleHandleW(L"vstdlib"), "KeyValuesSystem"))();
    static auto engine = findPattern<false>(crypt_str("engine"), "\xE8????\x83\xC4\x08\x84\xC0\x75\x10\xFF\x75\x0C");
    static auto client = findPattern<false>(crypt_str("client"), "\xE8????\x83\xC4\x08\x84\xC0\x75\x10");
    if (!engine && !client)
    {
        usingKeyValues = false;
        returnAddressClient = findPattern(crypt_str("client"), "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08");
        returnAddressEngine = findPattern(crypt_str("engine"), "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08");
        returnAddressStudioRender = findPattern(crypt_str("studiorender"), "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08");
        returnAddressMaterialSystem = findPattern(crypt_str("materialsystem"), "\x55\x8B\xEC\x56\x8B\xF1\x33\xC0\x57\x8B\x7D\x08");
    }
    else
    {
        keyValuesAllocEngine = relativeToAbsolute<std::uintptr_t>(engine + 1) + 0x4A;
        keyValuesAllocClient = relativeToAbsolute<std::uintptr_t>(client + 1) + 0x3E;
    }
    jmpEbxGadgetInClient = findPattern(crypt_str("client"), "\x1B\xFF\x23\xF8\xF6\x87") + 1;
    shouldDrawFogReturnAddress = relativeToAbsolute<std::uintptr_t>(findPattern(crypt_str("client"), "\xE8????\x8B\x0D????\x0F\xB6\xD0") + 1) + 82;
    performScreenOverlay = findPattern(crypt_str("client"), "\x55\x8B\xEC\x51\xA1????\x53\x56\x8B\xD9");
    memalloc = *reinterpret_cast<MemAlloc**>(GetProcAddress(GetModuleHandleW(L"tier0"), "g_pMemAlloc"));
    SteamClient* steamClient = ((SteamClient * (__cdecl*)(void))GetProcAddress(GetModuleHandleW(L"steam_api"), "SteamClient"))();
    steamGameCoordinator = (SteamGameCoordinator*)steamClient->getGenericInterface((void*)1, (void*)1, "SteamGameCoordinator001");
    //cheat.local(reinterpret_cast<player_t*>(findPattern(crypt_str("client"), crypt_str("\xA1????\x89\x45\xBC\x85\xC0")) + 1));

    createSimpleThread = reinterpret_cast<decltype(createSimpleThread)>(GetProcAddress(GetModuleHandleW(L"tier0.dll"), "CreateSimpleThread"));
    releaseThreadHandle = reinterpret_cast<decltype(releaseThreadHandle)>(GetProcAddress(GetModuleHandleW(L"tier0.dll"), "ReleaseThreadHandle"));
}
