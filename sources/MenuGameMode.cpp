//
// Created by bison on 09-02-25.
//

#include <cassert>
#include "MenuGameMode.h"
#include "raylib.h"
#include "raygui.h"
#include "ui/UI.h"
#include "data/SaveData.h"
#include "ai/PathFinding.h"

static GameData* game;

void MenuInit() {
    TraceLog(LOG_INFO, "MenuInit");

}

void MenuUpdate(float dt) {
    
}

static void StartNewGame() {
    int id = CreateCharacter(game->charData, CharacterClass::Warrior, CharacterFaction::Player, "Player1", "Fighter");
    AssignSkill(game->charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(game->charData.skills[id], SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(game->spriteData, game->charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(game->spriteData, game->weaponData, game->charData, id, "Sword");
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    game->party.emplace_back(id);

    id = CreateCharacter(game->charData, CharacterClass::Mage, CharacterFaction::Player, "Player2", "Fighter");
    AssignSkill(game->charData.skills[id], SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(game->charData.skills[id], SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(game->spriteData, game->charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(game->spriteData, game->weaponData, game->charData, id, "Bow");
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    game->party.emplace_back(id);

    game->state = GameState::LOAD_LEVEL;
    PushGameMode(GameModes::Level);
}

static void loadGame() {
    SaveData saveData;
    if(!LoadGameData(saveData, "savegame.json")) {
        TraceLog(LOG_DEBUG, "No existing savegame found");
        return;
    }
    game->levelFileName = saveData.currentLevel;
    game->state = GameState::LOAD_LEVEL_FROM_SAVE;
    game->levelState = saveData.levels;

    ClearAllCharacters(game->charData);
    game->spriteData.player.animationIdx.clear();
    game->spriteData.player.animData.clear();
    game->spriteData.player.renderData.clear();
    game->party.clear();
    for(auto& ch : saveData.party) {
        int id = CreateCharacter(game->charData, ch.characterClass, ch.faction, ch.name, ch.ai);
        game->charData.stats[id] = ch.stats;
        //AssignSkill(game->charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
        InitCharacterSprite(game->spriteData, game->charData.sprite[id], ch.spriteTemplate, true);
        Vector2 pos = GridToPixelPosition(ch.tilePosX, ch.tilePosY);
        SetCharacterSpritePos(game->spriteData, game->charData.sprite[id], pos);
        GiveWeapon(game->spriteData, game->weaponData, game->charData, id, ch.weaponTemplate);
        game->party.emplace_back(id);
    }

    game->quests = saveData.quests;

    PushGameMode(GameModes::Level);
}

static void saveGame() {
    SaveData saveData;
    saveData.currentLevel = game->levelFileName;
    saveData.levels = game->levelState;

    for(auto& id : game->party) {
        PartyCharacter pc;
        pc.name = game->charData.name[id];
        pc.faction = game->charData.faction[id];
        pc.characterClass = game->charData.characterClass[id];
        pc.ai = game->charData.ai[id];
        pc.stats = game->charData.stats[id];
        // save sprite template
        pc.spriteTemplate = game->charData.sprite[id].spriteTemplate;

        // save weapon template
        auto weaponId = game->charData.weaponIdx[id];
        auto weaponTplId = game->weaponData.instanceData.weaponTemplateIdx[weaponId];
        pc.weaponTemplate = game->weaponData.templateData.name[weaponTplId];
        // save position
        Vector2i pos = GetCharacterGridPosI(game->spriteData, game->charData.sprite[id]);
        pc.tilePosX = pos.x;
        pc.tilePosY = pos.y;

        saveData.party.push_back(pc);
    }

    saveData.quests = game->quests;

    SaveGameData(saveData, "savegame.json");
}

void MenuRender() {
    ClearBackground(DARKGRAY);
    DrawStatusText("Rule 34", WHITE, 10, 30);
    float offsetY = 75.0f;
    if(game->state == GameState::START_NEW_GAME) {
        if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Start New Game")) {
            StartNewGame();
        }
    } else {
        if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Resume Game")) {
            PushGameMode(GameModes::Level);
        }
        offsetY += 30.0f;
        if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Save Game")) {
            saveGame();
        }
    }
    offsetY += 30.0f;
    if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Load Game")) {
        loadGame();
    }
    offsetY += 30.0f;
    if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Options")) {
    }
    offsetY += 30.0f;
    if (GuiButton((Rectangle) {240 - 50, offsetY, 100, 20}, "Quit")) {
        PopGameMode();
    }
}

void MenuHandleInput() {
    
}


void MenuPreRender() {

}

void MenuDestroy() {
    
}

void MenuPause() {

}

void MenuResume() {

}

void SetupMenuGameMode(GameData* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Menu, MenuInit, MenuUpdate, MenuHandleInput, MenuRender, MenuPreRender, MenuDestroy, MenuPause, MenuResume);
}