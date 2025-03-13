//
// Created by bison on 11-03-25.
//

#include <fstream>
#include "Npc.h"
#include "util/json.hpp"

using json = nlohmann::json;

void InitNpcTemplateData(NpcTemplateData &data, const std::string &filename) {
    std::ifstream file(filename);
    json j;
    file >> j;
    for(auto& npcJson : j) {
        std::string name = npcJson["name"].get<std::string>();
        data.name.emplace_back(name);

        std::string ai = npcJson["ai"].get<std::string>();
        data.ai.emplace_back(ai);

        std::string sprite = npcJson["sprite"].get<std::string>();
        data.characterSprite.emplace_back(sprite);

        std::string weapon = npcJson["weapon"].get<std::string>();
        data.weaponTemplate.emplace_back(weapon);

        int level = npcJson["level"].get<int>();
        data.level.emplace_back(level);

        std::string charClass = npcJson["class"].get<std::string>();
        data.characterClass.emplace_back(StringToClass(charClass));

        std::string faction = npcJson["faction"].get<std::string>();
        data.faction.emplace_back(StringToFaction(faction));

        data.npcTemplates[name] = (int) data.name.size()-1;
    }
    TraceLog(LOG_INFO, "Loaded %i NPC templates.", data.name.size());
}

int CreateCharacterFromTemplate(NpcTemplateData& tplData, CharacterData& charData, SpriteData& spriteData, WeaponData& weaponData, const std::string &name) {
    auto& npcTemplates = tplData.npcTemplates;
    auto it = npcTemplates.find(name);
    if(it != npcTemplates.end()) {
        int templateIdx = it->second;
        int charIdx = CreateCharacter(charData, tplData.characterClass[templateIdx],
                                      tplData.faction[templateIdx], tplData.name[templateIdx],
                                      tplData.ai[templateIdx]);

        // level up character to desired level
        for(int i = 1; i < tplData.level[templateIdx]; ++i) {
            LevelUp(charData, charIdx, true);
        }
        InitCharacterSprite(spriteData, charData.sprite[charIdx], tplData.characterSprite[templateIdx], true);
        GiveWeapon(spriteData, weaponData, charData, charIdx, tplData.weaponTemplate[templateIdx]);
        return charIdx;
    } else {
        TraceLog(LOG_ERROR, "CreateCharacterFromTemplate: Npc template not found: %s", name.c_str());
        std::abort();
    }
}
