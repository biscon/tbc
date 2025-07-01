//
// Created by bison on 28-05-25.
//
#include "CharacterData.h"

void to_json(nlohmann::json& j, const CharacterStats& m) {
    j = nlohmann::json {
        {"STR", m.STR},
        {"REF", m.REF},
        {"END", m.END},
        {"INT", m.INT},
        {"PER", m.PER},
        {"CHA", m.CHA},
        {"LUK", m.LUK},
        {"LVL", m.LVL},
        {"HP", m.HP}
    };
}

void from_json(const nlohmann::json& j, CharacterStats& m) {
    j.at("STR").get_to(m.STR);
    j.at("REF").get_to(m.REF);
    j.at("END").get_to(m.END);
    j.at("INT").get_to(m.INT);
    j.at("PER").get_to(m.PER);
    j.at("CHA").get_to(m.CHA);
    j.at("LUK").get_to(m.LUK);
    j.at("LVL").get_to(m.LVL);
    j.at("HP").get_to(m.HP);
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
