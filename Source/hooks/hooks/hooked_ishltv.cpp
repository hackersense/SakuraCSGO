#include "..\hooks.hpp"

bool __fastcall Hooks::hooked_ishltv()
{
	if (cheat.globals.setuping_bones || cheat.globals.updating_animation)
		return true;

	static auto accumulate_layers_call = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 0D F6 87"));
	static auto setup_velocity_call = util::FindSignature(crypt_str("client.dll"), crypt_str("84 C0 75 38 8B 0D ? ? ? ? 8B 01 8B 80 ? ? ? ? FF D0"));

	if (uintptr_t(_ReturnAddress()) == accumulate_layers_call || uintptr_t(_ReturnAddress()) == setup_velocity_call)
		return true;

	return engine_hook.callOriginal<bool, 93>();
}