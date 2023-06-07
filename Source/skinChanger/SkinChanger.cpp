#include <algorithm>
#include <fstream>
#include "..\configs\configs.h"
#include "..\sdk\ProtobufReader.h"
#include "..\sdk\ItemSchema.h"

static const char* player_model_index_t[] =
{
	"models/player/custom_player/legacy/tm_phoenix_variantf.mdl",
	"models/player/custom_player/legacy/tm_phoenix_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianti.mdl",
	"models/player/custom_player/legacy/tm_leet_varianth.mdl",
	"models/player/custom_player/legacy/tm_phoenix_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantf.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantj.mdl",
	"models/player/custom_player/legacy/tm_leet_varianti.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantg.mdl",
	"models/player/custom_player/legacy/tm_balkan_varianth.mdl",
	"models/player/custom_player/legacy/tm_leet_variantf.mdl",
	"models/player/custom_player/legacy/tm_professional_varf.mdl",
	"models/player/custom_player/legacy/tm_professional_varf1.mdl",
	"models/player/custom_player/legacy/tm_professional_varf2.mdl",
	"models/player/custom_player/legacy/tm_professional_varf3.mdl",
	"models/player/custom_player/legacy/tm_professional_varf4.mdl",
	"models/player/custom_player/legacy/tm_professional_varg.mdl",
	"models/player/custom_player/legacy/tm_professional_varh.mdl",
	"models/player/custom_player/legacy/tm_professional_vari.mdl",
	"models/player/custom_player/legacy/tm_professional_varj.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantk.mdl",
	"models/player/custom_player/legacy/tm_balkan_variantl.mdl"
};

static const char* player_model_index_ct[] =
{
	"models/player/custom_player/legacy/ctm_st6_variante.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantk.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantf.mdl",
	"models/player/custom_player/legacy/ctm_sas_variantf.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantg.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantm.mdl",
	"models/player/custom_player/legacy/ctm_st6_varianti.mdl",
	"models/player/custom_player/legacy/ctm_fbi_variantb.mdl",
	"models/player/custom_player/legacy/ctm_swat_variante.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantf.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantg.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianth.mdl",
	"models/player/custom_player/legacy/ctm_swat_varianti.mdl",
	"models/player/custom_player/legacy/ctm_swat_variantj.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantj.mdl",
	"models/player/custom_player/legacy/ctm_st6_variantl.mdl"
};

std::unordered_map<std::string, int> SkinChanger::player_model_indexes;

std::vector<SkinChanger::PaintKit> SkinChanger::skinKits;
std::vector<SkinChanger::PaintKit> SkinChanger::gloveKits;
std::vector<SkinChanger::PaintKit> SkinChanger::displayKits;

static void apply_config_on_attributable_item(weapon_t* item, const item_setting* config, const uint64_t xuid, const unsigned xuid_low, bool isGlove) noexcept
{
	item->m_iItemIDHigh() = -1;

	item->m_iAccountID() = xuid_low;

	if (config->quality)
		item->m_iEntityQuality() = config->quality;

	if (config->custom_name[0])
		std::strncpy(item->m_szCustomName(), config->custom_name, sizeof(config->custom_name));
	//strcpy_s(item->m_szCustomName(), sizeof(config->custom_name), config->custom_name);

	if (config->paintKit)
		item->m_nFallbackPaintKit() = config->paintKit;

	if (config->seed)
		item->m_nFallbackSeed() = config->seed;

	if (config->stat_trak)
		item->m_nFallbackStatTrak() = config->stat_trak;

	item->m_flFallbackWear() = config->wear;

	if (auto& definition_index = item->m_iItemDefinitionIndex(); config->definition_override_index && config->definition_override_index != definition_index)
	{
		definition_index = config->definition_override_index;
		if (const auto def = memory->itemSystem()->getItemSchema()->getItemDefinitionInterface(definition_index)) {
			item->set_model_index(m_modelinfo()->GetModelIndex(isGlove ? def->getWorldDisplayModel() : def->getPlayerDisplayModel()));
			item->PreDataUpdate(0);
		}
	}
}

static auto get_wearable_create_fn() -> CreateClientClassFn
{
	auto classes = m_client()->GetAllClasses();

	while (classes->m_ClassID != CEconWearable)
		classes = classes->m_pNext;

	return classes->m_pCreateFn;
}

static weapon_t* make_glove(int entry, int serial) noexcept
{
	get_wearable_create_fn()(entry, serial);
	auto glove = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntity(entry));
	assert(glove);
	return glove;
}

static void post_data_update_start(player_t* local) noexcept
{
	//if (!local->is_alive())
	//	return;

	player_info_t player_info;

	if (!m_engine()->GetPlayerInfo(local->EntIndex(), &player_info))
		return;

	static auto glove_handle = CBaseHandle(0);

	auto wearables = local->m_hMyWearables();
	auto glove_config = get_by_definition_index(GLOVE_T_SIDE);
	auto glove = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(wearables[0]));

	if (!glove)
	{
		auto our_glove = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(glove_handle));

		if (our_glove)
		{
			wearables[0] = glove_handle;
			glove = our_glove;
		}
	}

	if (glove_config && glove_config->definition_override_index)
	{
		if (!glove)
		{
			auto entry = m_entitylist()->GetHighestEntityIndex() + 1;
			auto serial = rand() % 0x1000;

			glove = make_glove(entry, serial);
			glove->m_bInitialized() = true;
			wearables[0] = entry | serial << 16;
			glove_handle = wearables[0];
		}

		memory->equipWearable(glove, local);
		local->body() = 1;
		apply_config_on_attributable_item(glove, glove_config, player_info.xuid, player_info.xuidLow, true);
	}

	auto weapons = local->m_hMyWeapons();

	for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++)
	{
		auto weapon = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(weapons[weapon_handle]));

		if (!weapon)
			continue;

		auto& definition_index = weapon->m_iItemDefinitionIndex();

		if (auto active_conf = get_by_definition_index(is_knife(definition_index) ? WEAPON_KNIFE : definition_index))
			apply_config_on_attributable_item(weapon, active_conf, player_info.xuid, player_info.xuidLow, false);
	}

	auto view_model = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(local->m_hViewModel()));

	if (!view_model)
		return;

	auto view_model_weapon = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(view_model->m_hWeapon()));

	if (!view_model_weapon)
		return;

	const auto def = memory->itemSystem()->getItemSchema()->getItemDefinitionInterface(view_model_weapon->m_iItemDefinitionIndex());

	if (!def)
		return;

	view_model->set_model_index(m_modelinfo()->GetModelIndex(def->getPlayerDisplayModel()));

	auto world_model = reinterpret_cast<weapon_t*>(m_entitylist()->GetClientEntityFromHandle(view_model_weapon->m_hWeaponWorldModel()));

	if (!world_model)
		return;

	world_model->set_model_index(m_modelinfo()->GetModelIndex(def->getWorldDisplayModel()));
}

static bool UpdateRequired = false;
static bool hudUpdateRequired = false;

static constexpr void updateHud() noexcept
{
	if (auto hud_weapons = memory->findHudElement(memory->hud, crypt_str("CCSGO_HudWeaponSelection")) - 0x28)
		for (auto i = 0; i < *(hud_weapons + 32); i++)
			i = memory->clearHudWeapon(hud_weapons, i);

	hudUpdateRequired = false;
}

void SkinChanger::setup() noexcept
{
	auto items = std::ifstream(crypt_str("csgo/scripts/items/items_game_cdn.txt"));
	auto gameItems = std::string(std::istreambuf_iterator<char>{ items }, std::istreambuf_iterator<char>{ });

	if (!items.is_open())
		return;

	items.close();

	for (auto i = 0; i <= memory->itemSystem()->paintKits.lastElement; i++)
	{
		auto paintKit = memory->itemSystem()->paintKits.memory[i].value;

		if (paintKit->id == 9001)
			continue;

		auto itemName = m_localize()->FindSafe(paintKit->itemName.buffer + 1);
		auto itemNameLength = WideCharToMultiByte(CP_UTF8, 0, itemName, -1, nullptr, 0, nullptr, nullptr);

		if (std::string name(itemNameLength, 0); WideCharToMultiByte(CP_UTF8, 0, itemName, -1, &name[0], itemNameLength, nullptr, nullptr))
		{
			if (paintKit->id < 10000)
			{
				if (auto pos = gameItems.find('_' + std::string{ paintKit->name.buffer } + '='); pos != std::string::npos && gameItems.substr(pos + paintKit->name.length).find('_' + std::string{ paintKit->name.buffer } + '=') == std::string::npos)
				{
					if (auto weaponName = gameItems.rfind(crypt_str("weapon_"), pos); weaponName != std::string::npos)
					{
						name.back() = ' ';
						name += '(' + gameItems.substr(weaponName + 7, pos - weaponName - 7) + ')';
					}
				}
				skinKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
			else
			{
				std::string_view gloveName{ paintKit->name.buffer };
				name.back() = ' ';
				name += '(' + std::string(gloveName.substr(0, gloveName.find('_'))) + ')';
				gloveKits.emplace_back(paintKit->id, std::move(name), paintKit->name.buffer);
			}
		}
	}

	std::sort(skinKits.begin(), skinKits.end());
	std::sort(gloveKits.begin(), gloveKits.end());
}

void SkinChanger::run(ClientFrameStage_t stage) noexcept
{
	if (!cheat.available())
		return;

	if (stage == FRAME_NET_UPDATE_POSTDATAUPDATE_START)
	{
		post_data_update_start(cheat.local());

		if (!cheat.local()->is_alive())
		{
			UpdateRequired = false;
			hudUpdateRequired = false;
			return;
		}

		if (UpdateRequired)
		{
			UpdateRequired = false;
			hudUpdateRequired = true;

			m_clientstate()->ForceFullUpdate();
			cheat.globals.updating_skins = true;
		}
		else if (hudUpdateRequired && !cheat.globals.updating_skins)
		{
			hudUpdateRequired = false;
			if (!cheat.local()->IsDormant())
				updateHud();
		}
	}

	if (stage != FRAME_RENDER_START)
		return;

	static auto backup_model_index = -1;

	if (configs.player.enabled)
	{
		const char** player_model_index = nullptr;
		auto player_model = 0;

		switch (cheat.local()->m_iTeamNum())
		{
		case 2:
			player_model_index = player_model_index_t;
			player_model = configs.player.player_model_t;
			break;
		case 3:
			player_model_index = player_model_index_ct;
			player_model = configs.player.player_model_ct;
			break;
		}

		if (player_model)
		{
			if (!cheat.globals.backup_model)
			{
				auto model = cheat.local()->GetModel();

				if (model)
				{
					auto studio_model = m_modelinfo()->GetStudioModel(model);

					if (studio_model)
					{
						auto name = crypt_str("models/") + std::string(studio_model->szName);
						backup_model_index = m_modelinfo()->GetModelIndex(name.c_str());
					}
				}
			}

			if (SkinChanger::player_model_indexes.find(player_model_index[player_model - 1]) == SkinChanger::player_model_indexes.end())
				SkinChanger::player_model_indexes.emplace(player_model_index[player_model - 1], m_modelinfo()->GetModelIndex(player_model_index[player_model - 1]));

			cheat.local()->set_model_index(SkinChanger::player_model_indexes[player_model_index[player_model - 1]]);
			cheat.globals.backup_model = true;
		}
		else if (cheat.globals.backup_model)
		{
			cheat.local()->set_model_index(backup_model_index);
			cheat.globals.backup_model = false;
		}
	}
}

void SkinChanger::scheduleHudUpdate() noexcept
{
	if (!cheat.local()->is_alive())
		return;

	UpdateRequired = true;
}

std::uint64_t UserIdToXUID(const int user_id)
{
	for (auto i = 1; i < m_globals()->m_maxclients; i++)
	{
		const auto entity = static_cast<entity_t*>(m_entitylist()->GetClientEntity(i));

		if (!entity || !entity->is_player())
			continue;

		const auto player = reinterpret_cast<player_t*>(entity);

		if (!player)
			continue;

		player_info_t player_info;

		if (!m_engine()->GetPlayerInfo(i, &player_info))
			continue;

		if (user_id == player_info.userId)
			return player_info.xuid;
	}
	return 0;
}

void SkinChanger::overrideHudIcon(IGameEvent* event) noexcept
{
	if (!cheat.local())
		return;

	if (event->GetInt("attacker") != cheat.local()->get_player_info()->userId)
		return;

	if (const auto weapon = event->GetString("weapon"); weapon != "knife" && weapon != "knife_t")
		return;

	if (const auto active_conf = get_by_definition_index(WEAPON_KNIFE)) {
		if (const auto def = memory->itemSystem()->getItemSchema()->getItemDefinitionInterface(active_conf->definition_override_index)) {
			if (const auto defName = def->getDefinitionName(); defName && std::string(defName).rfind("weapon_") != std::string::npos)
				event->SetString("weapon", defName + 7);
		}
	}
}

void SkinChanger::updateStatTrak(IGameEvent* event) noexcept
{
	if (!cheat.local())
		return;

	if (const auto localUserId = cheat.local()->get_player_info()->userId; event->GetInt("attacker") != localUserId || event->GetInt("userid") == localUserId)
		return;

	const auto weapon = static_cast<weapon_t*>(cheat.local()->m_hActiveWeapon());
	if (!weapon)
		return;

	if (const auto conf = get_by_definition_index(is_knife(weapon->m_iItemDefinitionIndex()) ? WEAPON_KNIFE : weapon->m_iItemDefinitionIndex()); conf && conf->stat_trak > -1) {
		weapon->m_nFallbackStatTrak() = ++conf->stat_trak;
		weapon->PostDataUpdate(0);
	}
}

[[nodiscard]] static bool isDefaultKnifeNameLocalizationString(std::string_view string) noexcept
{
	return string == "#SFUI_WPNHUD_Knife" || string == "#SFUI_WPNHUD_Knife_T";
}

static void appendProtobufString(std::string_view string, std::vector<char>& buffer) noexcept
{
	assert(string.length() < 128);
	buffer.push_back(0x1A);
	buffer.push_back(static_cast<char>(string.length()));
	const char* s = &(*buffer.begin());
	string = std::string(s, buffer.size());
	std::cout << string;
	//std::ranges::copy(string, std::back_inserter(buffer));
}

[[nodiscard]] static std::vector<char> buildTextUserMessage(int destination, std::string_view string1, std::string_view string2, std::string_view string3 = {}) noexcept
{
	std::vector<char> buffer{ 0x8, static_cast<char>(destination) };
	appendProtobufString(string1, buffer);
	appendProtobufString(string2, buffer);
	appendProtobufString(string3, buffer);
	// game client expects text protobuf to contain 5 strings
	appendProtobufString("", buffer);
	appendProtobufString("", buffer);
	return buffer;
}

void SkinChanger::onUserTextMsg(const void*& data, int& size) noexcept
{
	if (!cheat.local())
		return;

	constexpr auto HUD_PRINTTALK = 3;
	constexpr auto HUD_PRINTCENTER = 4;
	// https://github.com/SteamDatabase/Protobufs/blob/017f1710737b7026cdd6d7e602f96a66dddb7b2e/csgo/cstrike15_usermessages.proto#L128-L131

	const auto reader = ProtobufReader{ static_cast<const std::uint8_t*>(data), size };

	if (reader.readInt32(1) == HUD_PRINTCENTER) {
		const auto strings = reader.readRepeatedString(3);
		if (strings.size() < 2)
			return;

		if (strings[0] != "#SFUI_Notice_CannotDropWeapon" &&
			strings[0] != "#SFUI_Notice_YouDroppedWeapon")
			return;

		if (!isDefaultKnifeNameLocalizationString(strings[1]))
			return;

		auto weapons = cheat.local()->m_hMyWeapons();

		for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++)
		{
			auto weapon = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(weapons[weapon_handle]);

			if (!weapon)
				continue;

			auto& definition_index = weapon->m_iItemDefinitionIndex();

			if (is_knife(definition_index))
			{
				static std::vector<char> buffer;
				buffer = buildTextUserMessage(HUD_PRINTCENTER, strings[0], weapon->get_weapon_info()->szWeaponName);
				data = buffer.data();
				size = static_cast<int>(buffer.size());
			}
		}
	}
	else if (reader.readInt32(1) == HUD_PRINTTALK) {
		const auto strings = reader.readRepeatedString(3);
		if (strings.size() < 3)
			return;

		if (strings[0] != "#Player_Cash_Award_Killed_Enemy" &&
			strings[0] != "#Player_Point_Award_Killed_Enemy" &&
			strings[0] != "#Player_Point_Award_Killed_Enemy_Plural")
			return;

		if (!isDefaultKnifeNameLocalizationString(strings[2]))
			return;

		auto weapons = cheat.local()->m_hMyWeapons();

		for (auto weapon_handle = 0; weapons[weapon_handle].IsValid(); weapon_handle++)
		{
			auto weapon = (weapon_t*)m_entitylist()->GetClientEntityFromHandle(weapons[weapon_handle]);

			if (!weapon)
				continue;

			auto& definition_index = weapon->m_iItemDefinitionIndex();

			if (is_knife(definition_index))
			{
				static std::vector<char> buffer;
				buffer = buildTextUserMessage(HUD_PRINTTALK, strings[0], weapon->get_weapon_info()->szWeaponName);
				data = buffer.data();
				size = static_cast<int>(buffer.size());
			}
		}
	}
}