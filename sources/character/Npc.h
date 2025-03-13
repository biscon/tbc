//
// Created by bison on 11-03-25.
//

#ifndef SANDBOX_NPC_H
#define SANDBOX_NPC_H

#include <string>
#include <vector>
#include "Character.h"
#include "data/NpcTemplateData.h"

void InitNpcTemplateData(NpcTemplateData& data, const std::string& filename);
int CreateCharacterFromTemplate(NpcTemplateData& tplData, CharacterData& charData, SpriteData& spriteData, WeaponData& weaponData, const std::string &name);



#endif //SANDBOX_NPC_H
