//
// Created by bison on 09-02-25.
//

#include <cassert>
#include "MenuGameMode.h"
#include "raylib.h"
#include "raygui.h"
#include "ui/UI.h"

static Game* game;

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

void MenuRender() {
    ClearBackground(DARKGRAY);
    DrawStatusText("Rule 34", WHITE, 10, 30);
    if(game->state == GameState::START_NEW_GAME) {
        if (GuiButton((Rectangle) {240 - 50, 50, 100, 20}, "Start New Game")) {
            StartNewGame();
        }
    } else {
        if (GuiButton((Rectangle) {240 - 50, 50, 100, 20}, "Resume Game")) {
            PushGameMode(GameModes::Level);
        }
    }
    if (GuiButton((Rectangle) {240 - 50, 80, 100, 20}, "Load Game")) {
    }
    if (GuiButton((Rectangle) {240 - 50, 110, 100, 20}, "Options")) {
    }
    if (GuiButton((Rectangle) {240 - 50, 140, 100, 20}, "Quit")) {
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

void SetupMenuGameMode(Game* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Menu, MenuInit, MenuUpdate, MenuHandleInput, MenuRender, MenuPreRender, MenuDestroy, MenuPause, MenuResume);
}