//
// Created by bison on 09-01-25.
//

#include <cmath>
#include "Character.h"
#include "util/Random.h"
#include "game/Items.h"
#include "graphics/CharSprite.h"


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
    stats.LVL = 1;
    stats.HP = CalculateCharHealth(stats);
    data.stats.emplace_back(stats);
    data.orientation.emplace_back(Orientation::Right);
    data.equippedItemIdx.emplace_back();
    data.equippedItemIdx.back().fill(-1);
    data.statusEffects.emplace_back();
    data.sprite.emplace_back();
    data.selectedWeaponSlot.push_back(static_cast<int>(ItemEquipSlot::Weapon1));

    data.skillValues.emplace_back();
    data.skillValues.back().fill(10);

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
    data.skillValues.erase(data.skillValues.begin() + id);
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
    data.skillValues.clear();
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
    Vector2 start = GetCharSpritePos(spriteData, charData.sprite[attackerId]);
    Vector2 end = GetCharSpritePos(spriteData, charData.sprite[defenderId]);
    auto& charSprite = charData.sprite[attackerId];

    // Horizontal movement
    if (end.x > start.x) {
        charSprite.orientation = CharOrientation::Right;
        charData.orientation[attackerId] = Orientation::Right;
    } else {
        charSprite.orientation = CharOrientation::Left;
        charData.orientation[attackerId] = Orientation::Left;
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
    if(data.charData.selectedWeaponSlot[charIdx] == static_cast<int>(slot)) {
        if(itemIdx != -1) {
            int tplIdx = GetItemTypeTemplateId(data, itemIdx);
            SetCharWeaponType(data.spriteData, data.charData.sprite[charIdx], data.weaponData.templateData[tplIdx].animType);
        } else {
            SetCharHideWeapon(data.spriteData, data.charData.sprite[charIdx], true);
        }
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
    //return 10 + (stats.END * 3) + ((stats.LVL - 1) * (stats.END / 2));
    /*
     * 15 + 2 * EN + ST
     * Every time you gain a level this increases by 2 + EN / 2
     */
    int hp = 15;
    hp += 2 * stats.END;
    hp += stats.STR;
    //TraceLog(LOG_INFO, "HP before levels: %d", hp);
    int hpPerLevel = 2 + (stats.END/2);
    //TraceLog(LOG_INFO, "HP per level: %d", hpPerLevel);
    hp += (stats.LVL-1) * hpPerLevel;
    //TraceLog(LOG_INFO, "HP after levels: %d", hp);
    return hp;
}

int CalculateCharInitiative(CharacterStats &stats) {
    // Initiative = REF + Random(0, 5) + (LUK / 2)
    return stats.REF + GetRandomValue(0, 5) + (stats.LUK / 2);
}

int CalculateCharMaxAP(CharacterStats &stats) {
    // AP = 4 + (REF / 2)
    return 5 + (stats.REF / 2);
}

void SetSelectedWeaponSlot(GameData& data, int charId, ItemEquipSlot slot) {
    if(static_cast<int>(slot) == data.charData.selectedWeaponSlot[charId]) {
        return;
    }

    data.charData.selectedWeaponSlot[charId] = static_cast<int>(slot);
    int itemIdx = GetEquippedItem(data, data.ui.selectedCharacter, slot);

    if(itemIdx != -1) {
        int tplIdx = GetItemTypeTemplateId(data, itemIdx);
        SetCharWeaponType(data.spriteData, data.charData.sprite[charId], data.weaponData.templateData[tplIdx].animType);
    } else {
        SetCharHideWeapon(data.spriteData, data.charData.sprite[charId], true);
    }
}

void SwapWeapons(GameData &data, int charIdx) {
    auto selectedSlot = static_cast<ItemEquipSlot>(data.charData.selectedWeaponSlot[charIdx]);
    switch(selectedSlot) {
        case ItemEquipSlot::Weapon1:
            data.charData.selectedWeaponSlot[charIdx] = static_cast<int>(ItemEquipSlot::Weapon2);
            break;
        case ItemEquipSlot::Weapon2:
            data.charData.selectedWeaponSlot[charIdx] = static_cast<int>(ItemEquipSlot::Weapon1);
            break;
        default:
            throw std::runtime_error("");
    }
    selectedSlot = static_cast<ItemEquipSlot>(data.charData.selectedWeaponSlot[charIdx]);
    int itemIdx = GetEquippedItem(data, data.ui.selectedCharacter, selectedSlot);

    if(itemIdx != -1) {
        int tplIdx = GetItemTypeTemplateId(data, itemIdx);
        SetCharWeaponType(data.spriteData, data.charData.sprite[charIdx], data.weaponData.templateData[tplIdx].animType);
    } else {
        SetCharHideWeapon(data.spriteData, data.charData.sprite[charIdx], true);
    }
}

int GetSelectedWeaponItemId(GameData& data, int charId) {
    int selectedSlot = data.charData.selectedWeaponSlot[charId];
    int itemId = GetEquippedItem(data, charId, static_cast<ItemEquipSlot>(selectedSlot));
    return itemId;
}

 WeaponTemplate* GetSelectedWeaponTemplate(GameData& data, int charId) {
    int selectedSlot = data.charData.selectedWeaponSlot[charId];
    int itemId = GetEquippedItem(data, charId, static_cast<ItemEquipSlot>(selectedSlot));
    if(itemId != -1) {
        int weaponTplId = GetItemTypeTemplateId(data, itemId);
        WeaponTemplate& weaponTpl = data.weaponData.templateData[weaponTplId];
        return &weaponTpl;
    }
    return nullptr;
}

WeaponInstance* GetSelectedWeaponInstance(GameData& data, int charId) {
    int selectedSlot = data.charData.selectedWeaponSlot[charId];
    int itemId = GetEquippedItem(data, charId, static_cast<ItemEquipSlot>(selectedSlot));
    if(itemId != -1) {
        WeaponInstance* instance = &data.weaponData.instanceData[data.itemData.instanceData[itemId].typeInstanceId];
        return instance;
    }
    return nullptr;
}

WeaponRanged* GetSelectedRangedTemplate(GameData& data, int charId) {
    int selectedSlot = data.charData.selectedWeaponSlot[charId];
    int itemId = GetEquippedItem(data, charId, static_cast<ItemEquipSlot>(selectedSlot));
    if(itemId != -1) {
        int weaponTplId = GetItemTypeTemplateId(data, itemId);
        WeaponTemplate& weaponTpl = data.weaponData.templateData[weaponTplId];
        if(weaponTpl.rangeDataId != -1) {
            WeaponRanged& weaponRanged = data.weaponData.rangedData[weaponTpl.rangeDataId];
            return &weaponRanged;
        }
    }
    return nullptr;
}

int GetSkillValue(GameData& data, Skill s, int charId) {
    return data.charData.skillValues[charId][static_cast<int>(s)];
}

int GetSkillValue(GameData& data, const std::string& skillName, int charId) {
    return GetSkillValue(data, SkillIdToEnum(skillName), charId);
}

void SetSkillValue(GameData &data, Skill s, int charId, int value) {
    data.charData.skillValues[charId][static_cast<size_t>(s)] = value;
}
