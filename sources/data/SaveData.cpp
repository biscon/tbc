//
// Created by bison on 28-05-25.
//
#include <fstream>
#include "SaveData.h"

void to_json(nlohmann::json& j, const MapState& m) {
    j = nlohmann::json{{"deadMobIds", m.deadMobIds}};
}

void from_json(const nlohmann::json& j, MapState& m) {
    j.at("deadMobIds").get_to(m.deadMobIds);
}

void to_json(nlohmann::json& j, const PartyCharacter& c) {
    j = nlohmann::json{
            {"name", c.name},
            {"ai", c.ai},
            {"characterClass", c.characterClass},
            {"faction", c.faction},
            {"stats", c.stats}
    };
}

void from_json(const nlohmann::json& j, PartyCharacter& c) {
    j.at("name").get_to(c.name);
    j.at("ai").get_to(c.ai);
    j.at("characterClass").get_to(c.characterClass);
    j.at("faction").get_to(c.faction);
    j.at("stats").get_to(c.stats);
}

bool SaveGameData(SaveData& data, const std::string& filename) {
    nlohmann::json j;
    j["currentMap"] = data.currentMap;
    //j["party"] = data.party;
    j["maps"] = data.maps;

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

    j.at("currentMap").get_to(data.currentMap);
    //j.at("party").get_to(data.party);
    j.at("maps").get_to(data.maps);
    return true;
}