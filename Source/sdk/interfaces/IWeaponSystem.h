#pragma once
class weapon_info_t;

class IWeaponSystem
{
public:
    virtual ~IWeaponSystem() = 0;
    virtual void unknown() = 0;
    virtual weapon_info_t* GetWeaponData(int weaponId) = 0;
};