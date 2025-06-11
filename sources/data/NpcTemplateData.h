//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_NPCTEMPLATEDATA_H
#define SANDBOX_NPCTEMPLATEDATA_H

#include <vector>
#include <string>
#include <unordered_map>
#include "CharacterData.h"

struct NpcTemplateData {
    std::vector<std::string> name;
    std::vector<std::string> charName;
    std::vector<CharacterClass> characterClass;
    std::vector<CharacterFaction> faction;
    std::vector<std::string> ai;
    std::vector<std::string> characterSprite;
    std::vector<int> level;
    std::vector<std::string> weaponTemplate;
    // name to index lookup
    std::unordered_map<std::string, int> npcTemplates;
};

#endif //SANDBOX_NPCTEMPLATEDATA_H
