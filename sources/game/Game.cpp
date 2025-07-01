//
// Created by bison on 18-02-25.
//

#include "Game.h"
#include "graphics/CharacterSprite.h"
#include "character/Character.h"
#include "GameMode.h"
#include "Items.h"

void CreateGame(GameData &game, const std::string& levelFileName) {
    game.state = GameState::START_NEW_GAME;
    game.levelFileName = levelFileName;
}

void DestroyGame(GameData &game) {

}

void StartNewGame(GameData &data) {
    int id = CreateCharacter(data.charData, CharacterFaction::Player, "Player1", "Fighter");
    AssignSkill(data.charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(data.charData.skills[id], SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(data.spriteData, data.charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(data, id, "item_weapon_knife", ItemEquipSlot::Weapon1);
    data.charData.stats[id].LVL = 5;
    data.charData.stats[id].HP = CalculateCharHealth(data.charData.stats[id]);
    data.charData.stats[id].AP = CalculateCharMaxAP(data.charData.stats[id]);
    data.party.emplace_back(id);

    id = CreateCharacter(data.charData, CharacterFaction::Player, "Player2", "Fighter");
    AssignSkill(data.charData.skills[id], SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(data.charData.skills[id], SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(data.spriteData, data.charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(data, id, "item_weapon_club", ItemEquipSlot::Weapon1);
    data.charData.stats[id].LVL = 5;
    data.charData.stats[id].HP = CalculateCharHealth(data.charData.stats[id]);
    data.charData.stats[id].AP = CalculateCharMaxAP(data.charData.stats[id]);
    data.party.emplace_back(id);

    data.state = GameState::LOAD_LEVEL;
    PushGameMode(GameModes::Level);
}

void LoadGame(GameData &data) {
    SaveData saveData;
    if(!LoadGameData(saveData, "savegame.json")) {
        TraceLog(LOG_DEBUG, "No existing savegame found");
        return;
    }
    data.levelFileName = saveData.currentLevel;
    data.state = GameState::LOAD_LEVEL_FROM_SAVE;
    data.levelState = saveData.levels;
    data.questState = saveData.quests;

    data.itemData.partyInventoryId = InventoryFromSaveState(data, saveData.partyInventory);

    ClearAllCharacters(data.charData);
    data.spriteData.player.animationIdx.clear();
    data.spriteData.player.animData.clear();
    data.spriteData.player.renderData.clear();
    data.party.clear();
    for(auto& ch : saveData.party) {
        int id = CreateCharacter(data.charData, ch.faction, ch.name, ch.ai);
        //AssignSkill(data.charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
        InitCharacterSprite(data.spriteData, data.charData.sprite[id], ch.spriteTemplate, true);

        // loop through equipment slots and instantiate items
        for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
            int itemId = -1;
            if(!ch.equippedItems[i].empty()) {
                itemId = CreateItem(data, ch.equippedItems[i], 1);
                SetEquippedItem(data, id, static_cast<ItemEquipSlot>(i), itemId);
            } else {
                data.charData.equippedItemIdx[id][i] = itemId;
            }
        }

        data.charData.stats[id] = ch.stats;
        data.charData.selectedWeaponSlot[id] = ch.selectedWeaponSlot;
        Vector2i savedPos = { ch.tilePosX, ch.tilePosY};
        SetCharacterGridPosI(data.spriteData, data.charData.sprite[id], savedPos);
        data.party.emplace_back(id);
    }

    PushGameMode(GameModes::Level);
}

void SaveGame(GameData &data) {
    SaveData saveData;
    saveData.currentLevel = data.levelFileName;
    saveData.levels = data.levelState;
    saveData.quests = data.questState;
    saveData.partyInventory = InventoryToSaveState(data, data.itemData.partyInventoryId);


    for(auto& id : data.party) {
        PartyCharacter pc;
        pc.name = data.charData.name[id];
        pc.faction = data.charData.faction[id];
        pc.ai = data.charData.ai[id];
        pc.stats = data.charData.stats[id];
        pc.selectedWeaponSlot = data.charData.selectedWeaponSlot[id];
        // save sprite template
        pc.spriteTemplate = data.charData.sprite[id].spriteTemplate;

        // save item equipment slots
        for (size_t i = 0; i < static_cast<size_t>(ItemEquipSlot::COUNT); ++i) {
            int itemId =  data.charData.equippedItemIdx[id][i];
            pc.equippedItems[i] = itemId == -1 ? "" : GetItemTemplateIdString(data, itemId);
        }

        // save position
        Vector2i pos = GetCharacterGridPosI(data.spriteData, data.charData.sprite[id]);
        pc.tilePosX = pos.x;
        pc.tilePosY = pos.y;

        saveData.party.push_back(pc);
    }

    SaveGameData(saveData, "savegame.json");
}

