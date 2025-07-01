//
// Created by bison on 09-01-25.
//

#include <cmath>
#include "Character.h"
#include "util/Random.h"
#include "game/Items.h"


bool IsAlive(CharacterData &data, int characterIdx) {
    return data.stats[characterIdx].HP > 0;
}

int CreateCharacter(CharacterData &data, CharacterFaction faction, const std::string& name, const std::string& ai) {
    data.name.push_back(name);
    data.ai.push_back(ai);
    data.faction.emplace_back(faction);

    CharacterStats stats{};
    stats.STR = 5;
    stats.REF = 5;
    stats.END = 5;
    stats.INT = 5;
    stats.PER = 5;
    stats.CHA = 5;
    stats.LUK = 5;
    stats.HP = CalculateCharHealth(stats);
    stats.LVL = 1;
    data.stats.emplace_back(stats);
    data.orientation.emplace_back(Orientation::Right);
    data.equippedItemIdx.emplace_back();
    data.equippedItemIdx.back().fill(-1);
    data.statusEffects.emplace_back();
    data.skills.emplace_back();
    data.sprite.emplace_back();
    data.selectedWeaponSlot.push_back(static_cast<int>(ItemEquipSlot::Weapon1));
    return (int) data.name.size()-1;
}

void DeleteCharacter(CharacterData& data, int id) {
    data.stats.erase(data.stats.begin() + id);
    data.sprite.erase(data.sprite.begin() + id);
    data.orientation.erase(data.orientation.begin() + id);
    data.equippedItemIdx.erase(data.equippedItemIdx.begin() + id);
    data.name.erase(data.name.begin() + id);
    data.ai.erase(data.ai.begin() + id);
    data.faction.erase(data.faction.begin() + id);
    data.statusEffects.erase(data.statusEffects.begin() + id);
    data.skills.erase(data.skills.begin() + id);
    data.selectedWeaponSlot.erase(data.selectedWeaponSlot.begin() + id);
}

void ClearAllCharacters(CharacterData& data) {
    data.stats.clear();
    data.sprite.clear();
    data.orientation.clear();
    data.equippedItemIdx.clear();
    data.name.clear();
    data.ai.clear();
    data.faction.clear();
    data.statusEffects.clear();
    data.skills.clear();
    data.selectedWeaponSlot.clear();
}

void GiveWeapon(GameData& data, int characterIdx, const std::string& itemTemplate, ItemEquipSlot slot) {
    int weaponId = CreateItem(data, itemTemplate, 1);
    SetEquippedItem(data, characterIdx, slot, weaponId);
}

Vector2 GetOrientationVector(Orientation orientation) {
    switch (orientation) {
        case Orientation::Up:
            return {0, -1};
        case Orientation::Down:
            return {0, 1};
        case Orientation::Left:
            return {-1, 0};
        case Orientation::Right:
            return {1, 0};
    }
    return {0, 0};
}

void FaceCharacter(SpriteData& spriteData, CharacterData &charData, int attackerId, int defenderId) {
    // Determine the direction of movement and set the appropriate animation
    Vector2 start = GetCharacterSpritePos(spriteData, charData.sprite[attackerId]);
    Vector2 end = GetCharacterSpritePos(spriteData, charData.sprite[defenderId]);
    if (fabs(end.x - start.x) > fabs(end.y - start.y)) {
        // Horizontal movement
        if (end.x > start.x) {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkRight, true);
            charData.orientation[attackerId] = Orientation::Right;
        } else {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkLeft, true);
            charData.orientation[attackerId] = Orientation::Left;
        }
    } else {
        // Vertical movement
        if (end.y > start.y) {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkDown, true);
            charData.orientation[attackerId] = Orientation::Down;
        } else {
            StartPausedCharacterSpriteAnim(spriteData, charData.sprite[attackerId], SpriteAnimationType::WalkUp, true);
            charData.orientation[attackerId] = Orientation::Up;
        }
    }
}

CharacterFaction StringToFaction(const std::string &factionName) {
    if(factionName == "Player") return CharacterFaction::Player;
    if(factionName == "Npc") return CharacterFaction::Npc;
    return CharacterFaction::Enemy;
}

int GetEquippedItem(const GameData& data, int charIdx, ItemEquipSlot slot) {
    return data.charData.equippedItemIdx[charIdx][static_cast<size_t>(slot)];
}

void SetEquippedItem(GameData& data, int charIdx, ItemEquipSlot slot, int itemIdx) {
    data.charData.equippedItemIdx[charIdx][static_cast<size_t>(slot)] = itemIdx;
    if(slot == ItemEquipSlot::Weapon1) {
        int tplIdx = GetItemTypeTemplateId(data, itemIdx);

        SetCharacterSpriteWeaponAnimation(data.spriteData, data.charData.sprite[charIdx], data.weaponData.templateData[tplIdx].animationTemplate);
    }
}

SpriteAnimationType CharacterOrientationToAnimType(GameData& data, int charId) {
    switch(data.charData.orientation[charId]) {
        case Orientation::Up:   return SpriteAnimationType::WalkUp;
        case Orientation::Down: return SpriteAnimationType::WalkDown;
        case Orientation::Left: return SpriteAnimationType::WalkLeft;
        case Orientation::Right: return SpriteAnimationType::WalkRight;
    }
    std::abort();
}

int CalculateCharHealth(CharacterStats &stats) {
    // HP = baseHP + (END × hpPerPoint) + (level - 1) × hpPerLevel
    return 10 + (stats.END * 3) + ((stats.LVL - 1) * (stats.END / 2));
}

int CalculateCharInitiative(CharacterStats &stats) {
    // Initiative = REF + Random(0, 5) + (LUK / 2)
    return stats.REF + GetRandomValue(0, 5) + (stats.LUK / 2);
}

int CalculateCharMaxAP(CharacterStats &stats) {
    // AP = 4 + (REF / 2)
    return 4 + (stats.REF / 2);
}
