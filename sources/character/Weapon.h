//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_WEAPON_H
#define SANDBOX_WEAPON_H

#include "data/WeaponData.h"
#include "data/SaveData.h"

void InitWeaponData(WeaponData& weaponData, const std::string& filename);
int CreateWeaponInstance(WeaponData& weaponData, int weaponTplId);
#endif //SANDBOX_WEAPON_H
