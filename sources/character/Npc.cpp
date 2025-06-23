//
// Created by bison on 11-03-25.
//

#include <fstream>
#include "Npc.h"
#include "util/json.hpp"
#include "Character.h"

void to_json(nlohmann::json& j, const NpcInstance& npc) {
    j = nlohmann::json{
            { "template", npc.npcTemplate },
            { "position", npc.position },
            { "dialogueNode", npc.dialogueNodeId }
    };
}

void from_json(const nlohmann::json& j, NpcInstance& npc) {
    j.at("template").get_to(npc.npcTemplate);
    j.at("position").get_to(npc.position);
    j.at("dialogueNode").get_to(npc.dialogueNodeId);
}

void InitNpcTemplateData(NpcTemplateData &data, const std::string &filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    file >> j;
    for(auto& npcJson : j) {
        std::string charName = npcJson["characterName"].get<std::string>();
        data.charName.emplace_back(charName);

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

int CreateCharacterFromTemplate(GameData& data, const std::string &npcTemplate) {
    auto& npcTemplates = data.npcTemplateData.npcTemplates;
    auto& tplData = data.npcTemplateData;
    auto it = npcTemplates.find(npcTemplate);
    if(it != npcTemplates.end()) {
        int templateIdx = it->second;
        int charIdx = CreateCharacter(data.charData, tplData.characterClass[templateIdx],
                                      tplData.faction[templateIdx], tplData.charName[templateIdx],
                                      tplData.ai[templateIdx]);

        // level up character to desired level
        for(int i = 1; i < tplData.level[templateIdx]; ++i) {
            LevelUp(data.charData, charIdx, true);
        }
        InitCharacterSprite(data.spriteData, data.charData.sprite[charIdx], tplData.characterSprite[templateIdx], true);
        std::string weaponTemplate = tplData.weaponTemplate[templateIdx];
        if(!weaponTemplate.empty()) {
            GiveWeapon(data.spriteData, data.weaponData, data.charData, charIdx, weaponTemplate);
        }
        return charIdx;
    } else {
        TraceLog(LOG_ERROR, "CreateCharacterFromTemplate: Npc template not found: %s", npcTemplate.c_str());
        std::abort();
    }
}
