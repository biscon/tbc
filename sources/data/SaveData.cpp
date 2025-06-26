//
// Created by bison on 28-05-25.
//
#include <fstream>
#include "SaveData.h"

void to_json(nlohmann::json& j, const DoorSaveState& s) {
    j = nlohmann::json{
            {"id", s.id},
            {"open", s.open},
            {"locked", s.locked}
    };
}

void from_json(const nlohmann::json& j, DoorSaveState& s) {
    j.at("id").get_to(s.id);
    j.at("open").get_to(s.open);
    j.at("locked").get_to(s.locked);
}

void to_json(nlohmann::json& j, const LevelSaveState& m) {
    j = nlohmann::json{
        {"defeatedGroups", m.defeatedGroups},
        {"doors", m.doors},
        {"flags", m.flags}
    };
}

void from_json(const nlohmann::json& j, LevelSaveState& m) {
    j.at("defeatedGroups").get_to(m.defeatedGroups);
    j.at("doors").get_to(m.doors);
    j.at("flags").get_to(m.flags);
}

void to_json(nlohmann::json& j, const PartyCharacter& c) {
    j = nlohmann::json{
            {"name", c.name},
            {"ai", c.ai},
            {"tilePosX", c.tilePosX},
            {"tilePosY", c.tilePosY},
            {"spriteTemplate", c.spriteTemplate},
            {"characterClass", c.characterClass},
            {"faction", c.faction},
            {"stats", c.stats}
    };
    nlohmann::json jSlots;
    for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
        jSlots[GetEquipSlotName(static_cast<ItemEquipSlot>(i))] = c.equippedItems[i];
    }
    j["equippedItems"] = jSlots;
}

void from_json(const nlohmann::json& j, PartyCharacter& c) {
    j.at("name").get_to(c.name);
    j.at("ai").get_to(c.ai);
    j.at("tilePosX").get_to(c.tilePosX);
    j.at("tilePosY").get_to(c.tilePosY);
    j.at("spriteTemplate").get_to(c.spriteTemplate);
    j.at("characterClass").get_to(c.characterClass);
    j.at("faction").get_to(c.faction);
    j.at("stats").get_to(c.stats);

    if(j.contains("equippedItems")) {
        const nlohmann::json &nodes = j.at("equippedItems");
        for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
            const std::string &slotName = it.key();
            c.equippedItems[GetEquipSlotIndexByName(slotName)] = it.value();
        }
    }
}

bool SaveGameData(SaveData& data, const std::string& filename) {
    nlohmann::json j;
    j["currentMap"] = data.currentLevel;
    j["party"] = data.party;
    j["levels"] = data.levels;

    nlohmann::json questsJson;
    for (const auto& pair : data.quests) {
        questsJson[pair.first] = pair.second;  // `to_json` will be called automatically
    }
    j["quests"] = questsJson;

    std::ofstream file(filename);
    if (!file) return false;
    file << j.dump(2); // pretty print
    return true;
}

bool LoadGameData(SaveData& data, const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return false;

    nlohmann::json j;
    file >> j;

    j.at("currentMap").get_to(data.currentLevel);
    j.at("party").get_to(data.party);
    j.at("levels").get_to(data.levels);

    // Handle QuestSaveState
    const nlohmann::json& nodes = j.at("quests");
    for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        const std::string& questId = it.key();
        data.quests[questId] = it.value().get<QuestSaveState>();
    }
    return true;
}