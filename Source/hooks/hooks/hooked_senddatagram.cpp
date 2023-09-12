#include "..\hooks.hpp"
#include "..\..\features\legitbot\backtrack.h"

void __fastcall Hooks::hooked_packetstart(void* ecx, void* edx, int incoming, int outgoing) noexcept
{
	static auto original_fn = clientstate_hook.getOriginal<void, 5>(incoming, outgoing);

    if (!cheat.available())
        return original_fn(ecx, incoming, outgoing);

	if (!localPlayer->is_alive())
		return original_fn(ecx, incoming, outgoing);
   
    if (cheat.globals.commands.empty())
        return original_fn(ecx, incoming, outgoing);

	if (m_gamerules()->m_bIsValveDS() && configs.misc.anti_untrusted)
		return original_fn(ecx, incoming, outgoing);

    for (auto it = cheat.globals.commands.rbegin(); it != cheat.globals.commands.rend(); ++it)
    {
        if (!it->is_outgoing)
            continue;

        if (it->command_number == outgoing || outgoing > it->command_number && (!it->is_used || it->previous_command_number == outgoing))
        {
            it->previous_command_number = outgoing;
            it->is_used = true;
            original_fn(ecx, incoming, outgoing);
            break;
        }
    }

    auto result = false;

    for (auto it = cheat.globals.commands.begin(); it != cheat.globals.commands.end();) 
    {
        if (outgoing == it->command_number || outgoing == it->previous_command_number)
            result = true;

        if (outgoing > it->command_number && outgoing > it->previous_command_number) 
            it = cheat.globals.commands.erase(it);
        else 
            ++it;
    }

    if (!result)
		original_fn(ecx, incoming, outgoing);
}

void __fastcall Hooks::hooked_packetend(void* ecx, void* edx) noexcept
{
	static auto original_fn = clientstate_hook.getOriginal<void, 6>();

	if (!localPlayer)
	{
		cheat.globals.data.clear();
		return original_fn(ecx);
	}

	if (!localPlayer->is_alive())
	{
		cheat.globals.data.clear();
		return original_fn(ecx);
	}

	if (*(int*)(uintptr_t(ecx) + 0x164) == *(int*)(uintptr_t(ecx) + 0x16C))
	{
		auto ack_cmd = *(int*)(uintptr_t(ecx) + 0x4D2C);
		auto correct = std::find_if(cheat.globals.data.begin(), cheat.globals.data.end(),
			[&ack_cmd](const correction_data& other_data)
			{
				return other_data.command_number == ack_cmd;
			}
		);

		auto netchannel = m_engine()->GetNetChannelInfo();

		if (netchannel && correct != cheat.globals.data.end())
		{
			if (cheat.globals.last_velocity_modifier > localPlayer->m_flVelocityModifier() + 0.1f)
			{
				auto weapon = localPlayer->getActiveWeapon();

				if (!weapon || weapon->m_iItemDefinitionIndex() != WEAPON_REVOLVER && !weapon->is_grenade())
				{
					for (auto& number : cheat.globals.choked_number)
					{
						auto cmd = m_input()->GetUserCmd(number);
						auto verified = m_input()->GetVerifiedUserCmd(number);

						if (cmd->m_buttons & (IN_ATTACK | IN_ATTACK2))
						{
							cmd->m_buttons &= ~IN_ATTACK;

							verified->m_cmd = *cmd;
							verified->m_crc = cmd->GetChecksum();
						}
					}
				}
			}

			cheat.globals.last_velocity_modifier = localPlayer->m_flVelocityModifier();
		}
	}

	return original_fn(ecx);
}

void __fastcall Hooks::hooked_packetprocess(void* ecx, void* edx, void* packet, bool header) noexcept
{
	static auto original_fn = clientstate_hook.getOriginal<void, 39>(packet, header);

	if (!m_clientstate()->m_pNetChannel)
		return original_fn(ecx, packet, header);

	original_fn(ecx, packet, header);

	for (auto it{ m_clientstate()->m_pEvents }; it != nullptr; it = it->m_pNext) {
		if (!it->m_sClassID)
			continue;

		// set all delays to instant.
		it->m_flFireDelay = 0.f;
	}

	m_engine()->FireEvents();
}

int __fastcall Hooks::hooked_senddatagram(INetChannel* network, void* edx, void* datagram) noexcept
{
	if (!key_binds::get().get_key_bind_state(key_bind_features::PING_SPIKE) || datagram || !m_engine()->IsInGame())
		return senddatagram_hook.callOriginal<int, INetChannel*>(network, datagram);

	const int instate = network->m_nInReliableState;
	const int insequencenr = network->m_nInSequenceNr;

	const float delta = max(0.f, (configs.backtrack.pingSpikeAmount / 1000.f)); //  - util::get_interpolation() - network->GetLatency(FLOW_OUTGOING

	backtrack::get().addLatencyToNetwork(network, delta);

	int result = senddatagram_hook.callOriginal<int, INetChannel*>(network, datagram);

	network->m_nInReliableState = instate;
	network->m_nInSequenceNr = insequencenr;

	return result;
}

int __stdcall Hooks::hooked_getunverifiedfilehashes(void* ecx, void* edx, int maxFiles) noexcept
{
	if (configs.misc.sv_pure_bypass)
		return 0;

	return filesystem_hook.callOriginal<int, 101>(ecx, maxFiles);
}

int __fastcall Hooks::hooked_canloadthirdpartyfiles(void* ecx, void* edx) noexcept
{
	if (configs.misc.sv_pure_bypass)
		return 1;

	return filesystem_hook.callOriginal<int, void*, 128>(ecx);
}
