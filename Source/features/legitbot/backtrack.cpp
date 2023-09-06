#include <algorithm>
#include <array>

#include "Backtrack.h"

static std::array<std::deque<Record>, 65> records;
static std::deque<IncomingSequence> sequences;

float getExtraTicks() noexcept
{
    if (!key_binds::get().get_key_bind_state(key_bind_features::PING_SPIKE) || configs.backtrack.pingSpikeAmount <= 0)
        return 0.f;
    return static_cast<float>(configs.backtrack.pingSpikeAmount) / 1000.f;
}

void backtrack::update() noexcept
{
    if (!m_engine()->IsInGame())
        return;

    if (!configs.backtrack.enabled || !cheat.local() || !cheat.local()->is_alive()) {
        for (auto& record : records)
            record.clear();
        return;
    }

    for (auto i = 1; i < m_globals()->m_maxclients; i++)
    {
        auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(i));
        if (!entity || entity == cheat.local() || entity->IsDormant() || !entity->is_alive() || entity->m_iTeamNum() == cheat.local()->m_iTeamNum() || entity->m_bGunGameImmunity()) {
            records[i].clear();
            continue;
        }

        if (!records[i].empty() && (records[i].front().simulationTime == entity->m_flSimulationTime()))
            continue;

       // if (!valid(entity->m_flSimulationTime()))
       //     continue;

        Record record{ };
        record.ownerIdx = entity->EntIndex();
        record.origin = entity->GetAbsOrigin();
        record.simulationTime = entity->m_flSimulationTime();

        entity->setup_bones_fixed(record.matrix, cheat.local()->m_CachedBoneData().m_Size, BONE_USED_BY_ANYTHING, m_globals()->m_curtime);

        if (record.matrix)
            record.positions.push_back(record.matrix[8].GetOrigin());
            //for (auto bone : { 8, 4, 3, 7, 6, 5 })
             //   record.positions.push_back(record.matrix[bone].GetOrigin());

        records[i].push_front(record);

        while (records[i].size() > 3 && records[i].size() > static_cast<size_t>(TIME_TO_TICKS(static_cast<float>(configs.backtrack.timeLimit) / 1000.f + getExtraTicks())))
            records[i].pop_back();

       // if (auto invalid = std::find_if(std::cbegin(records[i]), std::cend(records[i]), [](const Record& rec) { return !backtrack::get().valid(rec.simulationTime); }); invalid != std::cend(records[i]))
        //    records[i].erase(invalid, std::cend(records[i]));
    }
}

void backtrack::run(CUserCmd* cmd) noexcept
{
    if (!configs.backtrack.enabled)
        return;

    if (!(cmd->m_buttons & IN_ATTACK))
        return;

    if (!cheat.local()->is_alive())
        return;

    //auto bestFov{ configs.backtrack.fov };
    auto bestFov{ 360.0f };
    int bestTargetIndex{ };
    int bestRecord{ };

    const auto& aimPunch = cheat.globals.weapon->requires_recoil_control() ? cheat.local()->m_aimPunchAngle() : ZERO;

    for (int index = 1; index <= m_globals()->m_maxclients; index++) {
        auto entity = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(index));
        if (!entity || entity == cheat.local() || entity->IsDormant() || !entity->is_alive()
            || entity->m_iTeamNum() == cheat.local()->m_iTeamNum())
            continue;

        if (records[index].empty() || (!configs.backtrack.ignoreSmoke && memory->lineGoesThroughSmoke(cheat.globals.eye_pos, entity->GetAbsOrigin(), 1)) || (!configs.backtrack.ignoreFlash && cheat.local()->m_flFlashDuration() > 0))
            continue;

        for (int i = static_cast<int>(records[index].size() - 1U); i >= 0; i--)
        {
            const auto& record = records[index][i];
            if (!valid(record.simulationTime))
                continue;

            for (auto& position : record.positions) {
                auto angle = math::calculate_relative_angle(cheat.globals.eye_pos, position, cmd->m_viewangles + aimPunch);
                auto fov = std::hypotf(angle.x, angle.y);
                if (fov < bestFov) {
                    bestFov = fov;
                    bestRecord = i;
                    bestTargetIndex = index;
                }
            }
        }
    }

    if (bestRecord) {
        const auto& record = records[bestTargetIndex][bestRecord];

        if (!record.matrix)
            return;

        reinterpret_cast<player_t*>(m_entitylist()->GetClientEntity(bestTargetIndex))->set_abs_origin(record.origin);
        cmd->m_tickcount = TIME_TO_TICKS(record.simulationTime + util::get_interpolation());
    }
}

const std::deque<Record>* backtrack::getRecords(std::size_t index) noexcept
{
    if (!configs.backtrack.enabled)
        return nullptr;
    return &records[index];
}

bool backtrack::valid(float simtime) noexcept
{
    const auto network = m_engine()->GetNetChannelInfo();
    if (!network)
        return false;

    static auto sv_maxunlag = m_cvar()->FindVar(crypt_str("sv_maxunlag"));

    const auto deadTime = static_cast<int>(util::server_time() - sv_maxunlag->GetFloat());
    if (simtime < deadTime)
        return false;

    auto delta = std::clamp(network->GetLatency(FLOW_OUTGOING) + network->GetLatency(FLOW_INCOMING) + util::get_interpolation(), 0.f, m_cvar()->FindVar("sv_maxunlag")->GetFloat()) - (util::server_time() - simtime);
    return std::abs(delta) <= 0.2f;
}

void backtrack::addLatencyToNetwork(INetChannel* network, float latency) noexcept
{
    for (auto& sequence : sequences)
    {
        if ((util::server_time() - sequence.servertime) >= latency)
        {
            network->m_nInReliableState = sequence.inreliablestate;
            network->m_nInSequenceNr = sequence.sequencenr;
            break;
        }
    }
}

void backtrack::updateIncomingSequences() noexcept
{
    static int lastIncomingSequenceNumber = 0;

    //if (!key_binds::get().get_key_bind_state(key_bind_features::PING_SPIKE))
    //    return;

    if (!cheat.local())
        return;

    //if (cheat.local()->m_iTeamNum() == TeamID::None)
    //    return;

    auto network = m_clientstate()->m_pNetChannel;
    if (!network)
        return;

    if (network->m_nInSequenceNr != lastIncomingSequenceNumber)
    {
        lastIncomingSequenceNumber = network->m_nInSequenceNr;

        IncomingSequence sequence{ };
        sequence.inreliablestate = network->m_nInReliableState;
        sequence.sequencenr = network->m_nInSequenceNr;
        sequence.servertime = util::server_time();
        sequences.push_front(sequence);
    }

    while (sequences.size() > 2048)
        sequences.pop_back();
}
