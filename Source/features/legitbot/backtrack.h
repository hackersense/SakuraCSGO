#pragma once

#include <cstddef>
#include <deque>

#include "..\..\includes.hpp"

struct Record
{
    std::deque<Vector> positions;
    int ownerIdx;
    Vector origin;
    float simulationTime;
    matrix3x4_t matrix[MAXSTUDIOBONES];
};

struct IncomingSequence {
    int inreliablestate;
    int sequencenr;
    float servertime;
};

class backtrack : public singleton <backtrack>
{
public:
    void update() noexcept;
    void run(CUserCmd*) noexcept;
    const std::deque<Record>* getRecords(std::size_t index) noexcept;
    bool valid(float simtime) noexcept;
    void addLatencyToNetwork(INetChannel*, float) noexcept;
    void updateIncomingSequences() noexcept;
};
