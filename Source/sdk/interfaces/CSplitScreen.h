#pragma once

#include "CClientState.hpp"

struct CSplitScreen {
    void* vmt;

    struct SplitPlayer {
        std::byte pad[8];
        CClientState client;
    };

    SplitPlayer* splitScreenPlayers[1];
};