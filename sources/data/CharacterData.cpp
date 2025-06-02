//
// Created by bison on 28-05-25.
//
#include "CharacterData.h"

void to_json(nlohmann::json& j, const CharacterStats& m) {
    j = nlohmann::json {
        {"health", m.health},
        {"maxHealth", m.maxHealth},
        {"attack", m.attack},
        {"defense", m.defense},
        {"speed", m.speed},
        {"hunger", m.hunger},
        {"thirst", m.thirst},
        {"movePoints", m.movePoints},
        {"level", m.level}
    };
}

void from_json(const nlohmann::json& j, CharacterStats& m) {
    j.at("health").get_to(m.health);
    j.at("maxHealth").get_to(m.maxHealth);
    j.at("attack").get_to(m.attack);
    j.at("defense").get_to(m.defense);
    j.at("speed").get_to(m.speed);
    j.at("hunger").get_to(m.hunger);
    j.at("thirst").get_to(m.thirst);
    j.at("movePoints").get_to(m.movePoints);
    j.at("level").get_to(m.level);
}

void to_json(nlohmann::json& j, const CharacterClass& c) {
    switch (c) {
        case CharacterClass::Warrior: j = "Warrior"; break;
        case CharacterClass::Mage:    j = "Mage";    break;
        case CharacterClass::Rogue:   j = "Rogue";   break;
    }
}

void from_json(const nlohmann::json& j, CharacterClass& c) {
    const std::string& s = j.get<std::string>();
    if      (s == "Warrior") c = CharacterClass::Warrior;
    else if (s == "Mage")    c = CharacterClass::Mage;
    else if (s == "Rogue")   c = CharacterClass::Rogue;
    else throw std::invalid_argument("Invalid CharacterClass: " + s);
}

void to_json(nlohmann::json& j, const CharacterFaction& f) {
    switch (f) {
        case CharacterFaction::Player: j = "Player"; break;
        case CharacterFaction::Npc:    j = "Npc";    break;
        case CharacterFaction::Enemy:  j = "Enemy";  break;
    }
}

void from_json(const nlohmann::json& j, CharacterFaction& f) {
    const std::string& s = j.get<std::string>();
    if      (s == "Player") f = CharacterFaction::Player;
    else if (s == "Npc")    f = CharacterFaction::Npc;
    else if (s == "Enemy")  f = CharacterFaction::Enemy;
    else throw std::invalid_argument("Invalid CharacterFaction: " + s);
}
