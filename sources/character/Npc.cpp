//
// Created by bison on 11-03-25.
//

#include <fstream>
#include "Npc.h"
#include "util/json.hpp"
#include "Character.h"
#include "game/Items.h"

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

        // equipped items
        data.equippedItems.emplace_back();
        if(npcJson.contains("equippedItems")) {
            const nlohmann::json &nodes = npcJson.at("equippedItems");
            for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
                const std::string &slotName = it.key();
                data.equippedItems.back()[GetEquipSlotIndexByName(slotName)] = it.value();
            }
        }

        data.skillValues.emplace_back();
        data.skillValues.back().fill(5);
        if(npcJson.contains("skillValues")) {
            const nlohmann::json &nodes = npcJson.at("skillValues");
            for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
                const std::string &skillId = it.key();
                data.skillValues.back()[static_cast<size_t>(SkillIdToEnum(skillId))] = it.value();
            }
        }

        int level = npcJson["level"].get<int>();
        data.level.emplace_back(level);

        std::string faction = npcJson["faction"].get<std::string>();
        data.faction.emplace_back(StringToFaction(faction));

        CharacterStats stats = npcJson["stats"].get<CharacterStats>();
        data.stats.emplace_back(stats);

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
        int charIdx = CreateCharacter(data.charData,
                                      tplData.faction[templateIdx], tplData.charName[templateIdx],
                                      tplData.ai[templateIdx]);

        CharacterStats& stats = data.charData.stats[charIdx];
        stats = data.npcTemplateData.stats[templateIdx];
        stats.HP = CalculateCharHealth(stats);
        stats.AP = CalculateCharMaxAP(stats);

        // Set skill values from template
        data.charData.skillValues[charIdx] = data.npcTemplateData.skillValues[templateIdx];

        InitCharacterSprite(data.spriteData, data.charData.sprite[charIdx], tplData.characterSprite[templateIdx], true);

        // loop through equipment slots and instantiate items
        for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
            int itemId = -1;
            if(!tplData.equippedItems[templateIdx][i].empty()) {
                itemId = CreateItem(data, tplData.equippedItems[templateIdx][i], 1);
                SetEquippedItem(data, charIdx, static_cast<ItemEquipSlot>(i), itemId);
            }
        }
        return charIdx;
    } else {
        TraceLog(LOG_ERROR, "CreateCharacterFromTemplate: Npc template not found: %s", npcTemplate.c_str());
        std::abort();
    }
}
