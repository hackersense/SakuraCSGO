#include "..\hooks.hpp"
#include "..\..\features\serverside\engine_prediction.h"

void __stdcall Hooks::hooked_emitsound(SoundParams_t params) noexcept
{
	//if (engineprediction::get().isInPrediction())
	//	return;

	if (!strcmp(params.m_nSoundEntry, "UIPanorama.popup_accept_match_beep") && configs.misc.auto_accept) {
		if (const auto idx = memory->registeredPanoramaEvents->find(memory->makePanoramaSymbol("MatchAssistedAccept")); idx != -1) {
			if (const auto eventPtr = memory->registeredPanoramaEvents->memory[idx].value.makeEvent(nullptr))
				m_panoramauiengine()->accessUIEngine()->dispatchEvent(eventPtr);
		}

		FLASHWINFO flash{ sizeof(FLASHWINFO), WindowInfo::window, FLASHW_TRAY | FLASHW_TIMERNOFG, 0, 0 };
		FlashWindowEx(&flash);
		ShowWindow(WindowInfo::window, SW_RESTORE);
	}

	return engine_sound_hook.callOriginal<void, 5>(params);
}
