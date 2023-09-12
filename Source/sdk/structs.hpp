#pragma once
#include "..\includes.hpp"
#include "interfaces\IClientEntity.hpp"
#include "misc\EHandle.hpp"
#include "misc\UtlVector.hpp"
#include "math\QAngle.hpp"
#include "misc\CommandContext.h"
#include "..\utils\netmanager.hpp"
#include "misc\CBoneAccessor.hpp"
#include "..\features\ragebot\fakelag.h"
#include "..\sdk\misc\Recv.hpp"

#define TIME_TO_TICKS(time) (static_cast<int>(0.5f + static_cast<float>(time) / m_globals()->m_intervalpertick))
#define TICKS_TO_TIME(ticks) (m_globals()->m_intervalpertick * (ticks))

class player_t;
struct datamap_t;

enum CSWeaponType
{
    WEAPONTYPE_KNIFE = 0,
    WEAPONTYPE_PISTOL,
    WEAPONTYPE_SUBMACHINEGUN,
    WEAPONTYPE_RIFLE,
    WEAPONTYPE_SHOTGUN,
    WEAPONTYPE_SNIPER_RIFLE,
    WEAPONTYPE_MACHINEGUN,
    WEAPONTYPE_C4,
    WEAPONTYPE_PLACEHOLDER,
    WEAPONTYPE_GRENADE,
    WEAPONTYPE_UNKNOWN
};

class VarMapEntry_t
{
public:
    unsigned short type;
    unsigned short m_bNeedsToInterpolate;

    void* data;
    void* watcher;
};

struct VarMapping_t
{
    VarMapping_t()
    {
        m_Entries = nullptr;
        m_nInterpolatedEntries = 0;
        m_lastInterpolationTime = 0.0f;
    }

    VarMapEntry_t* m_Entries;
    int m_nInterpolatedEntries;
    float m_lastInterpolationTime;
};

struct client_hit_verify_t
{
    Vector position;
    float time;
    float expires;
};

class AnimationLayer;
class AnimationState;
class entity_t;
class clientanimating_t;

class weapon_info_t
{
public:
    std::byte       pad[4]; //0x0000
    char*           szConsoleName; //0x0004
    std::byte       pad2[12]; //0x0008
    int             iMaxClip1; //0x0014
    std::byte       pad3[12]; //0x0018
    int             iMaxClip2; //0x0024
    std::byte       pad4[4]; //0x0028
    char*           szWorldModel; //0x002C
    char*           szViewModel; //0x0030
    char*           szDropedModel; //0x0034
    std::byte       pad5[4]; //0x0038
    char*           szUnknown; //0x003C
    std::byte       pad6[56]; //0x0040
    char*           szEmptySound; //0x0078
    std::byte       pad7[4]; //0x007C
    char*           szBulletType; //0x0080
    std::byte       pad8[4]; //0x0084
    char*           szHudName; //0x0088
    char*           szWeaponName; //0x008C
    std::byte       pad9[60]; //0x0090
    int             WeaponType; //0x00CC
    int             iWeaponPrice; //0x00D0
    int             iKillAward; //0x00D4
    char*           szAnimationPrefex; //0x00D8
    float           flCycleTime; //0x00DC
    float           flCycleTimeAlt; //0x00E0
    float           flTimeToIdle; //0x00E4
    float           flIdleInterval; //0x00E8
    bool            bFullAuto; //0x00EC
    std::byte       pad10[3]; //0x00ED
    int             iDamage; //0x00F0
    float           flHeadshotmultyplrier;
    float           flArmorRatio; //0x00F4
    int             iBullets; //0x00F8
    float           flPenetration; //0x00FC
    float           flFlinchVelocityModifierLarge; //0x0100
    float           flFlinchVelocityModifierSmall; //0x0104
    float           flRange; //0x0108
    float           flRangeModifier; //0x010C
    std::byte       pad11[28]; //0x0110
    int             iCrosshairMinDistance; //0x012C
    float           flMaxPlayerSpeed; //0x0130
    float           flMaxPlayerSpeedAlt; //0x0134
    std::byte       pad12[4]; //0x0138
    float           flSpread; //0x013C
    float           flSpreadAlt; //0x0140
    float           flInaccuracyCrouch; //0x0144
    float           flInaccuracyCrouchAlt; //0x0148
    float           flInaccuracyStand; //0x014C
    float           flInaccuracyStandAlt; //0x0150
    float           flInaccuracyJumpIntial; //0x0154
    float           flInaccaurcyJumpApex;
    float           flInaccuracyJump; //0x0158
    float           flInaccuracyJumpAlt; //0x015C
    float           flInaccuracyLand; //0x0160
    float           flInaccuracyLandAlt; //0x0164
    float           flInaccuracyLadder; //0x0168
    float           flInaccuracyLadderAlt; //0x016C
    float           flInaccuracyFire; //0x0170
    float           flInaccuracyFireAlt; //0x0174
    float           flInaccuracyMove; //0x0178
    float           flInaccuracyMoveAlt; //0x017C
    float           flInaccuracyReload; //0x0180
    int             iRecoilSeed; //0x0184
    float           flRecoilAngle; //0x0188
    float           flRecoilAngleAlt; //0x018C
    float           flRecoilVariance; //0x0190
    float           flRecoilAngleVarianceAlt; //0x0194
    float           flRecoilMagnitude; //0x0198
    float           flRecoilMagnitudeAlt; //0x019C
    float           flRecoilMagnatiudeVeriance; //0x01A0
    float           flRecoilMagnatiudeVerianceAlt; //0x01A4
    float           flRecoveryTimeCrouch; //0x01A8
    float           flRecoveryTimeStand; //0x01AC
    float           flRecoveryTimeCrouchFinal; //0x01B0
    float           flRecoveryTimeStandFinal; //0x01B4
    int             iRecoveryTransititionStartBullet; //0x01B8
    int             iRecoveryTransititionEndBullet; //0x01BC
    bool            bUnzoomAfterShot; //0x01C0
    std::byte       pad13[31]; //0x01C1
    char*           szWeaponClass; //0x01E0
    std::byte       pad14[56]; //0x01E4
    float           flInaccuracyPitchShift; //0x021C
    float           flInaccuracySoundThreshold; //0x0220
    float           flBotAudibleRange; //0x0224
    std::byte       pad15[12]; //0x0228
    bool            bHasBurstMode; //0x0234
};

struct NoticeText_t
{
    wchar_t     text[512];
    int         unknown0;   // 0x400
    float       unknown1;   // 0x404
    float       unknown2;   // 0x408
    int         unknown3;   // 0x40C
    float       time;       // 0x410
    int         unknown4;   // 0x414
    float       fade;       // 0x418
    int         unknown5;   // 0x41C
};

struct KillFeed_t
{
    char                        pad[0x7C];
    CUtlVector<NoticeText_t>    notices;
};

class entity_t : public IClientEntity
{
public:
    NETVAR(int, body, crypt_str("CBaseAnimating"), crypt_str("m_nBody"));
    NETVAR(int, m_nModelIndex, crypt_str("CBaseEntity"), crypt_str("m_nModelIndex"));
    NETVAR(int, m_iTeamNum, crypt_str("CBaseEntity"), crypt_str("m_iTeamNum"));
    NETVAR(Vector, m_vecOrigin, crypt_str("CBaseEntity"), crypt_str("m_vecOrigin"));
    NETVAR(CHandle<player_t>, m_hOwnerEntity, crypt_str("CBaseEntity"), crypt_str("m_hOwnerEntity"));
    NETVAR(int, m_CollisionGroup, crypt_str("CBaseEntity"), crypt_str("m_CollisionGroup"));
    NETVAR(int, m_nSequence, crypt_str("CBaseAnimating"), crypt_str("m_nSequence"));

    void setUseCustomBloomScale(byte value)
    {
        *reinterpret_cast<byte*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_bUseCustomBloomScale"))) = value;
    }

    void setCustomBloomScale(float value)
    {
        *reinterpret_cast<float*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_flCustomBloomScale"))) = value;
    }

    void setUseCustomAutoExposureMin(byte value)
    {
        *reinterpret_cast<byte*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_bUseCustomAutoExposureMin"))) = value;
    }

    void setCustomAutoExposureMin(float value)
    {
        *reinterpret_cast<float*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_flCustomAutoExposureMin"))) = value;
    }

    void setUseCustomAutoExposureMax(byte value)
    {
        *reinterpret_cast<byte*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_bUseCustomAutoExposureMax"))) = value;
    }

    void setCustomAutoExposureMax(float value)
    {
        *reinterpret_cast<float*>(uintptr_t(this) + (int)netvars::get().get_offset(crypt_str("CEnvTonemapController"), crypt_str("m_flCustomAutoExposureMax"))) = value;
    }

    const matrix3x4_t& m_CoordinateFrame()
    {
        static auto m_CoordinateFrame = netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_CollisionGroup")) - 0x30;
        return *reinterpret_cast<matrix3x4_t*>(uintptr_t(this) + m_CoordinateFrame);
    }

    int GetPropInt(std::string& table, std::string& var)
    {
        static auto offset = netvars::get().get_offset(table.c_str(), var.c_str());
        int val = *reinterpret_cast<int*>(uintptr_t(this) + (int)offset);
        return val;
    }

    float GetPropFloat(std::string& table, std::string& var)
    {
        static auto offset = netvars::get().get_offset(table.c_str(), var.c_str());
        float val = *reinterpret_cast<float*>(uintptr_t(this) + (int)offset);
        return val;
    }

    bool GetPropBool(std::string& table, std::string& var)
    {
        static auto offset = netvars::get().get_offset(table.c_str(), var.c_str());
        bool val = *reinterpret_cast<bool*>(uintptr_t(this) + (int)offset);
        return val;
    }

    std::string GetPropString(std::string& table, std::string& var)
    {
        static auto offset = netvars::get().get_offset(table.c_str(), var.c_str());
        char* val = reinterpret_cast<char*>(uintptr_t(this) + (int)offset);
        return std::string(val);
    }

    void SetPropInt(std::string& table, std::string& var, int val)
    {
        *reinterpret_cast<int*>(uintptr_t(this) + (int)netvars::get().get_offset(table.c_str(), var.c_str())) = val;
    }

    void SetPropFloat(std::string& table, std::string& var, float val)
    {
        *reinterpret_cast<float*>(uintptr_t(this) + (int)netvars::get().get_offset(table.c_str(), var.c_str())) = val;
    }

    void SetPropBool(std::string& table, std::string& var, bool val)
    {
        *reinterpret_cast<bool*>(uintptr_t(this) + (int)netvars::get().get_offset(table.c_str(), var.c_str())) = val;
    }

    datamap_t* GetDataDescMap();
    datamap_t* GetPredDescMap();
    int lookupSequence(const char* sequence);
    CStudioHdr* get_model_ptr();
    std::array <float, 24>& m_flPoseParameter();
    float setPoseParameter(float value, int index);
    float get_first_sequence_anim_tag(int sequence, int animtag);
    bool is_player();
    void set_model_index(int index);
    void validate_model_index();
    void set_abs_angles(const Vector& angle);
    void set_abs_origin(const Vector& origin);
    void getSequenceLinearMotion(CStudioHdr* studioHdr, int sequence, Vector& v);
    float getSequenceMoveDist(CStudioHdr* studioHdr, int sequence);
};

class attributableitem_t : public entity_t
{
public:
    NETVAR(int, m_iItemDefinitionIndex, crypt_str("CBaseAttributableItem"), crypt_str("m_iItemDefinitionIndex"));
    NETVAR(int, m_nFallbackStatTrak, crypt_str("CBaseAttributableItem"), crypt_str("m_nFallbackStatTrak"));
    NETVAR(int, m_nFallbackPaintKit, crypt_str("CBaseAttributableItem"), crypt_str("m_nFallbackPaintKit"));
    NETVAR(int, m_nFallbackSeed, crypt_str("CBaseAttributableItem"), crypt_str("m_nFallbackSeed"));
    NETVAR(float, m_flFallbackWear, crypt_str("CBaseAttributableItem"), crypt_str("m_flFallbackWear"));
    NETVAR(int, m_iAccountID, crypt_str("CBaseAttributableItem"), crypt_str("m_iAccountID"));
    NETVAR(int, m_iItemIDLow, "CBaseAttributableItem", "m_iItemIDLow");
    NETVAR(int, m_iItemIDHigh, crypt_str("CBaseAttributableItem"), crypt_str("m_iItemIDHigh"));
    PNETVAR(char, m_szCustomName, crypt_str("CBaseAttributableItem"), crypt_str("m_szCustomName"));
    NETVAR(int, m_OriginalOwnerXuidLow, crypt_str("CBaseAttributableItem"), crypt_str("m_OriginalOwnerXuidLow"));
    NETVAR(int, m_OriginalOwnerXuidHigh, crypt_str("CBaseAttributableItem"), crypt_str("m_OriginalOwnerXuidHigh"));
    NETVAR(int, m_iEntityQuality, crypt_str("CBaseAttributableItem"), crypt_str("m_iEntityQuality"));
    NETVAR(bool, m_bInitialized, crypt_str("CBaseAttributableItem"), "m_bInitialized");

    VIRTUAL(GiveToPlayer(player_t* player), 285, void(__thiscall*)(void*, player_t*), player);

    auto GetAttributableValue()
    {
        return std::uintptr_t(this) + netvars::get().get_offset("CBaseAttributableItem", "m_Item") + /* m_AttributeList = */ 0x244;
    }
};

class weapon_t : public attributableitem_t
{
public:
    NETVAR(float, m_flNextPrimaryAttack, crypt_str("CBaseCombatWeapon"), crypt_str("m_flNextPrimaryAttack"));
    NETVAR(float, m_flNextSecondaryAttack, crypt_str("CBaseCombatWeapon"), crypt_str("m_flNextSecondaryAttack"));
    NETVAR(int, m_hWeapon, crypt_str("CBaseViewModel"), crypt_str("m_hWeapon"));
    NETVAR(short, m_iItemDefinitionIndex, crypt_str("CBaseCombatWeapon"), crypt_str("m_iItemDefinitionIndex"));
    NETVAR(int, m_iClip, crypt_str("CBaseCombatWeapon"), crypt_str("m_iClip1"));
    NETVAR(int, m_iViewModelIndex, crypt_str("CBaseCombatWeapon"), crypt_str("m_iViewModelIndex"));
    NETVAR(int, m_iWorldModelIndex, crypt_str("CBaseCombatWeapon"), crypt_str("m_iWorldModelIndex"));
    NETVAR(float, m_fAccuracyPenalty, crypt_str("CWeaponCSBase"), crypt_str("m_fAccuracyPenalty"));
    NETVAR(int, m_zoomLevel, crypt_str("CWeaponCSBaseGun"), crypt_str("m_zoomLevel"));
    NETVAR(bool, m_bPinPulled, crypt_str("CBaseCSGrenade"), crypt_str("m_bPinPulled"));
    NETVAR(float, m_fThrowTime, crypt_str("CBaseCSGrenade"), crypt_str("m_fThrowTime"));
    NETVAR(float, m_flPostponeFireReadyTime, crypt_str("CWeaponCSBase"), crypt_str("m_flPostponeFireReadyTime"));
    NETVAR(float, m_fLastShotTime, crypt_str("CWeaponCSBase"), crypt_str("m_fLastShotTime"));
    NETVAR(float, m_flRecoilIndex, crypt_str("CWeaponCSBase"), crypt_str("m_flRecoilIndex"));
    NETVAR(CHandle<weapon_t>, m_hWeaponWorldModel, crypt_str("CBaseCombatWeapon"), crypt_str("m_hWeaponWorldModel"));

    bool IsZoomable(bool extra_check = true) {
        return m_iItemDefinitionIndex() == WEAPON_SSG08
            || m_iItemDefinitionIndex() == WEAPON_SCAR20
            || m_iItemDefinitionIndex() == WEAPON_AWP
            || m_iItemDefinitionIndex() == WEAPON_G3SG1
            || (extra_check && (m_iItemDefinitionIndex() == WEAPON_SG553 || m_iItemDefinitionIndex() == WEAPON_AUG));
    }

    weapon_info_t* get_weapon_info();
    bool is_empty();
    bool can_fire(bool check_revolver, int tickbase = INT_MAX);
    bool can_shift_tickbase(int tickbase_shift);
    int get_weapon_group(bool rage);
    bool is_rifle();
    bool is_smg();
    bool is_shotgun();
    bool is_pistol();
    bool is_sniper();
    bool is_normal_grenade();
    bool is_grenade();
    bool is_knife();
    bool is_non_aim();
    bool can_double_tap();
    int get_max_tickbase_shift();
    int get_max_tickbase_shift_default();
    float get_inaccuracy();
    float get_spread();
    float get_max_speed();
    void update_accuracy_penality();
    const char* get_icon();
    std::string get_name();

    auto requires_recoil_control() noexcept
    {
        const auto weaponData = get_weapon_info();
        if (weaponData)
            return weaponData->flRecoilMagnitude < 35.0f && weaponData->flRecoveryTimeStand > weaponData->flCycleTime;
        return false;
    }
};

class viewmodel_t;

class player_t : public entity_t
{
public:
    NETVAR(int, m_nExplodeEffectTickBegin, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_nExplodeEffectTickBegin"));
    NETVAR(bool, m_bClientSideAnimation, crypt_str("CBaseAnimating"), crypt_str("m_bClientSideAnimation"));
    NETVAR(float, m_flCycle, crypt_str("CBaseAnimating"), crypt_str("m_flCycle"));
    NETVAR(bool, m_bHasDefuser, crypt_str("CCSPlayer"), crypt_str("m_bHasDefuser"));
    NETVAR(bool, m_bGunGameImmunity, crypt_str("CCSPlayer"), crypt_str("m_bGunGameImmunity"));
    NETVAR(int, m_iShotsFired, crypt_str("CCSPlayer"), crypt_str("m_iShotsFired"));
    NETVAR(Vector, m_angEyeAngles, crypt_str("CCSPlayer"), crypt_str("m_angEyeAngles[0]"));
    NETVAR(Vector, m_angRotation, crypt_str("CBaseEntity"), crypt_str("m_angRotation"));
    NETVAR(int, m_iAddonBits, crypt_str("CCSPlayer"), crypt_str("m_iAddonBits"));
    NETVAR(int, m_iPrimaryAddon, crypt_str("CCSPlayer"), crypt_str("m_iPrimaryAddon"));
    NETVAR(int, m_iSecondaryAddon, crypt_str("CCSPlayer"), crypt_str("m_iSecondaryAddon"));
    NETVAR(int, m_ArmorValue, crypt_str("CCSPlayer"), crypt_str("m_ArmorValue"));
    NETVAR(int, m_iAccount, crypt_str("CCSPlayer"), crypt_str("m_iAccount"));
    NETVAR(bool, m_bHasHelmet, crypt_str("CCSPlayer"), crypt_str("m_bHasHelmet"));
    NETVAR(bool, m_bHasHeavyArmor, crypt_str("CCSPlayer"), crypt_str("m_bHasHeavyArmor"));
    NETVAR(bool, m_bIsScoped, crypt_str("CCSPlayer"), crypt_str("m_bIsScoped"));
    OFFSET(float, m_flCollisionChangeTime, netvars::get().get_offset(crypt_str("CCSPlayer"), crypt_str("m_bIsScoped")) - 0x50);
    NETVAR(float, m_flLowerBodyYawTarget, crypt_str("CCSPlayer"), crypt_str("m_flLowerBodyYawTarget"));
    NETVAR(float, m_flFlashDuration, crypt_str("CCSPlayer"), crypt_str("m_flFlashDuration"));
    NETVAR(CBaseHandle, m_hVehicle, crypt_str("CBasePlayer"), crypt_str("m_hVehicle"));
    NETVAR(int, m_iHealth, crypt_str("CBasePlayer"), crypt_str("m_iHealth"));
    NETVAR(int, m_lifeState, crypt_str("CBasePlayer"), crypt_str("m_lifeState"));
    NETVAR(int, m_fFlags, crypt_str("CBasePlayer"), crypt_str("m_fFlags"));
    NETVAR(int, m_nHitboxSet, crypt_str("CBasePlayer"), crypt_str("m_nHitboxSet"));
    NETVAR(int, m_nTickBase, crypt_str("CBasePlayer"), crypt_str("m_nTickBase"));
    NETVAR(Vector, m_vecViewOffset, crypt_str("CBasePlayer"), crypt_str("m_vecViewOffset[0]"));
    NETVAR(Vector, m_viewPunchAngle, crypt_str("CBasePlayer"), crypt_str("m_viewPunchAngle"));
    NETVAR(Vector, m_aimPunchAngle, crypt_str("CBasePlayer"), crypt_str("m_aimPunchAngle"));
    NETVAR(Vector, m_aimPunchAngleVel, crypt_str("CBasePlayer"), crypt_str("m_aimPunchAngleVel"));
    NETVAR(CHandle<viewmodel_t>, m_hViewModel, crypt_str("CBasePlayer"), crypt_str("m_hViewModel[0]"));
    NETVAR(Vector, m_vecBaseVelocity, crypt_str("CBasePlayer"), crypt_str("m_vecBaseVelocity"));
    NETVAR(Vector, m_vecVelocity, crypt_str("CBasePlayer"), crypt_str("m_vecVelocity[0]"));
    NETVAR(char[18], m_szLastPlaceName, crypt_str("CBasePlayer"), crypt_str("m_szLastPlaceName"));
    NETVAR(Vector, m_vecLadderNormal, crypt_str("CBasePlayer"), crypt_str("m_vecLadderNormal"));
    NETVAR(Vector, m_vecMins, crypt_str("CBaseEntity"), crypt_str("m_vecMins"));
    NETVAR(Vector, m_vecMaxs, crypt_str("CBaseEntity"), crypt_str("m_vecMaxs"));
    NETVAR(float, m_flVelocityModifier, crypt_str("CCSPlayer"), crypt_str("m_flVelocityModifier"));
    NETVAR(float, m_flSimulationTime, crypt_str("CBaseEntity"), crypt_str("m_flSimulationTime"));
    OFFSET(float, m_flOldSimulationTime, netvars::get().get_offset(crypt_str("CBaseEntity"), crypt_str("m_flSimulationTime")) + 4);
    NETVAR(float, m_flDuckSpeed, crypt_str("CCSPlayer"), crypt_str("m_flDuckSpeed"));
    NETVAR(float, m_flDuckAmount, crypt_str("CCSPlayer"), crypt_str("m_flDuckAmount"));
    NETVAR(float, m_flStamina, crypt_str("CCSPlayer"), crypt_str("m_flStamina"));
    NETVAR(bool, m_bDucked, crypt_str("CCSPlayer"), crypt_str("m_bDucked"));
    NETVAR(bool, m_bDucking, crypt_str("CCSPlayer"), crypt_str("m_bDucking"));
    NETVAR(float, m_flHealthShotBoostExpirationTime, crypt_str("CCSPlayer"), crypt_str("m_flHealthShotBoostExpirationTime"));
    NETVAR(int, m_bInBombZone, crypt_str("CCSPlayer"), crypt_str("m_bInBombZone"));
    NETVAR(float, m_flFallVelocity, crypt_str("CBasePlayer"), crypt_str("m_flFallVelocity"));
    NETVAR(float, m_flStepSize, crypt_str("CBaseEntity"), crypt_str("m_flStepSize"));
    NETVAR(float, m_flNextAttack, crypt_str("CBaseCombatCharacter"), crypt_str("m_flNextAttack"));
    NETVAR(bool, m_bWaitForNoAttack, crypt_str("CCSPlayer"), crypt_str("m_bWaitForNoAttack"));
    NETVAR(bool, m_bStrafing, crypt_str("CCSPlayer"), crypt_str("m_bStrafing"));
    NETVAR(int, m_iMoveState, crypt_str("CCSPlayer"), crypt_str("m_iMoveState"));
    PNETVAR(CBaseHandle, m_hMyWearables, crypt_str("CBaseCombatCharacter"), crypt_str("m_hMyWearables"));
    NETVAR(int, m_iObserverMode, crypt_str("CBasePlayer"), crypt_str("m_iObserverMode"));
    NETVAR(CHandle<player_t>, m_hObserverTarget, crypt_str("CBasePlayer"), crypt_str("m_hObserverTarget"));
    NETVAR(int, m_iNextThinkTick, crypt_str("CBasePlayer"), crypt_str("m_nNextThinkTick"));
    NETVAR(float, m_flThirdpersonRecoil, crypt_str("CCSPlayer"), crypt_str("m_flThirdpersonRecoil"));
    NETVAR(CHandle<weapon_t>, m_hActiveWeapon, crypt_str("CBaseCombatCharacter"), crypt_str("m_hActiveWeapon"));
    NETVAR(CHandle<attributableitem_t>, m_hWeaponWorldModel, crypt_str("CBaseCombatWeapon"), crypt_str("m_hWeaponWorldModel"));
    NETVAR(CHandle<entity_t>, m_hGroundEntity, crypt_str("CBasePlayer"), crypt_str("m_hGroundEntity"));
    NETVAR(bool, m_bSpotted, crypt_str("CBaseEntity"), crypt_str("m_bSpotted"));
    NETVAR(std::uint32_t, m_bSpottedByMask, crypt_str("CBaseEntity"), crypt_str("m_bSpottedByMask"));
    NETVAR(int, m_vphysicsCollisionState, crypt_str("CBasePlayer"), crypt_str("m_vphysicsCollisionState"));
    NETVAR(bool, m_bIsWalking, crypt_str("CCSPlayer"), crypt_str("m_bIsWalking"));
    NETVAR(bool, m_bIsDefusing, crypt_str("CCSPlayer"), crypt_str("m_bIsDefusing"));
    NETVAR(Vector, m_vecRagdollPosition, crypt_str("DT_Ragdoll"), crypt_str("m_ragPos"));
    //     NETVAR(CBaseHandle*, m_hWeapons, crypt_str("CBaseCombatCharacter"), crypt_str("m_hMyWeapons"));
    //NETVAR(int[64], m_hMyWeapons, "CBaseCombatCharacter", "m_hMyWeapons");
    VIRTUAL(think(), 139, void(__thiscall*)(void*));
    VIRTUAL(set_sequence(int sequence), 219, void(__thiscall*)(void*, int), sequence);
    VIRTUAL(studio_frame_advance(), 220, void(__thiscall*)(void*));
    VIRTUAL(pre_think(), 318, void(__thiscall*)(void*));
    //VIRTUAL(post_think(), 319, void(__thiscall*)(void*));
    VIRTUAL(update_collision_bounds(), 340, void(__thiscall*)(void*));

    CBaseHandle* m_hMyWeapons();

    float& m_flSpawnTime();

    uint32_t& m_iMostRecentModelBoneCounter();
    float& m_flLastBoneSetupTime();

    int& get_occlusion_flags()
    {
        return *reinterpret_cast<int*>(this + 0xA28);
    }

    int& get_occlusion_framecount()
    {
        return *reinterpret_cast<int*>(this + 0xA30);
    }

    int& last_skip_framecount()
    {
        return *reinterpret_cast<int*>(this + 0xA68);
    }

    bool& use_new_animation_state()
    {
        return *reinterpret_cast<bool*>(uintptr_t(this) + 0x26C5);
    }

    bool physics_run_think(int index);
    VarMapping_t* var_mapping();
    int& m_nButtons();
    int& m_afButtonLast();
    int& m_afButtonPressed();
    int& m_afButtonReleased();
    void updateButtonState(int userCmdButtonMask);
    void checkHasThinkFunction(bool isThinking);
    int& m_nNextThinkTick();
    void runPreThink();
    void runThink();
    void runPostThink();
    AnimationState* get_animation_state();
    CStudioHdr* getStudioHdr();
    bool setup_bones_fixed(matrix3x4_t* matrix, int maxBones, int mask, float currentTime);
    bool setup_bones_fixed(matrix3x4_t* matrix, int mask);
    Vector get_eye_position();
    void modify_another_eye_position(AnimationState* animstate, Vector& eye_position);
    void modify_eye_position(Vector& eye_position);
    uint32_t& m_fEffects();
    uint32_t& m_iEFlags();
    Vector& m_vecAbsVelocity();
    int get_hitbox_bone_id(int hitbox_id);
    Vector get_bone_position(int hitbox_id);
    Vector bone_position(int hitbox_id);
    Vector hitbox_position(int hitbox_id);
    Vector hitbox_position_matrix(int hitbox_id, matrix3x4_t matrix[MAXSTUDIOBONES]);
    AnimationLayer* get_animlayers();
    AnimationLayer* get_animlayer(int overlay);
    CommandContext* getCommandContext();
    CUtlVector<matrix3x4_t>& m_CachedBoneData();
    CBoneAccessor& m_BoneAccessor();
    void invalidate_bone_cache();
    void set_abs_velocity(const Vector& velocity);
    float sequence_duration(int sequence);
    float get_sequence_cycle_rate(int sequence);
    float get_sequence_cycle_rate(StudioHdr* studiohdr, int sequence);
    float get_layer_sequence_cycle_rate(AnimationLayer* layer, int sequence);
    void update_clientside_animation();
    void update_clientside_animation_original();
    weapon_t* getActiveWeapon();
    bool is_alive();
    PlayerInfo* get_player_info();
    bool valid(bool check_team, bool check_dormant = true);
    int get_move_type();
    int animlayer_count();
    int sequence_activity(int sequence);
    float get_max_desync_delta();
    float get_min_desync_delta();
    float get_custom_desync_delta(float value);
    float get_max_desync_delta_for_auto();
    void invalidate_physics_recursive(int change_flags);
    float get_max_player_speed();
};

class projectile_t : public entity_t
{
public:
    NETVAR(Vector, m_vInitialVelocity, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_vInitialVelocity"));
    NETVAR(int, m_flAnimTime, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_flAnimTime"));
    NETVAR(int, m_nExplodeEffectTickBegin, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_nExplodeEffectTickBegin"));
    NETVAR(int, m_nBody, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_nBody"));
    NETVAR(int, m_nForceBone, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_nForceBone"));
    NETVAR(Vector, m_vecVelocity, crypt_str("CBaseGrenade"), crypt_str("m_vecVelocity"));
    NETVAR(CHandle<player_t>, m_hThrower, crypt_str("CBaseGrenade"), crypt_str("m_hThrower"));
    NETVAR(Vector, m_vecOrigin, crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_vecOrigin"));
    OFFSET(float, m_flSpawnTime, netvars::get().get_offset(crypt_str("CBaseCSGrenadeProjectile"), crypt_str("m_vecExplodeEffectOrigin")) + 0xC);
};

class viewmodel_t : public entity_t
{
public:
    NETVAR(int, m_nModelIndex, crypt_str("CBaseViewModel"), crypt_str("m_nModelIndex"));
    NETVAR(int, m_nViewModelIndex, crypt_str("CBaseViewModel"), crypt_str("m_nViewModelIndex"));
    NETVAR(CHandle<weapon_t>, m_hWeapon, crypt_str("CBaseViewModel"), crypt_str("m_hWeapon"));
    NETVAR(CHandle<player_t>, m_hOwner, crypt_str("CBaseViewModel"), crypt_str("m_hOwner"));
    NETVAR(int, m_nAnimationParity, crypt_str("CBaseViewModel"), crypt_str("m_nAnimationParity"));

    float& m_flCycle();
    float& m_flAnimTime();
    void SendViewModelMatchingSequence(int sequence);
};

class CCSBomb : public entity_t
{
public:
    NETVAR(float, m_flDefuseCountDown, crypt_str("CPlantedC4"), crypt_str("m_flDefuseCountDown"));
    NETVAR(int, m_hBombDefuser, crypt_str("CPlantedC4"), crypt_str("m_hBombDefuser"));
    NETVAR(float, m_flC4Blow, crypt_str("CPlantedC4"), crypt_str("m_flC4Blow"));
    NETVAR(bool, m_bBombDefused, crypt_str("CPlantedC4"), crypt_str("m_bBombDefused"));
};

class ragdoll_t : public entity_t
{
public:
    NETVAR(Vector, m_vecForce, crypt_str("CCSRagdoll"), crypt_str("m_vecForce"));
    NETVAR(Vector, m_vecRagdollVelocity, crypt_str("CCSRagdoll"), crypt_str("m_vecRagdollVelocity"));
};

struct inferno_t : public entity_t
{
    OFFSET(float, get_spawn_time, 0x20);

    NETVAR(int[100], m_fireXDelta, crypt_str("CInferno"), crypt_str("m_fireXDelta"));
    NETVAR(int[100], m_fireYDelta, crypt_str("CInferno"), crypt_str("m_fireYDelta"));
    NETVAR(int[100], m_fireZDelta, crypt_str("CInferno"), crypt_str("m_fireZDelta"));
    NETVAR(bool[100], m_bFireIsBurning, crypt_str("CInferno"), crypt_str("m_bFireIsBurning"));
    NETVAR(int, m_fireCount, crypt_str("CInferno"), crypt_str("m_fireCount"));

    //float spawnTime() noexcept
    //{
    //    return *reinterpret_cast<float*>(reinterpret_cast<uintptr_t>(this) + 0x103C0);
    //}

    static float get_expiry_time()
    {
        return 7.03125f;
    }
};

struct smoke_t : public entity_t
{
    NETVAR(int, m_nSmokeEffectTickBegin, crypt_str("CSmokeGrenadeProjectile"), crypt_str("m_nSmokeEffectTickBegin"));
    NETVAR(bool, m_bDidSmokeEffect, crypt_str("CSmokeGrenadeProjectile"), crypt_str("m_bDidSmokeEffect"));

    static float get_expiry_time()
    {
        return 19.0f;
    }
};

struct animstatePoseParamCache
{
public:
    bool m_bInitialized; //0
    std::byte pad[3]; //1
    int m_iIndex; //4
    char* m_szName; //8

    bool init(player_t* player, char* poseParamName) noexcept;
    void setValue(player_t* player, float value) noexcept;
}; //Size: 0xC(12)

struct proceduralFoot
{
    Vector m_vecPosAnim;
    Vector m_vecPosAnimLast;
    Vector m_vecPosPlant;
    Vector m_vecPlantVel;
    float m_flLockAmount;
    float m_flLastPlantTime;
};//Size: 0x38(56)

class AnimationState
{
public:
    std::byte               pad1[4];
    bool					m_bFirstRunSinceInit; //0x4
    std::byte               pad2[87];
    weapon_t* m_pLastBoneSetupWeapon; //0x5C
    player_t* m_pBaseEntity; //0x60
    weapon_t* m_pActiveWeapon; //0x64
    weapon_t* m_pLastActiveWeapon; //0x68
    float                   m_flLastClientSideAnimationUpdateTime; //0x6C
    int                     m_iLastClientSideAnimationUpdateFramecount; //0x70
    float                   m_flUpdateTimeDelta; //0x74
    float                   m_flEyeYaw; //0x78
    float                   m_flEyePitch; //0x7C
    float                   m_flGoalFeetYaw; //0x80
    float                   m_flCurrentFeetYaw; //0x84
    float                   m_flCurrentTorsoYaw; //0x88
    float                   m_flIdealVelocityLean; //0x8C //changes when moving/jumping/hitting ground
    float                   m_flLeanAmount; //0x90
    float                   m_flTimeToAlignLowerBody; //148 = 0x94
    float                   m_flFeetCycle; //0x98 0 to 1
    float                   m_flFeetYawRate; //0x9C 0 to 1
    float                   m_flFeetYawRateSmoothed;
    float                   m_flDuckAmount; //0xA4
    float                   m_flLandingDuckAdditiveSomething; //0xA8
    float                   m_flRecrouchWeight; //0xAC
    Vector                  m_vecOrigin; //0xB0, 0xB4, 0xB8
    Vector                  m_vecLastOrigin; //0xBC, 0xC0, 0xC4
    Vector					m_vecVelocity; //T 200(X),204(Y),208(Z)
    Vector					m_vecVelocityNormalized; //T 212(X),216(Y),220(Z)
    Vector					m_vecVelocityNormalizedNonZero; //T 224(X),228(Y),232(Z)
    float					m_flVelocityLengthXY; // 0xEC
    float					m_flVelocityLengthZ; // 0xF0
    float                   m_flSpeedNormalized; //0xF4 //from 0 to 1
    float                   m_flFeetSpeedForwardsOrSideWays; //0xF8 //from 0 to 2. something  is 1 when walking, 2.something when running, 0.653 when crouch walking
    float                   m_flFeetSpeedCrouchForwardOrSideways; //0xFC //from 0 to 3. something
    float                   m_flTimeSinceStartedMoving; //0x100
    float                   m_flTimeSinceStoppedMoving; //0x104
    bool                    m_bOnGround; //0x108
    bool                    m_bOnLanding; //0x109
    std::byte               pad6[2]; //0x11A
    float                   m_flJumpToFall; //268
    float                   m_flDurationInAir; //0x110
    float                   m_flLastOriginZ; //0x114
    float                   m_flHeadHeightOrOffsetFromHittingGroundAnimation; //0x118 from 0 to 1, is 1 when standing
    float                   m_flStopToFullRunningFraction; //0x11C from 0 to 1, doesnt change when walking or crouching, only running
    bool			        m_bLandedOnGroundThisFrame; //0x120
    bool			        m_bLeftTheGroundThisFrame; //0x121
    std::byte               pad7[2]; //NaN
    float                   m_flMovingFraction; //0x124 affected while jumping and running, or when just jumping, 0 to 1
    bool                    m_bOnLadder; //0x128
    std::byte               pad8[3];
    float                   m_flLadderWeight; //300 = 0x12C
    float                   m_flLadderSpeed; //304 = 0x130
    bool                    m_bWalkToRunTransitionState; //308 = 0x134
    bool                    m_bDefuseStarted; //T 309
    bool                    m_bPlantAnimStarted;//T 310
    bool                    m_bTwitchAnimStarted;//T 311
    bool                    m_bAdjustStarted;//T 312
    std::byte               pad9[3]; //T 313
    CUtlVector<uint16_t>	m_szActivityModifiers;//T 316 (size 20)
    float					m_flNextTwitchTime;//T 336
    float					m_flTimeOfLastKnownInjury;//T 340
    float					m_flLastVelocityTestTime; //344 = 0x158
    Vector					m_vecVelocityLast; //T 348(X), 352(Y), 356(Z)
    Vector					m_vecTargetAcceleration; //T 360(X), 364(Y), 368(Z)
    Vector					m_vecAcceleration; //T 372(X), 376(Y), 380(Z)
    float					m_flAccelerationWeight; //T 384
    float					m_flAimMatrixTransition;//T 388
    float					m_flAimMatrixTransitionDelay; //T 392
    bool					m_bFlashed;// T 396
    std::byte               pad10[3];
    float					strafeChangeWeight; //400 = 0x190
    float					strafeChangeTargetWeight; // T 404
    float					strafeChangeCycle; //408 = 0x198
    int						strafeSequence; //412 = 0x19C
    bool					strafeChanging; //416 = 0x1A0
    std::byte               pad11[3];
    float					durationStrafing; //420 = 0x1A4
    float					footLerp; // T 424
    bool					feetCrossed; // T 428
    bool					playerIsAccelerating; //429 = 0x1AD
    std::byte               pad12[2];
    animstatePoseParamCache poseParamMappings[20];//T 432
    float					durationMoveWeightIsTooHigh; //672 = 0x2A0
    float					m_flStaticApproachSpeed; //676 =  0x2A4
    int						previousMoveState; //680 = 0x2A8
    float					stutterStep; //684 = 0x2AC
    float					actionWeightBiasRemainder; //688 = 0x2B0 //Literally useless??
    proceduralFoot	        footLeft; //692 = 0x2B4
    proceduralFoot	        footRight;//T 748
    float					cameraSmoothHeight; //804 = 0x324
    bool					smoothHeightValid; //808 = 0x328
    std::byte               pad13[3];
    float					m_flLastTimeVelocityOverTen; //T 812
    std::byte               pad14[4]; //T 816
    float					m_flAimYawMin; //T 820
    float					m_flAimYawMax; //T 824
    float					m_flAimPitchMin; //T 828
    float					m_flAimPitchMax; //T 832
    int						m_iAnimstateModelVersion; //836

    float                   lowerBodyRealignTimer;
    bool                    deployRateLimiting;
    bool                    jumping;
    int                     buttons;

    void setupVelocity() noexcept;
    void setupMovement() noexcept;
    void setupAliveLoop() noexcept;

    void doAnimationEvent(int animationEvent) noexcept;

    void updateAnimLayer(size_t layerIndex, int sequence, float playbackRate, float weight, float cycle) noexcept;

    void incrementLayerCycleWeightRateGeneric(size_t layer) noexcept;
    void incrementLayerCycle(size_t layer, bool allowLoop) noexcept;
    void incrementLayerWeight(size_t layer) noexcept;
    bool isLayerSequenceCompleted(size_t layer) noexcept;

    void setLayerWeightRate(size_t layer, float previous) noexcept;
    void setLayerWeight(size_t layer, float weight) noexcept;
    void setLayerCycle(size_t layer, float newCycle) noexcept;
    void setLayerRate(size_t layer, float newRate) noexcept;
    void setLayerSequence(size_t layer, int32_t activity) noexcept;

    int32_t selectSequenceFromActivityModifier(int activity) noexcept;

    float getLayerIdealWeightFromSeqCycle(size_t layer) noexcept;
    float getLayerCycle(size_t layer) noexcept;
    float getLayerWeight(size_t layer) noexcept;
    int getLayerActivity(size_t layer) noexcept;
    int getLayerSequence(size_t layer) noexcept;

    int getTicksFromCycle(float playbackRate, float cycle, float previousCycle) noexcept;
    float calculatePlaybackRate(Vector velocity) noexcept;
};

class CHudChat
{
public:
    template <typename... Args>
    void printf(const char* fmt, Args... args) noexcept
    {
        (*reinterpret_cast<void(__cdecl***)(void*, int, const char*, ...)>(this))[26](this, 0, fmt, args...);
    }

    template <typename... Args>
    void printf(int filter, const char* fmt, Args... args) noexcept
    {
        (*reinterpret_cast<void(__cdecl***)(void*, int, const char*, ...)>(this))[26](this, filter, fmt, args...);
    }
};

class AnimationLayer
{
public:
    bool            m_bClientBlend;		        //0x0000
    float           m_flBlendIn;	            //0x0004
    StudioHdr*      m_pStudioHdr;	            //0x0008
    int             m_iDispatchSrc;             //0x000C
    int             m_iDispatchDst;             //0x0010
    unsigned int    m_iOrder;                   //0x0014
    unsigned int    m_iSequence;                //0x0018
    float           m_flPrevCycle;              //0x001C
    float           m_flWeight;                 //0x0020
    float           m_flWeightDeltaRate;        //0x0024
    float           m_flPlaybackRate;           //0x0028
    float           m_flCycle;                  //0x002C
    void*           m_pOwner;                   //0x0030
    int             m_iInvalidatePhysicsBits;   //0x0034
};

class INetworkStringTable
{
public:

    virtual					~INetworkStringTable(void) {};

    // Table Info
    virtual const char*     GetTableName(void) const = 0;
    virtual int			    GetTableId(void) const = 0;
    virtual int				GetNumStrings(void) const = 0;
    virtual int				GetMaxStrings(void) const = 0;
    virtual int				GetEntryBits(void) const = 0;

    // Networking
    virtual void			SetTick(int tick) = 0;
    virtual bool			ChangedSinceTick(int tick) const = 0;

    // Accessors (length -1 means don't change user data if string already exits)
    virtual int				AddString(bool bIsServer, const char* value, int length = -1, const void* userdata = 0) = 0;

    virtual const char*     GetString(int stringNumber) = 0;
    virtual void			SetStringUserData(int stringNumber, int length, const void* userdata) = 0;
    virtual const void*     GetStringUserData(int stringNumber, int* length) = 0;
    virtual int				FindStringIndex(char const* string) = 0; // returns INVALID_STRING_INDEX if not found

    // Callbacks
    virtual void			SetStringChangedCallback(void* object, void* changeFunc) = 0;
};

class CNetworkStringTableContainer
{
    template <typename T, typename ... args_t>
    constexpr T CallVFunc(void* thisptr, std::size_t nIndex, args_t... argList)
    {
        using VirtualFnz = T(__thiscall*)(void*, decltype(argList)...);
        return (*static_cast<VirtualFnz**>(thisptr))[nIndex](thisptr, argList...);
    }
public:
    INetworkStringTable* FindTable(const char* tableName)
    {
        typedef INetworkStringTable* (__thiscall* oFindTable)(PVOID, const char*);
        return CallVFunc< oFindTable >(this, 3)(this, tableName);
    }
};

inline CNetworkStringTableContainer* ClientStringTableContainer = new CNetworkStringTableContainer();
