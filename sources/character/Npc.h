//
// Created by bison on 11-03-25.
//

#ifndef SANDBOX_NPC_H
#define SANDBOX_NPC_H

#include <string>
#include <vector>
#include "Character.h"

struct NpcTemplateData {
    std::vector<std::string> name;
    std::vector<CharacterClass> characterClass;
    std::vector<CharacterFaction> faction;
    std::vector<std::string> ai;
    std::vector<std::string> characterSprite;
    std::vector<int> level;
    std::vector<std::string> weaponTemplate;
    // name to index lookup
    std::unordered_map<std::string, int> npcTemplates;
};

struct NpcData {
    NpcTemplateData templateData;
};

void InitNpcTemplateData(NpcTemplateData& data, const std::string& filename);

#endif //SANDBOX_NPC_H
