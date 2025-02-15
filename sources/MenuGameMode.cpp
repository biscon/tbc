//
// Created by bison on 09-02-25.
//

#include "MenuGameMode.h"
#include "raylib.h"
#include "raygui.h"
#include "ui/UI.h"

void MenuInit() {
    TraceLog(LOG_INFO, "MenuInit");

}

void MenuUpdate(float dt) {
    
}

void MenuRender() {
    ClearBackground(DARKGRAY);
    DrawStatusText("Rule 34", WHITE, 10, 30);
    if (GuiButton((Rectangle) {240 - 50, 50, 100, 20}, "Start New Game")) {
        PushGameMode(GameModes::Combat);
    }
    if (GuiButton((Rectangle) {240 - 50, 80, 100, 20}, "Load Game")) {
        PushGameMode(GameModes::Combat);
    }
    if (GuiButton((Rectangle) {240 - 50, 110, 100, 20}, "Options")) {
        //SetGameMode(GameModes::Dungeon);
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

void SetupMenuGameMode() {
    CreateGameMode(GameModes::Menu, MenuInit, MenuUpdate, MenuHandleInput, MenuRender, MenuPreRender, MenuDestroy, MenuPause, MenuResume);
}