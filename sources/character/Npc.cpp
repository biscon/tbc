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
        data.weaponTemplate.emplace_back(name);

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
