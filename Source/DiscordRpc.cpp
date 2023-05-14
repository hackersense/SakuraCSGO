#include "DiscordRpc.h"
#include "DiscordRPCSDK/Includes/discord_rpc.h"
#include "includes.hpp"

void Discord::Initialize()
{
    DiscordEventHandlers Handle;
    memset(&Handle, 0, sizeof(Handle));
    Discord_Initialize("882180733689548802", &Handle, 1, NULL);
}

void Discord::Update()
{
    DiscordRichPresence discord;
    memset(&discord, 0, sizeof(discord));
    if (m_engine()->IsInGame())
    {
        if (m_gamerules()->m_bIsValveDS())
            discord.state = "Official server";
        else
        {
            auto nci = m_engine()->GetNetChannelInfo();

            if (nci)
            {
                auto server = nci->GetAddress();

                auto tickrate = std::to_string(static_cast<int>(1.0f / m_globals()->m_intervalpertick));

                if (!strcmp(server, crypt_str("loopback")))
                {
                    auto local = ("Local server | " + tickrate + " tick");
                    discord.state = local.c_str();
                }
                else
                {
                    auto online = ("IP: " + std::string(server) + " | " + tickrate + " tick");
                    discord.state = online.c_str();
                }
            }
            else
                discord.state = "Unknown";
        }
    }
    else
        discord.state = "In the lobby";
    auto playing = ("Playing with " + std::string(configs.ragebot.enabled ? "Ragebot" : "Legitbot"));
    discord.details = playing.c_str();
    discord.largeImageKey = "82284613_p0";
    auto edition = ("HackerSense [" + cheat.edition + "]");
    discord.largeImageText = edition.c_str();
    discord.smallImageKey = "csgo";
    discord.smallImageText = "Counter-Strike: Global Offensive";
    discord.startTimestamp = cheat.timeStamp;
    Discord_UpdatePresence(&discord);
}