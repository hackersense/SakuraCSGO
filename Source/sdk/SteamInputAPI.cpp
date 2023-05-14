#include "SteamInputAPI.h"

#include "../includes.hpp"
#include "../steam_sdk/isteamclient.h"
#include "../steam_sdk/isteaminput.h"

// Steam api sucks!
struct SteamInputInner
{
    void* vtable;
    void* blank;
    void* blank2;
    SteamInputActionEventCallbackPointer actionEventCallback;
};
struct SteamInputHack
{
    void* vtable;
    SteamInputInner* inner;
};

typedef void (*RunFrameFn)(ISteamInput*, bool);
SteamInputActionEventCallbackPointer originalActionEventCallbackPointer;
static ISteamInput* steamInput = nullptr;
void SteamInputActionEventCallback(SteamInputActionEvent_t* event)
{
    if (event->eEventType == ESteamInputActionEventType_DigitalAction)
    {
        m_cvar()->ConsoleDPrintf("Input: (%s)\n", steamInput->GetStringForDigitalActionName(event->digitalAction.actionHandle));
    }
    // Call the original csgo controller callback
    originalActionEventCallbackPointer(event);
}


void SteamInputAPI::HookSteamInput() {
    typedef ISteamClient* (*GetSteamClientFn)();
    GetSteamClientFn GetSteamClientFunc = (GetSteamClientFn)(GetProcAddress(GetModuleHandleA("steam_api"), "SteamClient"));
    typedef HSteamPipe(*GetSteamPipeFn)();
    GetSteamPipeFn GetSteamPipeFunc = (GetSteamPipeFn)(GetProcAddress(GetModuleHandleA("steam_api"), "GetHSteamPipe"));

    ISteamClient* steamClient = GetSteamClientFunc();
    HSteamPipe steamPipe = GetSteamPipeFunc();
    HSteamUser steamUser = steamClient->ConnectToGlobalUser(steamPipe);

    typedef void* (*SteamFindOrCreateInterfaceFn)(HSteamUser, const char*);
    SteamFindOrCreateInterfaceFn SteamFindOrCreateInterfaceFunc = (SteamFindOrCreateInterfaceFn)(GetProcAddress(GetModuleHandleA("steam_api"), "SteamInternal_FindOrCreateUserInterface"));

    steamInput = (ISteamInput*)SteamFindOrCreateInterfaceFunc(steamUser, STEAMINPUT_INTERFACE_VERSION);

    // god-tier hack, Steam doesn't expose the original Callback and CSGO uses it
    SteamInputHack* hack = (SteamInputHack*)steamInput;
    originalActionEventCallbackPointer = hack->inner->actionEventCallback;
    steamInput->EnableActionEventCallbacks(SteamInputActionEventCallback);
}

void SteamInputAPI::UnhookSteamInput() {
    if (steamInput)
    {
        steamInput->EnableActionEventCallbacks(originalActionEventCallbackPointer);
    }
}