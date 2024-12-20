#include <array>
#include "structs.hpp"
#include "..\features\misc\misc.h"
#include "..\features\misc\logs.h"
#include "..\features\serverside\local_animations.h"
#include "..\features\serverside\setup_bones.h"

datamap_t* entity_t::GetDataDescMap()
{
	typedef datamap_t*(__thiscall* Fn)(void*);
	return call_virtual<Fn>(this, 15)(this);
}

datamap_t* entity_t::GetPredDescMap()
{
	typedef datamap_t*(__thiscall* Fn)(void*);
	return call_virtual<Fn>(this, 17)(this);
}

bool entity_t::is_player()
{
	if (!this)
		return false;

	//auto client_class = GetClientClass();

	//if (!client_class)
	//	return false;

	//return client_class->m_ClassID == CCSPlayer;

	using Fn = bool(__thiscall*)(void*);
	return call_virtual<Fn>(this, 158)(this);
}

void entity_t::set_model_index(int index)
{
	if (!this)
		return;

	using Fn = void(__thiscall*)(void*, int);
	return call_virtual<Fn>(this, 75)(this, index);
}

void entity_t::validate_model_index()
{
	if (!this)
		return;

	using Fn = void(__thiscall*)(void*);
	call_virtual<Fn>(this, 54)(this);
}

void entity_t::set_abs_angles(const Vector& angle)
{
	if (!this)
		return;

	return memory->setAbsAngles(this, angle);
}

void entity_t::set_abs_origin(const Vector& origin)
{
	if (!this)
		return;

	return memory->setAbsOrigin(this, origin);
}

#pragma optimize( "", off)
#pragma runtime_checks("", off) //Disable runtime checks to prevent ESP error
void entity_t::getSequenceLinearMotion(CStudioHdr* studioHdr, int sequence, Vector& v)
{
	if (!this)
		return;

	memory->getSequenceLinearMotion(studioHdr, sequence, (float*)&m_flPoseParameter(), &v);
}
#pragma runtime_checks("", restore) //Restore runtime checks
#pragma optimize( "", on )

float entity_t::getSequenceMoveDist(CStudioHdr* studioHdr, int sequence)
{
	Vector v{};

	getSequenceLinearMotion(studioHdr, sequence, v);

	return v.Length();
}

weapon_info_t* weapon_t::get_weapon_info()
{
	if (!this || !m_weaponsystem())
		return nullptr;

	//return VirtualMethod::call<weapon_info_t*, 461>(this);
	return m_weaponsystem()->GetWeaponData(m_iItemDefinitionIndex());
}

float weapon_t::get_inaccuracy()
{
	if (!this)
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, 483)(this);
}

float weapon_t::get_spread()
{
	if (!this)
		return 0.0f;

	return call_virtual<float(__thiscall*)(void*)>(this, 453)(this);
}

float weapon_t::get_max_speed()
{
	if (!this)
		return 0.0f;

	return std::fmaxf(call_virtual<float(__thiscall*)(void*)>(this, 442)(this), 0.001f);
}

void weapon_t::update_accuracy_penality()
{
	if (!this)
		return;

	call_virtual<void(__thiscall*)(void*)>(this, 484)(this);
}

bool weapon_t::is_empty()
{
	if (!this)
		return true;

	return m_iClip() <= 0;
}

bool weapon_t::can_fire(bool check_revolver, int tickbase)
{
	if (!this)
		return false;

	if (!is_non_aim() && is_empty())
		return false;

	auto owner = reinterpret_cast<player_t*>(m_entitylist()->GetClientEntityFromHandle(m_hOwnerEntity()));

	if (owner == localPlayer.get() && (antiaim::get().freeze_check || owner->m_fFlags() & FL_ATCONTROLS))
		return false;

	if (!owner->valid(false) || owner->m_bIsDefusing() || owner->m_bWaitForNoAttack())
		return false;

	auto server_time = (tickbase != INT_MAX ? tickbase : util::server_time()); // TICKS_TO_TIME

	if (tickbase == -1 && owner == localPlayer.get() && configs.ragebot.double_tap && misc::get().double_tap_key && !misc::get().hide_shots_key && !configs.ragebot.double_tap_instant)
		server_time = (cheat.globals.backup_tickbase - cheat.globals.tickbase_shift);

	if (owner->m_flNextAttack() > server_time)
		return false;

	if (m_flNextPrimaryAttack() > server_time)
		return false;

	if (check_revolver && m_iItemDefinitionIndex() == WEAPON_REVOLVER && m_flPostponeFireReadyTime() >= server_time)
		return false;

	return true;
}

bool weapon_t::can_shift_tickbase(int tickbase_shift)
{
	if (!this || !localPlayer)
		return false;

	auto curtime = TICKS_TO_TIME(localPlayer->m_nTickBase() - tickbase_shift);

	if (curtime < localPlayer->m_flNextAttack() || curtime < m_flNextPrimaryAttack())
		return false;

	return true;
}

int weapon_t::get_weapon_group(bool rage)
{
	if (!this)
		return -1;

	if (rage)
	{
		if (m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 1;
		else if (m_iItemDefinitionIndex() == WEAPON_REVOLVER)
			return 2;
		else if (is_pistol())
			return 3;
		else if (is_smg())
			return 4;
		else if (is_rifle())
			return 5;
		else if (m_iItemDefinitionIndex() == WEAPON_SCAR20 || m_iItemDefinitionIndex() == WEAPON_G3SG1)
			return 6;
		else if (m_iItemDefinitionIndex() == WEAPON_SSG08)
			return 7;
		else if (m_iItemDefinitionIndex() == WEAPON_AWP)
			return 8;
		else if (is_shotgun())
			return 9;
		else if (!is_non_aim() && m_iItemDefinitionIndex() != WEAPON_TASER)
			return 0;
	}
	else
	{
		if (m_iItemDefinitionIndex() == WEAPON_REVOLVER || m_iItemDefinitionIndex() == WEAPON_DEAGLE)
			return 0;
		else if (is_pistol())
			return 1;
		else if (is_rifle())
			return 2;
		else if (is_smg())
			return 3;
		else if (is_sniper())
			return 4;
		else if (is_shotgun())
			return 5;
	}

	return -1;
}

bool weapon_t::is_rifle()
{
	if (!this)
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AK47 || weapon_id == WEAPON_M4A1 || weapon_id == WEAPON_M4A1_SILENCER || weapon_id == WEAPON_GALILAR ||
		weapon_id == WEAPON_FAMAS || weapon_id == WEAPON_AUG || weapon_id == WEAPON_SG553;
}

bool weapon_t::is_smg()
{
	if (!this)
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_MAC10 || weapon_id == WEAPON_MP7 || weapon_id == WEAPON_MP9 || weapon_id == WEAPON_P90 ||
		weapon_id == WEAPON_BIZON || weapon_id == WEAPON_UMP45 || weapon_id == WEAPON_MP5SD;
}

bool weapon_t::is_shotgun()
{
	if (!this)
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_XM1014 || weapon_id == WEAPON_NOVA || weapon_id == WEAPON_SAWEDOFF || weapon_id == WEAPON_MAG7 ||
		weapon_id == WEAPON_M249 || weapon_id == WEAPON_NEGEV;
}

bool weapon_t::is_pistol()
{
	if (!this)
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_DEAGLE || weapon_id == WEAPON_ELITE || weapon_id == WEAPON_FIVESEVEN || weapon_id == WEAPON_P250 ||
		weapon_id == WEAPON_GLOCK || weapon_id == WEAPON_HKP2000 || weapon_id == WEAPON_CZ75A || weapon_id == WEAPON_USP_SILENCER ||
		weapon_id == WEAPON_TEC9 || weapon_id == WEAPON_REVOLVER;
}

bool weapon_t::is_sniper()
{
	if (!this)
		return false;

	int weapon_id = m_iItemDefinitionIndex();

	return weapon_id == WEAPON_AWP || weapon_id == WEAPON_SCAR20 || weapon_id == WEAPON_G3SG1 || weapon_id == WEAPON_SSG08;
}

bool weapon_t::is_normal_grenade()
{
	if (!this)
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE || idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV ||
		idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE;
}

bool weapon_t::is_grenade()
{
	if (!this)
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_FLASHBANG || idx == WEAPON_HEGRENADE || idx == WEAPON_BREACHCHARGE || idx == WEAPON_BUMPMINE || idx == WEAPON_SMOKEGRENADE || idx == WEAPON_MOLOTOV ||
		idx == WEAPON_DECOY || idx == WEAPON_INCGRENADE || idx == WEAPON_TAGRENADE || idx == WEAPON_SNOWBALL;
}

bool weapon_t::is_knife()
{
	if (!this)
		return false;

	int idx = m_iItemDefinitionIndex();

	return idx == WEAPON_KNIFE || idx == WEAPON_KNIFE_BAYONET || idx == WEAPON_KNIFE_BUTTERFLY || idx == WEAPON_KNIFE_FALCHION
		|| idx == WEAPON_KNIFE_FLIP || idx == WEAPON_KNIFE_GUT || idx == WEAPON_KNIFE_KARAMBIT || idx == WEAPON_KNIFE_M9_BAYONET
		|| idx == WEAPON_KNIFE_PUSH || idx == WEAPON_KNIFE_SURVIVAL_BOWIE || idx == WEAPON_KNIFE_T || idx == WEAPON_KNIFE_TACTICAL
		|| idx == WEAPON_KNIFE_GOLDEN || idx == WEAPON_KNIFE_GHOST || idx == WEAPON_KNIFE_GYPSY_JACKKNIFE || idx == WEAPON_KNIFE_STILETTO
		|| idx == WEAPON_KNIFE_URSUS || idx == WEAPON_KNIFE_WIDOWMAKER || idx == WEAPON_KNIFE_CSS || idx == WEAPON_KNIFE_CANIS
		|| idx == WEAPON_KNIFE_CORD || idx == WEAPON_KNIFE_OUTDOOR || idx == WEAPON_KNIFE_SKELETON;
}

bool weapon_t::is_non_aim()
{
	if (!this)
		return true;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_C4 || idx == WEAPON_HEALTHSHOT)
		return true;

	if (is_knife())
		return true;

	if (is_grenade())
		return true;

	return false;
}

bool weapon_t::can_double_tap()
{
	if (!this)
		return false;

	if (is_non_aim())
		return false;

	auto idx = m_iItemDefinitionIndex();

	if (idx == WEAPON_TASER || idx == WEAPON_REVOLVER || idx == WEAPON_SSG08 || idx == WEAPON_AWP || idx == WEAPON_XM1014 || idx == WEAPON_NOVA || idx == WEAPON_SAWEDOFF || idx == WEAPON_MAG7)
		return false;

	return true;
}

int weapon_t::get_max_tickbase_shift()
{
	return m_gamerules()->m_bIsValveDS() ? 6 : 16;
}

int weapon_t::get_max_tickbase_shift_default()
{
	if (!can_double_tap())
		return get_max_tickbase_shift();

	int max_tickbase_shift = 0;

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_M249:
	case WEAPON_MAC10:
	case WEAPON_P90:
	case WEAPON_MP5SD:
	case WEAPON_NEGEV:
	case WEAPON_MP9:
		max_tickbase_shift = 5;
		break;
	case WEAPON_ELITE:
	case WEAPON_UMP45:
	case WEAPON_BIZON:
	case WEAPON_TEC9:
	case WEAPON_MP7:
		max_tickbase_shift = 6;
		break;
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_GALILAR:
	case WEAPON_M4A1:
	case WEAPON_M4A1_SILENCER:
	case WEAPON_CZ75A:
		max_tickbase_shift = 7;
		break;
	case WEAPON_FIVESEVEN:
	case WEAPON_GLOCK:
	case WEAPON_P250:
	case WEAPON_SG553:
		max_tickbase_shift = 8;
		break;
	case WEAPON_HKP2000:
	case WEAPON_USP_SILENCER:
		max_tickbase_shift = 9;
		break;
	case WEAPON_DEAGLE:
		max_tickbase_shift = 13;
		break;
	case WEAPON_G3SG1:
	case WEAPON_SCAR20:
		max_tickbase_shift = 15;
		break;
	default:
		max_tickbase_shift = 16;
		break;
	}

	if (m_gamerules()->m_bIsValveDS())
		max_tickbase_shift = max(max_tickbase_shift, 6);

	return max_tickbase_shift;
}

const char* weapon_t::get_icon()
{
	if (!this)
		return " ";

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE_BAYONET:
		return "1";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "7";
	case WEAPON_KNIFE_BUTTERFLY:
		return "8";
	case WEAPON_KNIFE_FALCHION:
		return "0";
	case WEAPON_KNIFE_FLIP:
		return "2";
	case WEAPON_KNIFE_GUT:
		return "3";
	case WEAPON_KNIFE_KARAMBIT:
		return "4";
	case WEAPON_KNIFE_M9_BAYONET:
		return "5";
	case WEAPON_KNIFE_TACTICAL:
		return "6";
	case WEAPON_KNIFE_PUSH:
		return "]";
	case WEAPON_DEAGLE:
		return "A";
	case WEAPON_ELITE:
		return "B";
	case WEAPON_FIVESEVEN:
		return "C";
	case WEAPON_GLOCK:
		return "D";
	case WEAPON_HKP2000:
		return "E";
	case WEAPON_P250:
		return "F";
	case WEAPON_USP_SILENCER:
		return "G";
	case WEAPON_TEC9:
		return "H";
	case WEAPON_REVOLVER:
		return "J";
	case WEAPON_MAC10:
		return "K";
	case WEAPON_UMP45:
		return "L";
	case WEAPON_BIZON:
		return "M";
	case WEAPON_MP7:
		return "N";
	case WEAPON_MP9:
		return "O";
	case WEAPON_P90:
		return "P";
	case WEAPON_GALILAR:
		return "Q";
	case WEAPON_FAMAS:
		return "R";
	case WEAPON_M4A1_SILENCER:
		return "T";
	case WEAPON_M4A1:
		return "S";
	case WEAPON_AUG:
		return "U";
	case WEAPON_SG553:
		return "V";
	case WEAPON_AK47:
		return "W";
	case WEAPON_G3SG1:
		return "X";
	case WEAPON_SCAR20:
		return "Y";
	case WEAPON_AWP:
		return "Z";
	case WEAPON_SSG08:
		return "a";
	case WEAPON_XM1014:
		return "b";
	case WEAPON_SAWEDOFF:
		return "c";
	case WEAPON_MAG7:
		return "d";
	case WEAPON_NOVA:
		return "e";
	case WEAPON_NEGEV:
		return "f";
	case WEAPON_M249:
		return "g";
	case WEAPON_TASER:
		return "h";
	case WEAPON_FLASHBANG:
		return "i";
	case WEAPON_HEGRENADE:
		return "j";
	case WEAPON_SMOKEGRENADE:
		return "k";
	case WEAPON_MOLOTOV:
		return "l";
	case WEAPON_DECOY:
		return "m";
	case WEAPON_INCGRENADE:
		return "n";
	case WEAPON_C4:
		return "o";
	case WEAPON_CZ75A:
		return "I";
	default:
		return " ";
	}
}

std::string weapon_t::get_name()
{
	if (!this)
		return " ";

	switch (m_iItemDefinitionIndex())
	{
	case WEAPON_KNIFE:
		return "KNIFE";
	case WEAPON_KNIFE_T:
		return "KNIFE";
	case WEAPON_KNIFE_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_SURVIVAL_BOWIE:
		return "KNIFE";
	case WEAPON_KNIFE_BUTTERFLY:
		return "KNIFE";
	case WEAPON_KNIFE_FALCHION:
		return "KNIFE";
	case WEAPON_KNIFE_FLIP:
		return "KNIFE";
	case WEAPON_KNIFE_GUT:
		return "KNIFE";
	case WEAPON_KNIFE_KARAMBIT:
		return "KNIFE";
	case WEAPON_KNIFE_M9_BAYONET:
		return "KNIFE";
	case WEAPON_KNIFE_TACTICAL:
		return "KNIFE";
	case WEAPON_KNIFE_PUSH:
		return "KNIFE";
	case WEAPON_DEAGLE:
		return "DEAGLE";
	case WEAPON_ELITE:
		return "DUAL BERETTAS";
	case WEAPON_FIVESEVEN:
		return "FIVE-SEVEN";
	case WEAPON_GLOCK:
		return "GLOCK 18";
	case WEAPON_HKP2000:
		return "P2000";
	case WEAPON_P250:
		return "P250";
	case WEAPON_USP_SILENCER:
		return "USP-S";
	case WEAPON_TEC9:
		return "TEC-9";
	case WEAPON_REVOLVER:
		return "REVOLVER";
	case WEAPON_MAC10:
		return "MAC-10";
	case WEAPON_UMP45:
		return "UMP-45";
	case WEAPON_BIZON:
		return "PP-BIZON";
	case WEAPON_MP7:
		return "MP7";
	case WEAPON_MP9:
		return "MP9";
	case WEAPON_P90:
		return "P90";
	case WEAPON_GALILAR:
		return "GALIL AR";
	case WEAPON_FAMAS:
		return "FAMAS";
	case WEAPON_M4A1_SILENCER:
		return "M4A1-S";
	case WEAPON_M4A1:
		return "M4A4";
	case WEAPON_AUG:
		return "AUG";
	case WEAPON_SG553:
		return "SG 553";
	case WEAPON_AK47:
		return "AK-47";
	case WEAPON_G3SG1:
		return "G3SG1";
	case WEAPON_SCAR20:
		return "SCAR-20";
	case WEAPON_AWP:
		return "AWP";
	case WEAPON_SSG08:
		return "SSG 08";
	case WEAPON_XM1014:
		return "XM1014";
	case WEAPON_SAWEDOFF:
		return "SAWED-OFF";
	case WEAPON_MAG7:
		return "MAG-7";
	case WEAPON_NOVA:
		return "NOVA";
	case WEAPON_NEGEV:
		return "NEGEV";
	case WEAPON_M249:
		return "M249";
	case WEAPON_TASER:
		return "ZEUS X27";
	case WEAPON_FLASHBANG:
		return "FLASHBANG";
	case WEAPON_HEGRENADE:
		return "HE GRENADE";
	case WEAPON_SMOKEGRENADE:
		return "SMOKE";
	case WEAPON_MOLOTOV:
		return "MOLOTOV";
	case WEAPON_DECOY:
		return "DECOY";
	case WEAPON_INCGRENADE:
		return "INCENDIARY";
	case WEAPON_C4:
		return "C4";
	case WEAPON_CZ75A:
		return "CZ75-AUTO";
	default:
		return " ";
	}
}

int entity_t::lookupSequence(const char* sequence)
{
	return memory->lookUpSequence(this, sequence);
}

CStudioHdr* entity_t::get_model_ptr()
{
	return *reinterpret_cast<CStudioHdr**>(reinterpret_cast<uintptr_t>(this) + 0x2950);
}

std::array<float, 24>& entity_t::m_flPoseParameter()
{
	static auto _m_flPoseParameter = netvars::get().get_offset(crypt_str("CBaseAnimating"), crypt_str("m_flPoseParameter"));
	return *reinterpret_cast<std::array<float, 24>*>(uintptr_t(this) + _m_flPoseParameter);
}

float entity_t::get_first_sequence_anim_tag(int sequence, int animtag)
{
	float returnValue = 0.0;
	memory->getFirstSequenceAnimTag(this, sequence, animtag, 0);
	__asm movss returnValue, xmm0
	return returnValue;
}

float entity_t::setPoseParameter(float value, int index)
{
	static auto address = memory->studioSetPoseParameter;

	CStudioHdr* studioHdr = get_model_ptr();
	if (!studioHdr)
		return value;

	if (index >= 0)
	{
		float newValue;

		__asm {

			pushad

			movss xmm2, [value]
			lea ecx, [newValue]
			push ecx
			mov edx, index
			mov ecx, studioHdr
			call address
			pop ecx

			popad
		}
		m_flPoseParameter()[index] = newValue;
	}
	return value;
}

Vector player_t::get_eye_position()
{
	if (!this)
		return ZERO;

	//Vector position;
	//VirtualMethod::call<void, 285>(this, std::ref(position));

	Vector position = m_vecOrigin() + m_vecViewOffset();

	if (this != localPlayer.get())
	{
		//modify_another_eye_position(get_animation_state(), position);

		return position;
	}

	//modify_another_eye_position(get_animation_state(), position);
	modify_eye_position(position);

	return position;
}

void player_t::modify_another_eye_position(AnimationState* animstate, Vector& eye_position)
{
	if (!this)
		return;

	if (!animstate)
		return;

	if (!animstate->m_pBaseEntity)
		return;

	if (!animstate->m_pBaseEntity->is_player())
		return;

	if (!animstate->m_bOnLanding && animstate->m_flDuckAmount <= 0.0f && animstate->m_pBaseEntity->m_hGroundEntity())
		return;

	if (!animstate->m_pBaseEntity->is_alive())
		return;

	auto head_bone = -1;

	if (memory->lookUpBone)
		head_bone = memory->lookUpBone(animstate->m_pBaseEntity, crypt_str("head_0"));
	else
		head_bone = 8;

	if (head_bone == -1)
		return;

	Vector bones = get_bone_position(head_bone);//animstate->m_pBaseEntity->m_CachedBoneData().Base();

	bones.z += 1.7f;

	if (bones.z >= eye_position.z)
		return;

	eye_position.z = math::lerp2(math::simple_spline_remap_val_clamped(fabs(eye_position.z - bones.z), 4.0f, 10.0f, 0.0f, 1.0f), eye_position.z, bones.z);
}

void player_t::modify_eye_position(Vector& eye_position)
{
	if (!this)
		return;

	if (!local_animations::get().local_data.prediction_animstate)
		return;

	local_animations::get().local_data.prediction_animstate->m_pBaseEntity = this;

	if (!local_animations::get().local_data.prediction_animstate->m_pBaseEntity->is_alive())
		return;

	if (!local_animations::get().local_data.prediction_animstate->m_bOnLanding && local_animations::get().local_data.prediction_animstate->m_flDuckAmount <= 0.0f && local_animations::get().local_data.prediction_animstate->m_pBaseEntity->m_hGroundEntity())
		return;

	auto head_bone = -1;

	if (memory->lookUpBone)
		head_bone = memory->lookUpBone(local_animations::get().local_data.prediction_animstate->m_pBaseEntity, crypt_str("head_0"));
	else
		head_bone = 8;

	if (head_bone == -1)
		return;

	auto head_position = Vector(cheat.globals.prediction_matrix[head_bone][0][3], cheat.globals.prediction_matrix[head_bone][1][3], cheat.globals.prediction_matrix[head_bone][2][3] + 1.7f);

	if (head_position.z >= eye_position.z)
		return;

	eye_position.z = math::lerp2(math::simple_spline_remap_val_clamped(fabs(eye_position.z - head_position.z), 4.0f, 10.0f, 0.0f, 1.0f), eye_position.z, head_position.z);
}

bool player_t::is_alive()
{
	if (!this)
		return false;

	//if (m_iTeamNum() != TeamID::TT && m_iTeamNum() != TeamID::CT)
	//	return false;

	//if (m_lifeState() != LIFE_ALIVE)
	//	return false;

	//if (m_iHealth() < 1)
	//	return false;

	//return true;
	return VirtualMethod::call<bool, 156>(this);
}

PlayerInfo* player_t::get_player_info()
{
	if (!this)
		return nullptr;

	if (PlayerInfo playerInfo; m_engine()->GetPlayerInfo(EntIndex(), &playerInfo))
		return &playerInfo;

	return nullptr;
}

int	player_t::get_move_type()
{
	if (!this)
		return 0;

	static auto movetype = (netvars::get().get_offset("CBaseEntity", "m_nRenderMode") + 1);
	if (movetype)
		return *reinterpret_cast<int*>(uintptr_t(this) + movetype);

	return *reinterpret_cast<int*>(uintptr_t(this) + 0x25C);
}

int player_t::get_hitbox_bone_id(int hitbox_id)
{
	if (!this)
		return -1;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return -1;

	auto hitbox_set = hdr->getHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return -1;

	auto hitbox = hitbox_set->getHitbox(hitbox_id);

	if (!hitbox)
		return -1;

	return hitbox->bone;
}

Vector player_t::get_bone_position(int bone)
{
	if (!this)
		return ZERO;

	Vector vectors[4];
	memory->getBonePos(this, bone, vectors);
	return { vectors[1].x, vectors[2].y, vectors[3].z };
}

Vector player_t::bone_position(int hitbox_id)
{
	if (!this)
		return ZERO;

	if (matrix3x4_t boneMatrices[MAXSTUDIOBONES]; SetupBones(boneMatrices, MAXSTUDIOBONES, BONE_USED_BY_ANYTHING, 0.0f))
		return boneMatrices[hitbox_id].GetOrigin();
	else
		return ZERO;
}

Vector player_t::hitbox_position(int hitbox_id)
{
	if (!this)
		return ZERO;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return ZERO;

	auto hitbox_set = hdr->getHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return ZERO;

	auto hitbox = hitbox_set->getHitbox(hitbox_id);

	if (!hitbox)
		return ZERO;

	Vector min, max;

	math::vector_transform(hitbox->bbmin, m_CachedBoneData().Base()[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, m_CachedBoneData().Base()[hitbox->bone], max);

	return (min + max) * 0.5f;
}

Vector player_t::hitbox_position_matrix(int hitbox_id, matrix3x4_t matrix[MAXSTUDIOBONES])
{
	if (!this)
		return ZERO;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return ZERO;

	auto hitbox_set = hdr->getHitboxSet(m_nHitboxSet());

	if (!hitbox_set)
		return ZERO;

	auto hitbox = hitbox_set->getHitbox(hitbox_id);

	if (!hitbox)
		return ZERO;

	Vector min, max;

	math::vector_transform(hitbox->bbmin, matrix[hitbox->bone], min);
	math::vector_transform(hitbox->bbmax, matrix[hitbox->bone], max);

	return (min + max) * 0.5f;
}

CommandContext* player_t::getCommandContext()
{
	return reinterpret_cast<CommandContext*>(uintptr_t(this) + 0x350C);
}

SafeUtlVector<matrix3x4_t>& player_t::m_CachedBoneData()
{
	//return *reinterpret_cast<SafeUtlVector<matrix3x4_t>*>(uintptr_t(this) + 0x2914);
	static auto m_CachedBoneData = *(DWORD*)(util::FindSignature(crypt_str("client.dll"), crypt_str("FF B7 ?? ?? ?? ?? 52")) + 2) + 4;
	return *(SafeUtlVector<matrix3x4_t>*)(uintptr_t(this) + m_CachedBoneData);
}

CBoneAccessor& player_t::m_BoneAccessor()
{
	return *reinterpret_cast<CBoneAccessor*>(uintptr_t(this) + (netvars::get().get_offset(crypt_str("CBaseAnimating"), crypt_str("m_nForceBone")) + 0x1C));
}

void player_t::invalidate_bone_cache()
{
	if (!this)
		return;

	m_flLastBoneSetupTime() = -FLT_MAX;
	m_iMostRecentModelBoneCounter() = UINT_MAX;
}

void player_t::set_abs_velocity(const Vector& velocity)
{
	if (!this)
		return;

	using Fn = void(__thiscall*)(void*, const Vector&);
	static auto fn = reinterpret_cast<Fn>(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 56 57 8B 7D 08 8B F1 F3")));

	return fn(this, velocity);
}

float player_t::sequence_duration(int sequence)
{
	float returnValue;
	memory->sequenceDuration(this, sequence);
	__asm movss returnValue, xmm0;
	return returnValue;
}

float player_t::get_sequence_cycle_rate(int sequence)
{
	const float t = sequence_duration(sequence);
	if (t > 0.0f)
		return 1.0f / t;
	else
		return 1.0f / 0.1f;
}

float player_t::get_sequence_cycle_rate(StudioHdr* studiohdr, int sequence)
{
	if (!this)
		return 0.0f;

	using Fn = float(__thiscall*)(void*, StudioHdr*, int);
	return call_virtual<Fn>(this, 222)(this, studiohdr, sequence);
}

float player_t::get_layer_sequence_cycle_rate(AnimationLayer* layer, int sequence)
{
	if (!this)
		return 0.0f;

	using Fn = float(__thiscall*)(void*, AnimationLayer*, int);
	return call_virtual<Fn>(this, 223)(this, layer, sequence);
}

void player_t::update_clientside_animation()
{
	if (!this)
		return;

	auto animstate = get_animation_state();

	if (!animstate)
		return;

	if (animstate->m_iLastClientSideAnimationUpdateFramecount == m_globals()->m_framecount)
		animstate->m_iLastClientSideAnimationUpdateFramecount -= 1;

	if (animstate->m_iLastClientSideAnimationUpdateFramecount == m_globals()->m_curtime)
		animstate->m_iLastClientSideAnimationUpdateFramecount += TICKS_TO_TIME(1);

	update_clientside_animation_original();
}

void player_t::update_clientside_animation_original()
{
	if (!this)
		return;

	using Fn = void(__thiscall*)(void*);
	call_virtual<Fn>(this, 224)(this);
}

weapon_t* player_t::getActiveWeapon()
{
	if (!this)
		return nullptr;

	using Fn = weapon_t*(__thiscall*)(void*);
	return call_virtual<Fn>(this, 268)(this);
}

uint32_t& player_t::m_iMostRecentModelBoneCounter()
{
	static auto most_recent_model_bone_counter = *reinterpret_cast<uintptr_t*>(memory->invalidateBoneCache + 0x1B);

	return *reinterpret_cast<uint32_t*>(uintptr_t(this) + most_recent_model_bone_counter);
}

float& player_t::m_flLastBoneSetupTime()
{
	static auto last_bone_setup_time = *reinterpret_cast<uintptr_t*>(memory->invalidateBoneCache + 0x11);

	return *reinterpret_cast<float*>(uintptr_t(this) + last_bone_setup_time);
}

bool player_t::physics_run_think(int index)
{
	return memory->physicsRunThink(this, index);
}

VarMapping_t* player_t::var_mapping()
{
	return reinterpret_cast<VarMapping_t*>(DWORD(this) + 0x24);
}

int& player_t::m_nButtons()
{
	static auto buttons = util::find_in_datamap(GetPredDescMap(), "m_nButtons");
	return *reinterpret_cast<int*>(uintptr_t(this) + buttons);
}

int& player_t::m_afButtonLast()
{
	static auto buttonLast = util::find_in_datamap(GetPredDescMap(), "m_afButtonLast");
	return *reinterpret_cast<int*>(uintptr_t(this) + buttonLast);
}

int& player_t::m_afButtonPressed()
{
	static auto buttonPressed = util::find_in_datamap(GetPredDescMap(), "m_afButtonPressed");
	return *reinterpret_cast<int*>(uintptr_t(this) + buttonPressed);
}

int& player_t::m_afButtonReleased()
{
	static auto buttonReleased = util::find_in_datamap(GetPredDescMap(), "m_afButtonReleased");
	return *reinterpret_cast<int*>(uintptr_t(this) + buttonReleased);
}

void player_t::updateButtonState(int userCmdButtonMask)
{
	m_afButtonLast() = m_nButtons();

	m_nButtons() = userCmdButtonMask;
	const int buttonsChanged = m_afButtonLast() ^ m_nButtons();

	m_afButtonPressed() = buttonsChanged & m_nButtons();
	m_afButtonReleased() = buttonsChanged & (~m_nButtons());
}

void player_t::checkHasThinkFunction(bool isThinking)
{
	return memory->checkHasThinkFunction(this, isThinking);
}

int& player_t::m_nNextThinkTick()
{
	return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0xFC);
}

void player_t::runPreThink()
{
	if (!this)
		return;

	if (physics_run_think(0))
		pre_think();
}

void player_t::runThink()
{
	if (!this)
		return;

	const auto nextThinkTick = m_nNextThinkTick();
	if (nextThinkTick != -1 &&
		nextThinkTick > 0 &&
		nextThinkTick <= m_nTickBase())
	{
		m_nNextThinkTick() = -1;

		checkHasThinkFunction(false);
		think();
	}
}

void player_t::runPostThink()
{
	if (!this)
		return;

	//We have to rebuild postThink because we dont want to mess with weapons
	m_modelcache()->BeginLock();

	if (is_alive())
	{
		update_collision_bounds();

		if (m_fFlags() & FL_ONGROUND)
			m_flFallVelocity() = 0.0f;

		if (m_nSequence() == -1)
			set_sequence(0);

		studio_frame_advance();
		memory->postThinkVPhysics(this);
	}

	memory->simulatePlayerSimulatedEntities(this);

	m_modelcache()->EndLock();
}

bool player_t::valid(bool check_team, bool check_dormant)
{
	if (!this || !localPlayer || !is_player() || !is_alive())
		return false;

	if (IsDormant() && check_dormant)
		return false;

	if (check_team && localPlayer->m_iTeamNum() == m_iTeamNum())
		return false;

	return true;
}

int player_t::animlayer_count()
{
	if (!this)
		return 0;

	//return *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x299C);
	int count = *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(this) + 0x299C);

	return count > 0 ? count : 13;
}

AnimationLayer* player_t::get_animlayers()
{
	if (!this)
		return nullptr;

	return *reinterpret_cast<AnimationLayer**>(reinterpret_cast<uintptr_t>(this) + 0x2990);
}

AnimationLayer* player_t::get_animlayer(int overlay)
{
	if (!this)
		return 0;

	return &get_animlayers()[overlay];
}

int player_t::sequence_activity(int sequence)
{
	if (!this)
		return 0;

	auto hdr = m_modelinfo()->GetStudioModel(GetModel());

	if (!hdr)
		return 0;

	static auto sequence_activity = reinterpret_cast<int(__fastcall*)(void*, StudioHdr*, int)>(util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 53 8B 5D 08 56 8B F1 83")));
	return sequence_activity(this, hdr, sequence);
}

CBaseHandle* player_t::m_hMyWeapons()
{
	return reinterpret_cast<CBaseHandle*>(uintptr_t(this) + netvars::get().get_offset("CBaseCombatCharacter", "m_hActiveWeapon") - 0x100);
}

float& player_t::m_flSpawnTime()
{
	static auto m_iAddonBits = netvars::get().get_offset("CCSPlayer", "m_iAddonBits");
	return *reinterpret_cast<float*>(uintptr_t(this) + m_iAddonBits - 4);
}

AnimationState* player_t::get_animation_state()
{
	static int m_bIsScoped = netvars::get().get_offset("CCSPlayer", "m_bIsScoped");
	return *reinterpret_cast<AnimationState**>(reinterpret_cast<void*>(uintptr_t(this) + (m_bIsScoped - 0x14)));
}

CStudioHdr* player_t::getStudioHdr()
{
	static auto studio_hdr = util::FindSignature(crypt_str("client.dll"), crypt_str("8B B7 ?? ?? ?? ?? 89 74 24 20"));
	return *reinterpret_cast<CStudioHdr**>(uintptr_t(this) + *reinterpret_cast<uintptr_t*>(studio_hdr + 0x2) + 0x4);
}

bool player_t::setup_bones_fixed(matrix3x4_t* matrix, int maxBones, int mask, float currentTime)
{
	if (!this)
		return false;

	int* render = reinterpret_cast<int*>(this + 0x274);
	int backupRender = *render;
	uint32_t backup_effects = m_fEffects();
	Vector backupAbsOrigin = GetAbsOrigin();

	*render = 0;
	invalidate_bone_cache();
	m_fEffects() |= EF_NOINTERP;

	if (this != localPlayer.get())
		set_abs_origin(m_vecOrigin());

	cheat.globals.setuping_bones = true;

	//if (mask == BONE_USED_BY_ANYTHING) {
	bool setuped = SetupBones(matrix, maxBones, mask, currentTime);
		/*
	} else {
		CSetupBones setup_bones;

		setup_bones.m_animating = this;
		setup_bones.m_vecOrigin = m_vecOrigin();
		setup_bones.m_animLayers = get_animlayers();
		setup_bones.m_pHdr = m_pStudioHdr();
		setup_bones.m_nAnimOverlayCount = animlayer_count();
		setup_bones.m_angAngles = Vector(0.0f, animstate ? animstate->m_flGoalFeetYaw : 0.0f, 0.0f);
		setup_bones.m_boneMatrix = matrix;

		memcpy(setup_bones.m_flPoseParameters, &m_flPoseParameter(), 24 * sizeof(float));

		auto weapon = getActiveWeapon();
		auto world_model = weapon ? weapon->m_hWeaponWorldModel().Get() : nullptr;

		if (world_model)
			memcpy(setup_bones.m_flWorldPoses, &world_model->m_flPoseParameter(), 24 * sizeof(float));

		setup_bones.m_boneMask = mask;

		Vector position[MAXSTUDIOBONES];
		Quaternion quat[MAXSTUDIOBONES];

		setup_bones.m_vecBones = position;
		setup_bones.m_quatBones = quat;
		setup_bones.m_flCurtime = m_flSimulationTime();

		setup_bones.setup();

		if (m_CachedBoneData().Base() != m_BoneAccessor().m_pBones)
			memcpy(m_BoneAccessor().m_pBones, setup_bones.m_boneMatrix, m_CachedBoneData().Count() * sizeof(matrix3x4_t));

		setup_bones.fix_bones_rotations();

		setuped = true;
	}
	*/
	cheat.globals.setuping_bones = false;

	if (this != localPlayer.get())
		set_abs_origin(backupAbsOrigin);

	*render = backupRender;
	m_fEffects() = backup_effects;

	return setuped;
}

bool player_t::setup_bones_fixed(matrix3x4_t* matrix, int mask)
{
	if (!this)
		return false;

	return setup_bones_fixed(matrix, MAXSTUDIOBONES, mask, m_flSimulationTime());
}

uint32_t& player_t::m_fEffects()
{
	static auto m_fEffects = util::find_in_datamap(GetPredDescMap(), crypt_str("m_fEffects"));
	return *reinterpret_cast<uint32_t*>(uintptr_t(this) + m_fEffects);
}

uint32_t& player_t::m_iEFlags()
{
	static auto m_iEFlags = util::find_in_datamap(GetPredDescMap(), crypt_str("m_iEFlags"));
	return *reinterpret_cast<uint32_t*>(uintptr_t(this) + m_iEFlags);
}

Vector& player_t::m_vecAbsVelocity()
{
	static auto m_vecAbsVelocity = util::find_in_datamap(GetPredDescMap(), crypt_str("m_vecAbsVelocity"));
	return *reinterpret_cast<Vector*>(uintptr_t(this) + m_vecAbsVelocity);
}

float player_t::get_max_desync_delta()
{
	if (!this)
		return 0.0f;
	
	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;
	
	return get_custom_desync_delta(animstate->m_flAimYawMax);
}

float player_t::get_min_desync_delta()
{
	if (!this)
		return 0.0f;
	
	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;

	return get_custom_desync_delta(animstate->m_flAimYawMin);
}

float player_t::get_custom_desync_delta(float value)
{
	if (!this)
		return 0.0f;

	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;

	auto speedfactor = math::clamp(animstate->m_flFeetSpeedForwardsOrSideWays, 0.0f, 1.0f);
	auto avg_speedfactor = (animstate->m_flStopToFullRunningFraction * -0.3f - 0.2f) * speedfactor + 1.0f;

	auto duck_amount = animstate->m_flDuckAmount;

	if (duck_amount)
	{
		auto max_velocity = max(animstate->m_flFeetSpeedCrouchForwardOrSideways, 1.0f);
		auto duck_speed = duck_amount * max_velocity;

		avg_speedfactor += duck_speed * (0.5f - avg_speedfactor);
	}

	return value * avg_speedfactor;
}

float player_t::get_max_desync_delta_for_auto()
{
	if (!this)
		return 0.0f;

	auto animstate = get_animation_state();

	if (!animstate)
		return 0.0f;

	auto speedfactor = math::clamp(animstate->m_flFeetSpeedForwardsOrSideWays, 0.0f, 1.0f);
	auto avg_speedfactor = (animstate->m_flStopToFullRunningFraction * -0.3f - 0.2f) * speedfactor + 1.0f;

	auto duck_amount = animstate->m_flDuckAmount;

	if (duck_amount)
	{
		auto max_velocity = max(animstate->m_flFeetSpeedCrouchForwardOrSideways, 1.0f);
		auto duck_speed = duck_amount * max_velocity;

		avg_speedfactor += duck_speed * (0.5f - avg_speedfactor);
	}

	auto m_flMinBodyYaw = avg_speedfactor * abs(animstate->m_flAimYawMin);
	auto m_flMaxBodyYaw = avg_speedfactor * abs(animstate->m_flAimYawMax);

	if (abs(animstate->m_flEyeYaw - animstate->m_flGoalFeetYaw) <= m_flMaxBodyYaw)
		if (m_flMinBodyYaw > abs(animstate->m_flEyeYaw - animstate->m_flGoalFeetYaw))
			return abs(m_flMinBodyYaw);
		else
			return abs(m_flMaxBodyYaw);

	return abs(m_flMaxBodyYaw);
}

void player_t::invalidate_physics_recursive(int change_flags)
{
	static auto m_uInvalidatePhysics = util::FindSignature(crypt_str("client.dll"), crypt_str("55 8B EC 83 E4 F8 83 EC 0C 53 8B 5D 08 8B C3 56 83 E0 04"));
	reinterpret_cast<void(__thiscall*)(void*, int)>(m_uInvalidatePhysics)(this, change_flags);
}

float player_t::get_max_player_speed()
{
	weapon_t* weapon = localPlayer->getActiveWeapon();
	if (weapon)
	{
		auto weapon_info = weapon->get_weapon_info();

		if (weapon_info)
			return localPlayer->m_bIsScoped() ? weapon_info->flMaxPlayerSpeedAlt : weapon_info->flMaxPlayerSpeed;
	}
	//return weapon->get_max_speed();

	return localPlayer->get_animation_state()->m_flSpeedNormalized;
}

float& viewmodel_t::m_flCycle()
{
	static auto m_flCycle = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flCycle"));
	return *reinterpret_cast<float*>(uintptr_t(this) + m_flCycle);
}

float& viewmodel_t::m_flAnimTime()
{
	static auto m_flAnimTime = util::find_in_datamap(GetPredDescMap(), crypt_str("m_flAnimTime"));
	return *reinterpret_cast<float*>(uintptr_t(this) + m_flAnimTime);
}

void viewmodel_t::SendViewModelMatchingSequence(int sequence)
{
	using Fn = void(__thiscall*)(void*, int);
	call_virtual<Fn>(this, 247)(this, sequence);
}
