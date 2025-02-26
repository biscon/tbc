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
    //game->party.reserve(10);
    game->party.emplace_back();
    Character &warrior = game->party.back();
    CreateCharacter(warrior, CharacterClass::Warrior, CharacterFaction::Player, "Player1", "Fighter");
    AssignSkill(warrior.skills, SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(warrior.skills, SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(warrior.sprite, "MaleWarrior", true);
    GiveWeapon(warrior, "Sword");
    LevelUp(warrior, true);
    LevelUp(warrior, true);
    LevelUp(warrior, true);
    LevelUp(warrior, true);


    game->party.emplace_back();
    Character &mage = game->party.back();
    CreateCharacter(mage, CharacterClass::Mage, CharacterFaction::Player, "Player2", "Fighter");
    AssignSkill(mage.skills, SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(mage.skills, SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(mage.sprite, "MaleBase", true);
    GiveWeapon(mage, "Bow");
    LevelUp(mage, true);
    LevelUp(mage, true);
    LevelUp(mage, true);
    LevelUp(mage, true);

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