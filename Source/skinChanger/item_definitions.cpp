#include "item_definitions.hpp"
// These are std::vectors because else I'd have to write their size in the header or write my own container

// We need these for overriding viewmodels and icons
const game_data::weapon_info* game_data::get_weapon_info(int defindex)
{
	const static std::map<int, weapon_info> info =
	{
		{WEAPON_KNIFE,{"models/weapons/w_knife_default_ct.mdl", "models/weapons/v_knife_default_ct.mdl", "knife_default_ct"}},
		{WEAPON_KNIFE_T,{"models/weapons/w_knife_default_t.mdl", "models/weapons/v_knife_default_t.mdl", "knife_t"}},
		{WEAPON_KNIFE_BAYONET, {"models/weapons/w_knife_bayonet.mdl", "models/weapons/v_knife_bayonet.mdl", "bayonet"}},
		{WEAPON_KNIFE_CSS, {"models/weapons/w_knife_css.mdl", "models/weapons/v_knife_css.mdl", "knife_css"}},
		{WEAPON_KNIFE_FLIP, {"models/weapons/w_knife_flip.mdl", "models/weapons/v_knife_flip.mdl", "knife_flip"}},
		{WEAPON_KNIFE_GUT, {"models/weapons/w_knife_gut.mdl", "models/weapons/v_knife_gut.mdl", "knife_gut"}},
		{WEAPON_KNIFE_KARAMBIT, {"models/weapons/w_knife_karam.mdl", "models/weapons/v_knife_karam.mdl", "knife_karambit"}},
		{WEAPON_KNIFE_M9_BAYONET, {"models/weapons/w_knife_m9_bay.mdl", "models/weapons/v_knife_m9_bay.mdl", "knife_m9_bayonet"}},
		{WEAPON_KNIFE_TACTICAL, {"models/weapons/w_knife_tactical.mdl", "models/weapons/v_knife_tactical.mdl", "knife_tactical"}},
		{WEAPON_KNIFE_FALCHION, {"models/weapons/w_knife_falchion_advanced.mdl", "models/weapons/v_knife_falchion_advanced.mdl", "knife_falchion"}},
		{WEAPON_KNIFE_SURVIVAL_BOWIE, {"models/weapons/w_knife_survival_bowie.mdl", "models/weapons/v_knife_survival_bowie.mdl", "knife_survival_bowie"}},
		{WEAPON_KNIFE_BUTTERFLY, {"models/weapons/w_knife_butterfly.mdl", "models/weapons/v_knife_butterfly.mdl", "knife_butterfly"}},
		{WEAPON_KNIFE_PUSH, {"models/weapons/w_knife_push.mdl", "models/weapons/v_knife_push.mdl", "knife_push"}},
		{WEAPON_KNIFE_CORD, {"models/weapons/w_knife_cord.mdl", "models/weapons/v_knife_cord.mdl", "knife_cord"}},
		{WEAPON_KNIFE_CANIS, {"models/weapons/w_knife_canis.mdl", "models/weapons/v_knife_canis.mdl", "knife_canis"}},
		{WEAPON_KNIFE_URSUS,{"models/weapons/w_knife_ursus.mdl", "models/weapons/v_knife_ursus.mdl", "knife_ursus"}},
		{WEAPON_KNIFE_GYPSY_JACKKNIFE,{"models/weapons/w_knife_gypsy_jackknife.mdl", "models/weapons/v_knife_gypsy_jackknife.mdl", "knife_gypsy_jackknife"}},
		{WEAPON_KNIFE_OUTDOOR,{"models/weapons/w_knife_outdoor.mdl", "models/weapons/v_knife_outdoor.mdl", "knife_outdoor"}},
		{WEAPON_KNIFE_STILETTO,{"models/weapons/w_knife_stiletto.mdl", "models/weapons/v_knife_stiletto.mdl", "knife_stiletto"}},
		{WEAPON_KNIFE_WIDOWMAKER,{"models/weapons/w_knife_widowmaker.mdl", "models/weapons/v_knife_widowmaker.mdl", "knife_widowmaker"}},
		{WEAPON_KNIFE_SKELETON,{"models/weapons/w_knife_skeleton.mdl", "models/weapons/v_knife_skeleton.mdl", "knife_skeleton"}},
		{GLOVE_STUDDED_BLOODHOUND,{"models/weapons/w_models/arms/w_glove_bloodhound.mdl", "models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound.mdl"}},
		{GLOVE_T_SIDE,{"models/weapons/w_models/arms/w_glove_fingerless.mdl","models/weapons/v_models/arms/glove_fingerless/v_glove_fingerless.mdl"}},
		{GLOVE_CT_SIDE,{"models/weapons/w_models/arms/w_glove_hardknuckle.mdl","models/weapons/v_models/arms/glove_hardknuckle/v_glove_hardknuckle.mdl"}},
		{GLOVE_SPORTY,{"models/weapons/w_models/arms/w_glove_sporty.mdl","models/weapons/v_models/arms/glove_sporty/v_glove_sporty.mdl"}},
		{GLOVE_SLICK,{"models/weapons/w_models/arms/w_glove_slick.mdl","models/weapons/v_models/arms/glove_slick/v_glove_slick.mdl"}},
		{GLOVE_LEATHER_WRAP,{"models/weapons/w_models/arms/w_glove_handwrap_leathery.mdl","models/weapons/v_models/arms/glove_handwrap_leathery/v_glove_handwrap_leathery.mdl"}},
		{GLOVE_MOTORCYCLE,{"models/weapons/w_models/arms/w_glove_motorcycle.mdl","models/weapons/v_models/arms/glove_motorcycle/v_glove_motorcycle.mdl"}},
		{GLOVE_SPECIALIST,{"models/weapons/w_models/arms/w_glove_specialist.mdl","models/weapons/v_models/arms/glove_specialist/v_glove_specialist.mdl"}},
		{GLOVE_HYDRA,{"models/weapons/w_models/arms/w_glove_bloodhound_hydra.mdl","models/weapons/v_models/arms/glove_bloodhound_hydra/v_glove_bloodhound_hydra.mdl"}},
		{GLOVE_STUDDE_BROKEN_FANG,{"models/weapons/w_models/arms/w_glove_bloodhound_brokenfang.mdl","models/weapons/v_models/arms/glove_bloodhound/v_glove_bloodhound_brokenfang.mdl"}}
	};

	const auto entry = info.find(defindex);
	return entry == end(info) ? nullptr : &entry->second;
}
