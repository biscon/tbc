//
// Created by bison on 28-05-25.
//
#include <fstream>
#include "SaveData.h"
#include "ItemData.h"

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
            {"faction", c.faction},
            {"stats", c.stats},
            {"selectedWeaponSlot", c.selectedWeaponSlot}
    };
    nlohmann::json jSlots;
    for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
        jSlots[GetEquipSlotName(static_cast<ItemEquipSlot>(i))] = c.equippedItems[i];
    }
    j["equippedItems"] = jSlots;

    nlohmann::json jSkillValues;
    for (size_t i = 0; i < static_cast<size_t>(Skill::Count); ++i) {
        jSkillValues[EnumToSkillId(static_cast<Skill>(i))] = c.skillValues[i];
    }
    j["skillValues"] = jSkillValues;
}

void from_json(const nlohmann::json& j, PartyCharacter& c) {
    j.at("name").get_to(c.name);
    j.at("ai").get_to(c.ai);
    j.at("tilePosX").get_to(c.tilePosX);
    j.at("tilePosY").get_to(c.tilePosY);
    j.at("spriteTemplate").get_to(c.spriteTemplate);
    j.at("faction").get_to(c.faction);
    j.at("stats").get_to(c.stats);
    j.at("selectedWeaponSlot").get_to(c.selectedWeaponSlot);

    if(j.contains("equippedItems")) {
        const nlohmann::json &nodes = j.at("equippedItems");
        for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
            const std::string &slotName = it.key();
            c.equippedItems[GetEquipSlotIndexByName(slotName)] = it.value();
        }
    }

    if(j.contains("skillValues")) {
        const nlohmann::json &nodes = j.at("skillValues");
        for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
            const std::string &skillId = it.key();
            c.skillValues[static_cast<size_t>(SkillIdToEnum(skillId))] = it.value();
        }
    }
}

void to_json(nlohmann::json& j, const InventorySaveState& s) {
    j = nlohmann::json{
        {"capacity", s.capacity},
        {"itemTemplateIds", s.itemTemplateIds},
    };
}

void from_json(const nlohmann::json& j, InventorySaveState& s) {
    j.at("capacity").get_to(s.capacity);
    j.at("itemTemplateIds").get_to(s.itemTemplateIds);
}

bool SaveGameData(SaveData& data, const std::string& filename) {
    nlohmann::json j;
    j["currentMap"] = data.currentLevel;
    j["party"] = data.party;
    j["levels"] = data.levels;
    j["partyInventory"] = data.partyInventory;

    //const Inventory& partyInventory = game->

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
    j.at("partyInventory").get_to(data.partyInventory);

    // Handle QuestSaveState
    const nlohmann::json& nodes = j.at("quests");
    for (nlohmann::json::const_iterator it = nodes.begin(); it != nodes.end(); ++it) {
        const std::string& questId = it.key();
        data.quests[questId] = it.value().get<QuestSaveState>();
    }
    return true;
}