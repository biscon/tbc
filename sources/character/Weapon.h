//
// Created by bison on 28-01-25.
//

#ifndef SANDBOX_WEAPON_H
#define SANDBOX_WEAPON_H

#include "data/WeaponData.h"

void InitWeaponTemplateData(WeaponTemplateData& weaponTemplateData, const std::string& filename);
int CreateWeapon(WeaponData& weaponData, const std::string& templateName);
#endif //SANDBOX_WEAPON_H
