#pragma once

#include <cassert>

class player_t;

class LocalPlayer {
public:
    void init(player_t** entity) noexcept
    {
        assert(!localEntity);
        localEntity = entity;
    }

    constexpr operator bool() noexcept
    {
        assert(localEntity);
        return *localEntity != nullptr;
    }

    constexpr auto operator->() noexcept
    {
        assert(localEntity && *localEntity);
        return *localEntity;
    }

    constexpr auto get() noexcept
    {
        assert(localEntity && *localEntity);
        return *localEntity;
    }
private:
    player_t** localEntity = nullptr;
};

inline LocalPlayer localPlayer;
