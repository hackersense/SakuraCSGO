#include "configs.h"
#include "base64.h"
#include "aes256.hpp"
#include "..\includes.hpp"
#include "..\utils\util.hpp"
#include "..\utils\StringUtils.h"
#include <shlobj.h>

std::unordered_map<std::string, float[4]> colors;

C_ConfigManager* cfg_manager = new C_ConfigManager();
Config configs;

item_setting* get_by_definition_index(const int definition_index)
{
	auto it = std::find_if(std::begin(configs.skins.skinChanger), std::end(configs.skins.skinChanger), [definition_index](const item_setting& e)
		{
			return e.itemId == definition_index;
		});

	return it == std::end(configs.skins.skinChanger) ? nullptr : &*it;
}

void C_ConfigManager::setup()
{
	setup_item(&configs.legitbot.enabled, false, crypt_str("Legitbot.enabled"));
	setup_item(&configs.legitbot.key, key_bind(HOLD), crypt_str("Legitbot.key"));
	setup_item(&configs.legitbot.friendly_fire, false, crypt_str("Legitbot.friendly_fire"));
	setup_item(&configs.legitbot.autopistol, false, crypt_str("Legitbot.autopistol"));
	setup_item(&configs.legitbot.autoscope, false, crypt_str("Legitbot.autoscope"));
	setup_item(&configs.legitbot.sniper_scope_only, false, crypt_str("Legitbot.sniper_scope_only"));
	setup_item(&configs.legitbot.ignore_flash, false, crypt_str("Legitbot.ignore_flash"));
	setup_item(&configs.legitbot.jump_check, false, crypt_str("Legitbot.jump_check"));
	setup_item(&configs.legitbot.ignore_smoke, false, crypt_str("Legitbot.ignore_smoke"));

	for (auto i = 0; i < 6; i++)
	{
		setup_item(&configs.legitbot.weapon[i].aimlock, false, std::to_string(i) + crypt_str("Legitbot.aimlock"));
		setup_item(&configs.legitbot.weapon[i].silent, false, std::to_string(i) + crypt_str("Legitbot.silent"));
		setup_item(&configs.legitbot.weapon[i].hitboxes, 0, std::to_string(i) + crypt_str("Legitbot.hitboxes"));
		setup_item(&configs.legitbot.weapon[i].autostop, false, std::to_string(i) + crypt_str("Legitbot.autostop"));
		setup_item(&configs.legitbot.weapon[i].fov, 0.0f, std::to_string(i) + crypt_str("Legitbot.fov"));
		setup_item(&configs.legitbot.weapon[i].smooth, 1.0f, std::to_string(i) + crypt_str("Legitbot.smooth"));
		setup_item(&configs.legitbot.weapon[i].maxAimInaccuracy, 0, std::to_string(i) + crypt_str("Legitbot.maxAimInaccuracy"));
		setup_item(&configs.legitbot.weapon[i].min_damage, 0, std::to_string(i) + crypt_str("Legitbot.min_damage"));
		setup_item(&configs.legitbot.weapon[i].between_shots, false, std::to_string(i) + crypt_str("Legitbot.between_shots"));
		setup_item(&configs.legitbot.weapon[i].shotsFired, 0, std::to_string(i) + crypt_str("Legitbot.shots_fired"));
		setup_item(&configs.legitbot.weapon[i].standaloneRCS, false, std::to_string(i) + crypt_str("Legitbot.standaloneRCS"));
		setup_item(&configs.legitbot.weapon[i].silentRCS, false, std::to_string(i) + crypt_str("Legitbot.silentRCS"));
		setup_item(&configs.legitbot.weapon[i].recoilControlHorizontal, 0, std::to_string(i) + crypt_str("Legitbot.recoilControlHorizontal"));
		setup_item(&configs.legitbot.weapon[i].recoilControlVertical, 0, std::to_string(i) + crypt_str("Legitbot.recoilControlVertical"));
		setup_item(&configs.legitbot.weapon[i].randomRCS, false, std::to_string(i) + crypt_str("Legitbot.randomRCS"));
	}

	setup_item(&configs.backtrack.enabled, false, crypt_str("Backtrack.enabled"));
	setup_item(&configs.backtrack.ignoreSmoke, false, crypt_str("Backtrack.ignore_smoke"));
	setup_item(&configs.backtrack.ignoreFlash, false, crypt_str("Backtrack.ignore_flash"));
	//setup_item(&configs.backtrack.recoilBasedFov, false, crypt_str("Backtrack.recoil_based_fov"));
	//setup_item(&configs.backtrack.fov, 360, crypt_str("Backtrack.fov"));
	setup_item(&configs.backtrack.timeLimit, 200, crypt_str("Backtrack.time_limit"));
	setup_item(&configs.backtrack.pingSpike, key_bind(HOLD), crypt_str("Backtrack.ping_spike"));
	setup_item(&configs.backtrack.pingSpikeAmount, 200, crypt_str("Backtrack.ping_spike_amount"));

	setup_item(&configs.ragebot.enabled, false, crypt_str("Ragebot.enabled"));
	setup_item(&configs.ragebot.silent_aim, false, crypt_str("Ragebot.silent_aim"));
	setup_item(&configs.ragebot.field_of_view, 1, crypt_str("Ragebot.fov"));
	setup_item(&configs.ragebot.autowall, 0, crypt_str("Ragebot.autowall"));
	setup_item(&configs.ragebot.zeus_bot, false, crypt_str("Ragebot.zeus_bot"));
	setup_item(&configs.ragebot.zeus_hitchance, 1, crypt_str("Ragebot.zeus_hitchance"));
	setup_item(&configs.ragebot.knife_bot, false, crypt_str("Ragebot.knife_bot"));
	setup_item(&configs.ragebot.autoshoot, false, crypt_str("Ragebot.autoshoot"));
	setup_item(&configs.ragebot.double_tap, false, crypt_str("Ragebot.double_tap"));
	setup_item(&configs.ragebot.double_tap_instant, false, crypt_str("Ragebot.double_tap_instant"));
	setup_item(&configs.ragebot.disable_teleport, false, crypt_str("Ragebot.disable_teleport"));
	setup_item(&configs.ragebot.double_tap_key, key_bind(TOGGLE), crypt_str("Ragebot.double_tap_key"));
	setup_item(&configs.ragebot.hide_shots, false, crypt_str("Ragebot.hide_shots"));
	setup_item(&configs.ragebot.hide_shots_key, key_bind(TOGGLE), crypt_str("Ragebot.hide_shots_key"));
	setup_item(&configs.ragebot.autoscope, 0, crypt_str("Ragebot.autoscope"));
	setup_item(&configs.ragebot.safe_point_key, key_bind(HOLD), crypt_str("Ragebot.safe_point_key"));
	setup_item(&configs.ragebot.body_aim_key, key_bind(HOLD), crypt_str("Ragebot.body_aim_key"));
	setup_item(&configs.ragebot.damage_override_key, key_bind(HOLD), crypt_str("Ragebot.damage_override_key"));
	setup_item(&configs.ragebot.resolver, 2, crypt_str("Ragebot.resolver"));
	setup_item(&configs.ragebot.resolver_mode, 0, crypt_str("Ragebot.resolver_mode"));
	setup_item(&configs.ragebot.resolver_override, key_bind(HOLD), crypt_str("Ragebot.resolver_override"));
	setup_item(&configs.ragebot.adaptive_resolver, false, crypt_str("Ragebot.adaptive_resolver"));
	setup_item(&configs.ragebot.adaptive_resolver_ignore_body, false, crypt_str("Ragebot.adaptive_resolver_ignore_body"));
	setup_item(&configs.ragebot.dormant, false, crypt_str("Ragebot.dormant"));

	for (auto i = 0; i < 10; i++)
	{
		if (i != 0)
			setup_item(&configs.ragebot.weapon[i].enabled, false, std::to_string(i) + crypt_str("Ragebot.enabled"));
		setup_item(&configs.ragebot.weapon[i].selection_type, 0, std::to_string(i) + crypt_str("Ragebot.selection_type"));
		setup_item(&configs.ragebot.weapon[i].autostop, false, std::to_string(i) + crypt_str("Ragebot_automaticstop"));
		setup_item(&configs.ragebot.weapon[i].autostop_modifiers, 6, std::to_string(i) + crypt_str("Ragebot.autostop_conditions"));
		setup_item(&configs.ragebot.weapon[i].double_tap_hitchance, false, std::to_string(i) + crypt_str("Ragebot.double_tap_hitchance"));
		setup_item(&configs.ragebot.weapon[i].double_tap_hitchance_amount, 1, std::to_string(i) + crypt_str("Ragebot.double_tap_hitchance_amount"));
		setup_item(&configs.ragebot.weapon[i].hitchance, 0, std::to_string(i) + crypt_str("Ragebot.hitchance"));
		setup_item(&configs.ragebot.weapon[i].minimum_visible_damage, 1, std::to_string(i) + crypt_str("Ragebot.minimum_visible_damage"));
		setup_item(&configs.ragebot.weapon[i].minimum_damage, 1, std::to_string(i) + crypt_str("Ragebot.minimum_damage"));
		setup_item(&configs.ragebot.weapon[i].minimum_override_damage, 1, std::to_string(i) + crypt_str("Ragebot.minimum_override_damage"));
		setup_item(&configs.ragebot.weapon[i].hitboxes, 9, std::to_string(i) + crypt_str("Ragebot.hitboxes"));
		setup_item(&configs.ragebot.weapon[i].safe_points_conditions, 6, std::to_string(i) + crypt_str("Ragebot.safe_points_conditions"));
		setup_item(&configs.ragebot.weapon[i].body_aim_conditions, 4, std::to_string(i) + crypt_str("Ragebot.body_aim_conditions"));
		setup_item(&configs.ragebot.weapon[i].body_aim_hitboxes, 8, std::to_string(i) + crypt_str("Ragebot.body_aim_hitboxes"));
		setup_item(&configs.ragebot.weapon[i].head_aim_conditions, 2, std::to_string(i) + crypt_str("Ragebot.head_aim_conditions"));
		setup_item(&configs.ragebot.weapon[i].head_aim_only_conditions, 2, std::to_string(i) + crypt_str("Ragebot.head_aim_only_conditions"));
		setup_item(&configs.ragebot.weapon[i].scan_point_mode, 0, std::to_string(i) + crypt_str("Ragebot.scan_point_mode"));
		setup_item(&configs.ragebot.weapon[i].multipoints_hitboxes, 9, std::to_string(i) + crypt_str("Ragebot.multipoints_hitboxes"));
		setup_item(&configs.ragebot.weapon[i].multipoints_scale, 0.0f, std::to_string(i) + crypt_str("Ragebot.multipoints_scale"));
		setup_item(&configs.ragebot.weapon[i].adaptive_point_scale, false, std::to_string(i) + crypt_str("Ragebot.adaptive_point_scale"));
		setup_item(&configs.ragebot.weapon[i].head_scale, 0.0f, std::to_string(i) + crypt_str("Ragebot.head_scale"));
		setup_item(&configs.ragebot.weapon[i].body_scale, 0.0f, std::to_string(i) + crypt_str("Ragebot.body_scale"));
		setup_item(&configs.ragebot.weapon[i].max_misses, false, std::to_string(i) + crypt_str("Ragebot.max_misses"));
		setup_item(&configs.ragebot.weapon[i].max_misses_amount, 0, std::to_string(i) + crypt_str("Ragebot.max_misses_amount"));
		setup_item(&configs.ragebot.weapon[i].ignore_limbs, false, std::to_string(i) + crypt_str("Ragebot.ignore_limbs"));
		setup_item(&configs.ragebot.weapon[i].dynamic_body_aim_conditions, true, std::to_string(i) + crypt_str("Ragebot.dynamic_body_aim_conditions"));
	}

	setup_item(&configs.antiaim.enabled, false, crypt_str("Antiaim.enabled"));
	setup_item(&configs.antiaim.antiaim_type, 0, crypt_str("Antiaim.antiaim_type"));
	setup_item(&configs.antiaim.desync, 0, crypt_str("Antiaim.desync"));
	setup_item(&configs.antiaim.legit_lby_type, 0, crypt_str("Antiaim.legit_lby_type"));
	setup_item(&configs.antiaim.lby_type, 0, crypt_str("Antiaim.lby_type"));
	setup_item(&configs.antiaim.manual_back, key_bind(TOGGLE), crypt_str("Antiaim.manual_back"));
	setup_item(&configs.antiaim.manual_left, key_bind(TOGGLE), crypt_str("Antiaim.manual_left"));
	setup_item(&configs.antiaim.manual_right, key_bind(TOGGLE), crypt_str("Antiaim.manual_right"));
	setup_item(&configs.antiaim.flip_desync, key_bind(TOGGLE), crypt_str("Antiaim.flip_desync"));
	setup_item(&configs.antiaim.flip_indicator, false, crypt_str("Antiaim.flip_indicator"));
	setup_item(&configs.antiaim.flip_indicator_color, Color::White, crypt_str("Antiaim.flip_indicator_color"));
	setup_item(&configs.antiaim.jitter_move, false, crypt_str("Antiaim.jitter_move"));
	setup_item(&configs.antiaim.auto_direction, false, crypt_str("Antiaim.auto_direction"));
	setup_item(&configs.antiaim.zero_pitch, key_bind(HOLD), crypt_str("Antiaim.zero_pitch"));
	setup_item(&configs.antiaim.fake_flick_180, key_bind(HOLD), crypt_str("Antiaim.fake_flick_180"));

	for (auto i = 0; i < 3; i++)
	{
		setup_item(&configs.antiaim.type[i].pitch, 0, std::to_string(i) + crypt_str("Antiaim.pitch"));
		setup_item(&configs.antiaim.type[i].custom_pitch, 0, std::to_string(i) + crypt_str("Antiaim.custom_pitch"));
		setup_item(&configs.antiaim.type[i].roll, 0, std::to_string(i) + crypt_str("Antiaim.roll"));
		setup_item(&configs.antiaim.type[i].fake_flick, false, std::to_string(i) + crypt_str("Antiaim.fake_flick"));
		setup_item(&configs.antiaim.type[i].fake_flick_disable_with_manual, false, std::to_string(i) + crypt_str("Antiaim.fake_flick_disable_with_manual"));
		setup_item(&configs.antiaim.type[i].base_angle, 0, std::to_string(i) + crypt_str("Antiaim.base_angle"));
		setup_item(&configs.antiaim.type[i].yaw, 0, std::to_string(i) + crypt_str("Antiaim.yaw"));
		setup_item(&configs.antiaim.type[i].custom_yaw, 0, std::to_string(i) + crypt_str("Antiaim.custom_yaw"));
		setup_item(&configs.antiaim.type[i].range, 1, std::to_string(i) + crypt_str("Antiaim.range"));
		setup_item(&configs.antiaim.type[i].yaw_jitter_type, 0, std::to_string(i) + crypt_str("Antiaim.yaw_jitter_type"));
		setup_item(&configs.antiaim.type[i].yaw_jitter_ticks_max, 3, std::to_string(i) + crypt_str("Antiaim.yaw_jitter_ticks_max"));
		setup_item(&configs.antiaim.type[i].yaw_jitter_ticks_min, 3, std::to_string(i) + crypt_str("Antiaim.yaw_jitter_ticks_min"));
		setup_item(&configs.antiaim.type[i].speed, 1, std::to_string(i) + crypt_str("Antiaim.speed"));
		setup_item(&configs.antiaim.type[i].desync, 0, std::to_string(i) + crypt_str("Antiaim.desync"));
		setup_item(&configs.antiaim.type[i].jitter_type, 0, std::to_string(i) + crypt_str("Antiaim.jitter_type"));
		setup_item(&configs.antiaim.type[i].jitter_ticks_max, 3, std::to_string(i) + crypt_str("Antiaim.jitter_ticks_max"));
		setup_item(&configs.antiaim.type[i].jitter_ticks_min, 3, std::to_string(i) + crypt_str("Antiaim.jitter_ticks_min"));
		setup_item(&configs.antiaim.type[i].desync_mode, 0, std::to_string(i) + crypt_str("Antiaim.desync_mode"));
		setup_item(&configs.antiaim.type[i].desync_range, 90, std::to_string(i) + crypt_str("Antiaim.desync_range"));
		setup_item(&configs.antiaim.type[i].inverted_desync_range, 90, std::to_string(i) + crypt_str("Antiaim.inverted_desync_range"));
		//setup_item(&configs.antiaim.type[i].body_lean_mode, 0, std::to_string(i) + crypt_str("Antiaim.body_lean_mode"));
		//setup_item(&configs.antiaim.type[i].body_lean, 0, std::to_string(i) + crypt_str("Antiaim.body_lean"));
		//setup_item(&configs.antiaim.type[i].inverted_body_lean, 0, std::to_string(i) + crypt_str("Antiaim.inverted_body_lean"));
	}

	setup_item(&configs.fakelag.enabled, false, crypt_str("Fakelag.enabled"));
	setup_item(&configs.fakelag.type, 0, crypt_str("Fakelag.type"));
	setup_item(&configs.fakelag.enablers, 4, crypt_str("Fakelag.enablers"));
	setup_item(&configs.fakelag.min_amount, 1, crypt_str("Fakelag.min_limit"));
	setup_item(&configs.fakelag.amount, 1, crypt_str("Fakelag.limit"));
	setup_item(&configs.fakelag.triggers_amount, 1, crypt_str("Fakelag.triggers_limit"));

	setup_item(&configs.radar.enabled, false, crypt_str("Radar.enabled"));
	setup_item(&configs.radar.local, false, crypt_str("Radar.local"));
	setup_item(&configs.radar.enemy, false, crypt_str("Radar.enemy"));
	setup_item(&configs.radar.teammate, false, crypt_str("Radar.teammate"));
	setup_item(&configs.radar.planted_c4, false, crypt_str("Radar.planted_c4"));
	setup_item(&configs.radar.dropped_c4, false, crypt_str("Radar.dropped_c4"));
	setup_item(&configs.radar.health, false, crypt_str("Radar.health"));

	setup_item(&configs.player.enabled, false, crypt_str("Player.enabled"));
	setup_item(&configs.player.arrows_color, Color::White, crypt_str("Player.arrows_color"));
	setup_item(&configs.player.arrows, false, crypt_str("Player.arrows"));
	setup_item(&configs.player.distance, 1, crypt_str("Player.arrows_distance"));
	setup_item(&configs.player.size, 1, crypt_str("Player.arrows_size"));
	setup_item(&configs.player.show_multi_points, false, crypt_str("Player.show_multi_points"));
	setup_item(&configs.player.show_multi_points_color, Color::White, crypt_str("Player.show_multi_points_color"));
	setup_item(&configs.player.lag_hitbox, false, crypt_str("Player.lag_hitbox"));
	setup_item(&configs.player.lag_hitbox_color, Color::White, crypt_str("Player.lag_hitbox_color"));
	setup_item(&configs.player.player_model_t, 0, crypt_str("Esp.player_model_t"));
	setup_item(&configs.player.player_model_ct, 0, crypt_str("Esp.player_model_ct"));
	setup_item(&configs.player.local_chams_type, 0, crypt_str("Player.local_chams_type"));
	setup_item(&configs.player.fake_chams_enabled, false, crypt_str("Player.fake_chams_enabled"));
	setup_item(&configs.player.visualize_lag, false, crypt_str("Player.vizualize_lag"));
	setup_item(&configs.player.layered, false, crypt_str("Player.layered"));
	setup_item(&configs.player.fake_chams_color, Color::White, crypt_str("Player.fake_chams_color"));
	setup_item(&configs.player.fake_chams_type, 0, crypt_str("Player.fake_chams_type"));
	setup_item(&configs.player.fake_double_material, false, crypt_str("Player.fake_double_material"));
	setup_item(&configs.player.fake_double_material_color, Color::White, crypt_str("Player.fake_double_material_color"));
	setup_item(&configs.player.fake_animated_material, false, crypt_str("Player.fake_animated_material"));
	setup_item(&configs.player.fake_animated_material_color, Color::White, crypt_str("Player.fake_animated_material_color"));
	setup_item(&configs.player.backtrack_chams, false, crypt_str("Player.backtrack_chams"));
	setup_item(&configs.player.backtrack_chams_material, 0, crypt_str("Player.backtrack_chams_material"));
	setup_item(&configs.player.backtrack_chams_color, Color::White, crypt_str("Player.backtrack_chams_color"));
	setup_item(&configs.player.transparency_in_scope, false, crypt_str("Player.transparency_in_scope"));
	setup_item(&configs.player.transparency_in_scope_amount, 1.0f, crypt_str("Player.transparency_in_scope_amount"));

	for (auto i = 0; i < 3; i++)
	{
		setup_item(&configs.player.type[i].box, false, std::to_string(i) + crypt_str("Player.box"));
		setup_item(&configs.player.type[i].box_color, Color::White, std::to_string(i) + crypt_str("Player.box_color"));
		setup_item(&configs.player.type[i].name, false, std::to_string(i) + crypt_str("Player.name"));
		setup_item(&configs.player.type[i].name_color, Color::White, std::to_string(i) + crypt_str("Player.name_color"));
		setup_item(&configs.player.type[i].health, false, std::to_string(i) + crypt_str("Player.health"));
		setup_item(&configs.player.type[i].custom_health_color, false, std::to_string(i) + crypt_str("Player.custom_health_color"));
		setup_item(&configs.player.type[i].health_color, Color::White, std::to_string(i) + crypt_str("Player.health_color"));
		setup_item(&configs.player.type[i].weapon, 2, std::to_string(i) + crypt_str("Player.weapon"));
		setup_item(&configs.player.type[i].weapon_color, Color::White, std::to_string(i) + crypt_str("Player.weapon_bar_color"));
		setup_item(&configs.player.type[i].skeleton, false, std::to_string(i) + crypt_str("Player.skeleton"));
		setup_item(&configs.player.type[i].skeleton_color, Color::White, std::to_string(i) + crypt_str("Player.skeleton_color"));
		setup_item(&configs.player.type[i].ammo, false, std::to_string(i) + crypt_str("Player.ammo"));
		setup_item(&configs.player.type[i].ammobar_color, Color::White, std::to_string(i) + crypt_str("Player.ammobar_color"));
		setup_item(&configs.player.type[i].snap_lines, false, std::to_string(i) + crypt_str("Player.snap_lines"));
		setup_item(&configs.player.type[i].snap_lines_color, Color::White, std::to_string(i) + crypt_str("Player.snap_lines_color"));
		setup_item(&configs.player.type[i].footsteps, false, std::to_string(i) + crypt_str("Player.footsteps"));
		setup_item(&configs.player.type[i].footsteps_color, Color::White, std::to_string(i) + crypt_str("Player.footsteps_color"));
		setup_item(&configs.player.type[i].thickness, 0, std::to_string(i) + crypt_str("Player.thickness"));
		setup_item(&configs.player.type[i].radius, 0, std::to_string(i) + crypt_str("Player.radius"));
		setup_item(&configs.player.type[i].glow, false, std::to_string(i) + crypt_str("Player.glow"));
		setup_item(&configs.player.type[i].glow_type, 0, std::to_string(i) + crypt_str("Player.glow_type"));
		setup_item(&configs.player.type[i].glow_color, Color::White, std::to_string(i) + crypt_str("Player.glow_color"));
		setup_item(&configs.player.type[i].chams, 2, std::to_string(i) + crypt_str("Player.chams"));
		setup_item(&configs.player.type[i].chams_color, Color::White, std::to_string(i) + crypt_str("Player.chams_color"));
		setup_item(&configs.player.type[i].xqz_color, Color::White, std::to_string(i) + crypt_str("Player.xqz_color"));
		setup_item(&configs.player.type[i].chams_type, 0, std::to_string(i) + crypt_str("Player.chams_type"));
		setup_item(&configs.player.type[i].double_material, false, std::to_string(i) + crypt_str("Player.double_material"));
		setup_item(&configs.player.type[i].double_material_color, Color::White, std::to_string(i) + crypt_str("Player.double_material_color"));
		setup_item(&configs.player.type[i].animated_material, false, std::to_string(i) + crypt_str("Player.animated_material"));
		setup_item(&configs.player.type[i].animated_material_color, Color::White, std::to_string(i) + crypt_str("Player.animated_material_color"));
		setup_item(&configs.player.type[i].ragdoll_chams, false, std::to_string(i) + crypt_str("Player.ragdoll_chams"));
		setup_item(&configs.player.type[i].ragdoll_chams_material, 0, std::to_string(i) + crypt_str("Player.ragdoll_chams_material"));
		setup_item(&configs.player.type[i].ragdoll_chams_color, Color::White, std::to_string(i) + crypt_str("Player.ragdoll_chams_color"));
		setup_item(&configs.player.type[i].flags, 8, std::to_string(i) + crypt_str("Player.esp_flags"));
	}

	setup_item(&configs.esp.indicators, 11, crypt_str("Esp.rage_indicators"));
	setup_item(&configs.esp.removals, 11, crypt_str("Esp.world_removals"));
	setup_item(&configs.esp.fix_zoom_sensivity, false, crypt_str("Esp.fix_zoom_sensivity"));
	setup_item(&configs.esp.grenade_warning, false, crypt_str("Visuals.grenade_warning"));
	setup_item(&configs.esp.grenade_prediction, false, crypt_str("Esp.grenade_prediction"));
	setup_item(&configs.esp.grenade_prediction_click_only, false, crypt_str("Esp.grenade_prediction_click_only"));
	setup_item(&configs.esp.grenade_prediction_color, Color::White, crypt_str("Esp.grenade_prediction_color"));
	setup_item(&configs.esp.grenade_prediction_tracer_color, Color::White, crypt_str("Esp.grenade_prediction_tracer_color"));
	setup_item(&configs.esp.projectiles, false, crypt_str("Esp.projectiles"));
	setup_item(&configs.esp.projectiles_color, Color::White, crypt_str("Esp.projectiles_color"));
	setup_item(&configs.esp.molotov_timer, false, crypt_str("Esp.molotov_timer"));
	setup_item(&configs.esp.molotov_timer_color, Color::White, crypt_str("Esp.molotov_timer_color"));
	setup_item(&configs.esp.smoke_timer, false, crypt_str("Esp.smoke_timer"));
	setup_item(&configs.esp.smoke_timer_color, Color::White, crypt_str("Esp.smoke_timer_color"));
	setup_item(&configs.esp.bomb_timer, false, crypt_str("Esp.bomb_timer"));
	setup_item(&configs.esp.bright, false, crypt_str("Esp.bright"));
	setup_item(&configs.esp.nightmode, false, crypt_str("Esp.nightmode"));
	setup_item(&configs.esp.world_color, Color::White, crypt_str("Esp.world_color"));
	setup_item(&configs.esp.props_color, Color::White, crypt_str("Esp.props_color"));
	setup_item(&configs.esp.skybox_changer, 0, crypt_str("Esp.skybox_changer"));
	setup_item(&configs.esp.custom_skybox, crypt_str(""), crypt_str("Esp.custom_skybox"));
	setup_item(&configs.esp.skybox_color, Color::White, crypt_str("Esp.skybox_color"));
	setup_item(&configs.esp.client_bullet_impacts, false, crypt_str("Esp.client_bullet_impacts"));
	setup_item(&configs.esp.client_bullet_impacts_color, Color::White, crypt_str("Esp.client_bullet_impacts_color"));
	setup_item(&configs.esp.server_bullet_impacts, false, crypt_str("Esp.server_bullet_impacts"));
	setup_item(&configs.esp.server_bullet_impacts_color, Color::White, crypt_str("Esp.server_bullet_impacts_color"));
	setup_item(&configs.esp.bullet_tracer, false, crypt_str("Esp.bullet_tracer"));
	setup_item(&configs.esp.bullet_tracer_color, Color::White, crypt_str("Esp.bullet_tracer_color"));
	setup_item(&configs.esp.enemy_bullet_tracer, false, crypt_str("Esp.enemy_bullet_tracer"));
	setup_item(&configs.esp.enemy_bullet_tracer_color, Color::White, crypt_str("Esp.enemy_bullet_tracer_color"));
	setup_item(&configs.esp.preserve_killfeed, false, crypt_str("Esp.preserve_killfeed"));
	setup_item(&configs.esp.hitmarker, 2, crypt_str("Esp.hit_marker"));
	setup_item(&configs.esp.hitsound, 0, crypt_str("Esp.hitsound"));
	setup_item(&configs.esp.custom_hitsound, crypt_str(""), crypt_str("Esp.custom_hitsound"));
	setup_item(&configs.esp.killsound, 0, crypt_str("Esp.killsound"));
	setup_item(&configs.esp.custom_killsound, crypt_str(""), crypt_str("Esp.custom_killsound"));
	setup_item(&configs.esp.damage_marker, false, crypt_str("Esp.damage_marker"));
	setup_item(&configs.esp.kill_effect, false, crypt_str("Esp.kill_effect"));
	setup_item(&configs.esp.kill_effect_duration, 3.0f, crypt_str("Esp.kill_effect_duration"));
	setup_item(&configs.esp.force_crosshair, false, crypt_str("Esp.force_crosshair"));
	setup_item(&configs.esp.fov, 0, crypt_str("Esp.fov"));
	setup_item(&configs.esp.viewmodel_fov, 0, crypt_str("Esp.viewmodel_fov"));
	setup_item(&configs.esp.viewmodel_x, 0, crypt_str("Esp.viewmodel_x"));
	setup_item(&configs.esp.viewmodel_y, 0, crypt_str("Esp.viewmodel_y"));
	setup_item(&configs.esp.viewmodel_z, 0, crypt_str("Esp.viewmodel_z"));
	setup_item(&configs.esp.viewmodel_roll, 0, crypt_str("Esp.viewmodel_roll"));
	setup_item(&configs.esp.arms_chams, false, crypt_str("Esp.arms_chams"));
	setup_item(&configs.esp.arms_chams_type, 0, crypt_str("Esp.arms_chams_type"));
	setup_item(&configs.esp.arms_chams_color, Color::White, crypt_str("Esp.arms_chams_color"));
	setup_item(&configs.esp.arms_double_material, false, crypt_str("Esp.arms_double_material"));
	setup_item(&configs.esp.arms_double_material_color, Color::White, crypt_str("Esp.arms_double_material_color"));
	setup_item(&configs.esp.arms_animated_material, false, crypt_str("Esp.arms_animated_material"));
	setup_item(&configs.esp.arms_animated_material_color, Color::White, crypt_str("Esp.arms_animated_material_color"));
	setup_item(&configs.esp.weapon_chams, false, crypt_str("Esp.weapon_chams"));
	setup_item(&configs.esp.weapon_chams_type, 0, crypt_str("Esp.weapon_chams_type"));
	setup_item(&configs.esp.weapon_chams_color, Color::White, crypt_str("Esp.weapon_chams_color"));
	setup_item(&configs.esp.weapon_double_material, false, crypt_str("Esp.weapon_double_material"));
	setup_item(&configs.esp.weapon_double_material_color, Color::White, crypt_str("Esp.weapon_double_material_color"));
	setup_item(&configs.esp.weapon_animated_material, false, crypt_str("Esp.weapon_animated_material"));
	setup_item(&configs.esp.weapon_animated_material_color, Color::White, crypt_str("Esp.weapon_animated_material_color"));
	setup_item(&configs.esp.taser_range, false, crypt_str("Esp.taser_range"));
	setup_item(&configs.esp.show_spread, false, crypt_str("Esp.show_spread"));
	setup_item(&configs.esp.show_spread_color, Color::White, crypt_str("Esp.show_spread_color"));
	setup_item(&configs.esp.penetration_reticle, false, crypt_str("Esp.penetration_reticle"));
	setup_item(&configs.esp.world_modulation, false, crypt_str("Esp.world_modulation"));
	setup_item(&configs.esp.rain, false, crypt_str("Esp.rain"));
	setup_item(&configs.esp.bloom, 0.0f, crypt_str("Esp.bloom"));
	setup_item(&configs.esp.exposure, 0.0f, crypt_str("Esp.exposure"));
	setup_item(&configs.esp.ambient, 0.0f, crypt_str("Esp.ambient"));
	setup_item(&configs.esp.fog, false, crypt_str("Fog.enabled"));
	setup_item(&configs.esp.fog_distance, 0, crypt_str("Fog.distance"));
	setup_item(&configs.esp.fog_density, 0, crypt_str("Fog.density"));
	setup_item(&configs.esp.fog_color, Color::White, crypt_str("Fog.color"));
	setup_item(&configs.esp.motionblur, false, crypt_str("Motionblur.enabled"));
	setup_item(&configs.esp.motionblur_forward, false, crypt_str("Motionblur.forward"));
	setup_item(&configs.esp.motionblur_falling_min, 10, crypt_str("Motionblur.falling_min"));
	setup_item(&configs.esp.motionblur_falling_max, 20, crypt_str("Motionblur.falling_max"));
	setup_item(&configs.esp.motionblur_falling_intensity, 1, crypt_str("Motionblur.falling_intensity"));
	setup_item(&configs.esp.motionblur_rotation_intensity, 1, crypt_str("Motionblur.rotation_intensity"));
	setup_item(&configs.esp.motionblur_strength, 1, crypt_str("Motionblur.strength"));
	setup_item(&configs.esp.lightning_on_hit, false, crypt_str("Lightning.enabled"));
	setup_item(&configs.esp.lightning_on_hit_killonly, false, crypt_str("Lightning.kill_only"));
	setup_item(&configs.esp.lightning_on_hit_width, 10, crypt_str("Lightning.width"));
	setup_item(&configs.esp.lightning_on_hit_radius, 500, crypt_str("Lightning.radius"));
	setup_item(&configs.esp.lightning_on_hit_beams, 12, crypt_str("Lightning.beams"));
	setup_item(&configs.esp.smoke_color_enabled, false, crypt_str("Smokecolor.enabled"));
	setup_item(&configs.esp.smoke_color, Color::White, crypt_str("Smokecolor.color"));
	setup_item(&configs.esp.molotov_color_enabled, false, crypt_str("Molotovcolor.enabled"));
	setup_item(&configs.esp.molotov_color, Color::White, crypt_str("Molotovcolor.color"));
	setup_item(&configs.esp.console_color_enabled, false, crypt_str("Consolecolor.enabled"));
	setup_item(&configs.esp.console_color, Color::White, crypt_str("Consolecolor.color"));

	setup_item(&configs.esp.weapon, 6, crypt_str("Esp.dropped_weapon"));
	setup_item(&configs.esp.box_color, Color::White, crypt_str("Esp.dropped_weapon_box_color"));
	setup_item(&configs.esp.weapon_color, Color::White, crypt_str("Esp.dropped_weapon_color"));
	setup_item(&configs.esp.weapon_glow_color, Color::White, crypt_str("Esp.dropped_weapon_glow_color"));
	setup_item(&configs.esp.weapon_ammo_color, Color::White, crypt_str("Esp.dropped_weapon_ammo_color"));
	setup_item(&configs.esp.grenade_esp, 4, crypt_str("Esp.grenade_esp"));
	setup_item(&configs.esp.grenade_glow_color, Color::White, crypt_str("Esp.grenade_glow_color"));
	setup_item(&configs.esp.grenade_box_color, Color::White, crypt_str("Esp.grenade_box_color"));

	setup_item(&configs.killfeedChanger.enabled, false, crypt_str("KillfeedChanger.enabled"));
	setup_item(&configs.killfeedChanger.headshot, false, crypt_str("KillfeedChanger.headshot"));
	setup_item(&configs.killfeedChanger.dominated, false, crypt_str("KillfeedChanger.dominated"));
	setup_item(&configs.killfeedChanger.revenge, false, crypt_str("KillfeedChanger.revenge"));
	setup_item(&configs.killfeedChanger.penetrated, false, crypt_str("KillfeedChanger.penetrated"));
	setup_item(&configs.killfeedChanger.noscope, false, crypt_str("KillfeedChanger.noscope"));
	setup_item(&configs.killfeedChanger.thrusmoke, false, crypt_str("KillfeedChanger.thrusmoke"));
	setup_item(&configs.killfeedChanger.attackerblind, false, crypt_str("KillfeedChanger.attackerblind"));

	setup_item(&configs.misc.thirdperson_toggle, key_bind(TOGGLE), crypt_str("Misc.thirdperson_toggle"));
	setup_item(&configs.misc.thirdperson_when_spectating, false, crypt_str("Misc.thirdperson_when_spectating"));
	setup_item(&configs.misc.smooth_thirdperson, false, crypt_str("Misc.smooth_thirdperson"));
	setup_item(&configs.misc.keep_thirdperson, true, crypt_str("Misc.keep_thirdperson"));
	setup_item(&configs.misc.thirdperson_distance, 100, crypt_str("Misc.thirdperson_distance"));
	setup_item(&configs.misc.spectators_list, false, crypt_str("Misc.spectators_list"));
	setup_item(&configs.misc.spectatorlist_pos_x, 0.0f, crypt_str("Misc.spectatorlist_pos_x"));
	setup_item(&configs.misc.spectatorlist_pos_y, 0.0f, crypt_str("Misc.spectatorlist_pos_y"));
	setup_item(&configs.misc.keybind_list, false, crypt_str("Misc.keybind_list"));
	setup_item(&configs.misc.keybinds_pos_x, 0.0f, crypt_str("Misc.keybinds_pos_x"));
	setup_item(&configs.misc.keybinds_pos_y, 0.0f, crypt_str("Misc.keybinds_pos_y"));
	setup_item(&configs.misc.ragdolls, false, crypt_str("Misc.ragdolls"));
	setup_item(&configs.misc.sv_pure_bypass, false, crypt_str("Misc.sv_pure_bypass"));
	setup_item(&configs.misc.bunnyhop, false, crypt_str("Misc.autohop"));
	setup_item(&configs.misc.airstrafe, 0, crypt_str("Misc.airstrafe"));
	setup_item(&configs.misc.force_relay_cluster, 0, crypt_str("Misc.force_relay_cluster"));
	setup_item(&configs.misc.crouch_in_air, false, crypt_str("Misc.crouch_in_air"));
	setup_item(&configs.misc.automatic_peek, key_bind(HOLD), crypt_str("Misc.automatic_peek"));
	setup_item(&configs.misc.automatic_peek_color, Color::White, crypt_str("Misc.automatic_peek_color"));
	setup_item(&configs.misc.edge_jump, key_bind(HOLD), crypt_str("Misc.edge_jump"));
	setup_item(&configs.misc.jump_bug, key_bind(HOLD), crypt_str("Misc.jump_bug"));
	setup_item(&configs.misc.long_jump, key_bind(HOLD), crypt_str("Misc.long_jump"));
	setup_item(&configs.misc.mini_jump, key_bind(HOLD), crypt_str("Misc.mini_jump"));
	setup_item(&configs.misc.no_duck_cooldown, false, crypt_str("Misc.no_duck_cooldown"));
	setup_item(&configs.misc.fake_duck_key, key_bind(HOLD), crypt_str("Misc.fake_duck_key"));
	setup_item(&configs.misc.fast_stop, false, crypt_str("Misc.fast_stop"));
	setup_item(&configs.misc.slide_walk, 0, crypt_str("Misc.slide_walk"));
	setup_item(&configs.misc.server_hitbox, false, crypt_str("Misc.server_hitbox"));
	setup_item(&configs.misc.slowwalk_key, key_bind(HOLD), crypt_str("Misc.slowwalk_key"));
	setup_item(&configs.misc.log_output, 3, crypt_str("Misc.log_output"));
	setup_item(&configs.misc.events_to_log, 3, crypt_str("Misc.events_to_log"));
	//setup_item(&configs.misc.show_default_log, true, crypt_str("Misc.show_default_log"));
	setup_item(&configs.misc.log_color, Color::White, crypt_str("Misc.log_color"));
	setup_item(&configs.misc.inventory_access, false, crypt_str("Misc.inventory_access"));
	setup_item(&configs.misc.reveal_votes, false, crypt_str("Misc.reveal_votes"));
	setup_item(&configs.misc.rank_reveal, false, crypt_str("Misc.rank_reveal"));
	setup_item(&configs.misc.clan_tag, false, crypt_str("Misc.clan_tag"));
	setup_item(&configs.misc.radar_hack, false, crypt_str("Misc.radar_hack"));
	setup_item(&configs.misc.reveal_money, false, crypt_str("Misc.reveal_money"));
	setup_item(&configs.misc.reveal_suspect, false, crypt_str("Misc.reveal_suspect"));
	setup_item(&configs.misc.auto_accept, false, crypt_str("Misc.auto_accept"));
	setup_item(&configs.misc.anti_crasher, false, crypt_str("Misc.anti_crasher"));
	setup_item(&configs.misc.anti_exploit, false, crypt_str("Misc.anti_exploit"));
	setup_item(&configs.misc.anti_afk, false, crypt_str("Misc.anti_afk"));
	setup_item(&configs.misc.ad_block, false, crypt_str("Misc.ad_block"));
	setup_item(&configs.misc.slider_to_input, false, crypt_str("Misc.slider_to_input"));
	setup_item(&configs.misc.aspect_ratio, 0.0f, crypt_str("Misc.aspect_ratio"));
	setup_item(&configs.misc.air_static_legs, false, crypt_str("Misc.air_static_legs"));
	setup_item(&configs.misc.anti_screenshot, false, crypt_str("Misc.anti_screenshot"));
	setup_item(&configs.misc.anti_untrusted, true, crypt_str("Misc.antiuntrusted"));
	setup_item(&configs.misc.walkbot, 0, crypt_str("Misc.walkbot"));
	setup_item(&configs.misc.walkbot_key, key_bind(HOLD), crypt_str("Misc.walkbot_key"));
	setup_item(&configs.misc.buybot_enabled, false, crypt_str("Misc.buybot_enabled"));
	setup_item(&configs.misc.buybot1, 0, crypt_str("Misc.buybot1"));
	setup_item(&configs.misc.buybot2, 0, crypt_str("Misc.buybot2"));
	setup_item(&configs.misc.buybot3, 4, crypt_str("Misc.buybot3"));

	setup_item(&configs.blockbot.enabled, false, crypt_str("Blockbot.enabled"));
	setup_item(&configs.blockbot.key, key_bind(HOLD), crypt_str("Blockbot.key"));
	setup_item(&configs.blockbot.type, 0, crypt_str("Blockbot.type"));

	setup_item(&configs.skins.deagleSpinner, false, crypt_str("Skins.deagle_spinner"));

	for (auto i = 0; i < configs.skins.skinChanger.size(); i++)
	{
		setup_item(&configs.skins.skinChanger.at(i).definition_override_index, 0, std::to_string(i) + crypt_str("Skins.definition_override_index"));
		setup_item(&configs.skins.skinChanger.at(i).definition_override_vector_index, 0, std::to_string(i) + crypt_str("Skins.definition_override_vector_index"));
		setup_item(&configs.skins.skinChanger.at(i).entity_quality_vector_index, 0, std::to_string(i) + crypt_str("Skins.entity_quality_vector_index"));
		setup_item(&configs.skins.skinChanger.at(i).itemId, 0, std::to_string(i) + crypt_str("Skins.itemId"));
		setup_item(&configs.skins.skinChanger.at(i).itemIdIndex, 0, std::to_string(i) + crypt_str("Skins.itemIdIndex"));
		setup_item(&configs.skins.skinChanger.at(i).paintKit, 0, std::to_string(i) + crypt_str("Skins.paintKit"));
		setup_item(&configs.skins.skinChanger.at(i).paint_kit_vector_index, 0, std::to_string(i) + crypt_str("Skins.paint_kit_vector_index"));
		setup_item(&configs.skins.skinChanger.at(i).quality, 0, std::to_string(i) + crypt_str("Skins.quality"));
		setup_item(&configs.skins.skinChanger.at(i).seed, 0, std::to_string(i) + crypt_str("Skins.seed"));
		setup_item(&configs.skins.skinChanger.at(i).stat_trak, 0, std::to_string(i) + crypt_str("Skins.stat_trak"));
		setup_item(&configs.skins.skinChanger.at(i).wear, 0.0f, std::to_string(i) + crypt_str("Skins.wear"));
		setup_item(&configs.skins.custom_name_tag[i], crypt_str(""), std::to_string(i) + crypt_str("Skins.custom_name_tag"));
	}

	setup_item(&configs.profileChanger.enabled, false, crypt_str("Profile.enabled"));
	setup_item(&configs.profileChanger.friendly, 0, crypt_str("Profile.friendly"));
	setup_item(&configs.profileChanger.teacher, 0, crypt_str("Profile.teacher"));
	setup_item(&configs.profileChanger.leader, 0, crypt_str("Profile.leader"));
	setup_item(&configs.profileChanger.rank, 0, crypt_str("Profile.rank"));
	setup_item(&configs.profileChanger.wins, 0, crypt_str("Profile.wins"));
	setup_item(&configs.profileChanger.r_rank, 0, crypt_str("Profile.r_rank"));
	setup_item(&configs.profileChanger.r_wins, 0, crypt_str("Profile.r_wins"));
	setup_item(&configs.profileChanger.t_rank, 0, crypt_str("Profile.t_rank"));
	setup_item(&configs.profileChanger.t_wins, 0, crypt_str("Profile.t_wins"));
	setup_item(&configs.profileChanger.level, 0, crypt_str("Profile.level"));
	setup_item(&configs.profileChanger.exp, 0, crypt_str("Profile.exp"));
	setup_item(&configs.profileChanger.ban_type, 0, crypt_str("Profile.ban_type"));
	setup_item(&configs.profileChanger.ban_time, 0, crypt_str("Profile.ban_time"));
	setup_item(&configs.profileChanger.fake_prime, false, crypt_str("Profile.fake_prime"));
	setup_item(&configs.profileChanger.musickit, 0, crypt_str("Profile.music"));
	setup_item(&configs.profileChanger.musickit_stattrak, -1, crypt_str("Profile.music_stattrak"));

	setup_item(&configs.menu.menu_theme, Color(164, 204, 30), crypt_str("Menu.menu_color"));
	setup_item(&configs.menu.watermark, 0, crypt_str("Menu.watermark"));
	setup_item(&configs.menu.music_status, false, crypt_str("Menu.music_status"));
	setup_item(&configs.menu.key, VK_HOME, crypt_str("Menu.key"));

	setup_item(&configs.scripts.scripts, crypt_str("Scripts.loaded"));
}

void C_ConfigManager::add_item(void* pointer, const char* name, const std::string& type) {
	items.push_back(new C_ConfigItem(std::string(name), pointer, type));
}

void C_ConfigManager::setup_item(int* pointer, int value, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("int"));
	*pointer = value;
}

void C_ConfigManager::setup_item(bool* pointer, bool value, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("bool"));
	*pointer = value;
}

void C_ConfigManager::setup_item(float* pointer, float value, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("float"));
	*pointer = value;
}

void C_ConfigManager::setup_item(key_bind* pointer, key_bind value, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("KeyBind"));
	*pointer = value;
}

void C_ConfigManager::setup_item(Color* pointer, Color value, const std::string& name)
{
	colors[name][0] = (float)value.r() / 255.0f;
	colors[name][1] = (float)value.g() / 255.0f;
	colors[name][2] = (float)value.b() / 255.0f;
	colors[name][3] = (float)value.a() / 255.0f;

	add_item(pointer, name.c_str(), crypt_str("Color"));
	*pointer = value;
}

void C_ConfigManager::setup_item(std::vector<int>* pointer, int size, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("vector<int>"));
	pointer->clear();

	for (int i = 0; i < size; i++)
		pointer->push_back(FALSE);
}

void C_ConfigManager::setup_item(std::vector<std::string>* pointer, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("vector<string>"));
}

void C_ConfigManager::setup_item(std::string* pointer, const std::string& value, const std::string& name)
{
	add_item(pointer, name.c_str(), crypt_str("string"));
	*pointer = value;
}

void C_ConfigManager::save(std::string config)
{
	std::string file = get_folder_path(PathType::Configs) + config;

	json allJson;

	for (auto it : items)
	{
		json j;

		j[crypt_str("name")] = it->name;
		j[crypt_str("type")] = it->type;

		if (!it->type.compare(crypt_str("int")))
			j[crypt_str("value")] = (int)*(int*)it->pointer;
		else if (!it->type.compare(crypt_str("float")))
			j[crypt_str("value")] = (float)*(float*)it->pointer;
		else if (!it->type.compare(crypt_str("bool")))
			j[crypt_str("value")] = (bool)*(bool*)it->pointer;
		else if (!it->type.compare(crypt_str("KeyBind")))
		{
			auto k = *(key_bind*)(it->pointer);

			std::vector <int> a = { k.key, k.mode };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[crypt_str("value")] = ja.dump();
		}
		else if (!it->type.compare(crypt_str("Color")))
		{
			auto c = *(Color*)(it->pointer);

			std::vector<int> a = { c.r(), c.g(), c.b(), c.a() };
			json ja;

			for (auto& i : a)
				ja.push_back(i);

			j[crypt_str("value")] = ja.dump();
		}
		else if (!it->type.compare(crypt_str("vector<int>")))
		{
			auto& ptr = *(std::vector<int>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[crypt_str("value")] = ja.dump();
		}
		else if (!it->type.compare(crypt_str("vector<string>")))
		{
			auto& ptr = *(std::vector<std::string>*)(it->pointer);
			json ja;

			for (auto& i : ptr)
				ja.push_back(i);

			j[crypt_str("value")] = ja.dump();
		}
		else if (!it->type.compare(crypt_str("string")))
			j[crypt_str("value")] = (std::string) * (std::string*)it->pointer;

		allJson.push_back(j);
	}

	auto get_type = [](menu_item_type type)
	{
		switch (type)
		{
		case CHECK_BOX:
			return "bool";
		case COMBO_BOX:
		case SLIDER_INT:
			return "int";
		case SLIDER_FLOAT:
			return "float";
		case COLOR_PICKER:
			return "Color";
		}
	};

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto& script = c_lua::get().scripts.at(i);

		for (auto& item : c_lua::get().items.at(i))
		{
			if (item.second.type == NEXT_LINE)
				continue;

			json j;
			auto type = (std::string)get_type(item.second.type);

			j[crypt_str("name")] = item.first;
			j[crypt_str("type")] = type;

			if (!type.compare(crypt_str("bool")))
				j[crypt_str("value")] = item.second.check_box_value;
			else if (!type.compare(crypt_str("int")))
				j[crypt_str("value")] = item.second.type == COMBO_BOX ? item.second.combo_box_value : item.second.slider_int_value;
			else if (!type.compare(crypt_str("float")))
				j[crypt_str("value")] = item.second.slider_float_value;
			else if (!type.compare(crypt_str("Color")))
			{
				std::vector <int> color =
				{
					item.second.color_picker_value.r(),
					item.second.color_picker_value.g(),
					item.second.color_picker_value.b(),
					item.second.color_picker_value.a()
				};

				json j_color;

				for (auto& i : color)
					j_color.push_back(i);

				j[crypt_str("value")] = j_color.dump();
			}

			allJson.push_back(j);
		}
	}

	std::string data;

	Base64 base64;
	base64.encode(allJson.dump(), &data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << std::setw(4) << data << std::endl;
	ofs.close();
}

void C_ConfigManager::load(std::string config, bool load_script_items)
{
	static auto find_item = [](std::vector< C_ConfigItem* > items, std::string name) -> C_ConfigItem*
	{
		for (int i = 0; i < (int)items.size(); i++)
			if (!items[i]->name.compare(name))
				return items[i];

		return nullptr;
	};

	std::string file = get_folder_path(PathType::Configs) + config;

	std::string data;

	std::ifstream ifs;
	ifs.open(file + '\0');

	ifs >> data;
	ifs.close();

	if (data.empty())
		return;

	Base64 base64;

	std::string decoded_data;
	base64.decode(data, &decoded_data);

	std::ofstream ofs;
	ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	ofs << decoded_data;
	ofs.close();

	json allJson;

	std::ifstream ifs_final;
	ifs_final.open(file + '\0');

	ifs_final >> allJson;
	ifs_final.close();

	base64.encode(allJson.dump(), &data);

	std::ofstream final_ofs;
	final_ofs.open(file + '\0', std::ios::out | std::ios::trunc);

	final_ofs << data;
	final_ofs.close();

	for (auto it = allJson.begin(); it != allJson.end(); ++it)
	{
		json j = *it;

		std::string name = j[crypt_str("name")];
		std::string type = j[crypt_str("type")];

		auto script_item = std::count_if(name.begin(), name.end(),
			[](char& c)
			{
				return c == '.';
			}
		) >= 2;

		if (load_script_items && script_item)
		{
			std::string script_name;
			auto first_point = false;

			for (auto& c : name)
			{
				if (c == '.')
				{
					if (first_point)
						break;
					else
						first_point = true;
				}

				script_name.push_back(c);
			}

			auto script_id = c_lua::get().get_script_id(script_name);

			if (script_id == -1)
				continue;

			for (auto& current_item : c_lua::get().items.at(script_id))
			{
				if (current_item.first == name)
				{
					if (!type.compare(crypt_str("bool")))
					{
						current_item.second.type = CHECK_BOX;
						current_item.second.check_box_value = j[crypt_str("value")].get<bool>();
					}
					else if (!type.compare(crypt_str("int")))
					{
						if (current_item.second.type == COMBO_BOX)
							current_item.second.combo_box_value = j[crypt_str("value")].get<int>();
						else
							current_item.second.slider_int_value = j[crypt_str("value")].get<int>();
					}
					else if (!type.compare(crypt_str("float")))
						current_item.second.slider_float_value = j[crypt_str("value")].get<float>();
					else if (!type.compare(crypt_str("Color")))
					{
						std::vector<int> a;
						json ja = json::parse(j[crypt_str("value")].get<std::string>().c_str());

						for (json::iterator it = ja.begin(); it != ja.end(); ++it)
							a.push_back(*it);

						colors.erase(name);
						current_item.second.color_picker_value = Color(a[0], a[1], a[2], a[3]);
					}
				}
			}
		}
		else if (!load_script_items && !script_item)
		{
			auto item = find_item(items, name);

			if (item)
			{
				if (!type.compare(crypt_str("int")))
					*(int*)item->pointer = j[crypt_str("value")].get<int>();
				else if (!type.compare(crypt_str("float")))
					*(float*)item->pointer = j[crypt_str("value")].get<float>();
				else if (!type.compare(crypt_str("bool")))
					*(bool*)item->pointer = j[crypt_str("value")].get<bool>();
				else if (!type.compare(crypt_str("KeyBind")))
				{
					std::vector<int> a;
					json ja = json::parse(j[crypt_str("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					*(key_bind*)item->pointer = key_bind((ButtonCode_t)a[0], (key_bind_mode)a[1]);
				}
				else if (!type.compare(crypt_str("Color")))
				{
					std::vector<int> a;
					json ja = json::parse(j[crypt_str("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						a.push_back(*it);

					colors.erase(item->name);
					*(Color*)item->pointer = Color(a[0], a[1], a[2], a[3]);
				}
				else if (!type.compare(crypt_str("vector<int>")))
				{
					auto ptr = static_cast<std::vector <int>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[crypt_str("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(crypt_str("vector<string>")))
				{
					auto ptr = static_cast<std::vector <std::string>*> (item->pointer);
					ptr->clear();

					json ja = json::parse(j[crypt_str("value")].get<std::string>().c_str());

					for (json::iterator it = ja.begin(); it != ja.end(); ++it)
						ptr->push_back(*it);
				}
				else if (!type.compare(crypt_str("string")))
					*(std::string*)item->pointer = j[crypt_str("value")].get<std::string>();
			}
		}
	}
}

void C_ConfigManager::remove(std::string config)
{
	std::remove((get_folder_path(PathType::Configs) + config + '\0').c_str());
}

void C_ConfigManager::config_files()
{
	std::string folder = get_folder_path(PathType::Configs);

	files.clear();

	std::string path = folder + crypt_str("*.cfg");
	WIN32_FIND_DATAA fd;

	HANDLE hFind = FindFirstFileA(path.c_str(), &fd);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		do
		{
			if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
				files.push_back(fd.cFileName);
		} while (FindNextFileA(hFind, &fd));

		FindClose(hFind);
	}
}

std::string C_ConfigManager::get_folder_path(PathType type)
{
	char filePath[255];
	GetModuleFileNameA(NULL, filePath, 255);
	strrchr(filePath, '\\')[1] = 0;

	std::string path = std::string(filePath) + "\\" + cheat.cheatname + "\\";

	if (type != PathType::Default)
		if (type == PathType::Walkbot)
			path += crypt_str("WalkBot\\");
		else if (type == PathType::JavaScript)
			path += crypt_str("JavaScripts\\");
		else if (type == PathType::Lua)
			path += crypt_str("Scripts\\");
		else if (type == PathType::Dump)
			path += crypt_str("Dump\\");
		else
			path += crypt_str("Configs\\");

	CreateDirectoryA(path.c_str(), NULL);

	return path;
}