//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_GAMEDATA_H
#define SANDBOX_GAMEDATA_H

#include "WeaponData.h"
#include "CharacterData.h"
#include "SpriteData.h"
#include "NpcTemplateData.h"
#include "AnimationData.h"
#include "SkillData.h"
#include "StatusEffectData.h"
#include "TileMapData.h"
#include "SaveData.h"
#include "DialogueData.h"
#include "QuestData.h"
#include "SettingsData.h"

const int gameScreenWidth = 640;
const int gameScreenHeight = 360;
const int gameScreenWidthF = 640.0f;
const int gameScreenHeightF = 360.0f;
const int gameScreenHalfWidth = gameScreenWidth/2;
const int gameScreenHalfHeight = gameScreenHeight/2;
const float gameScreenHalfWidthF = gameScreenWidthF/2.0f;
const float gameScreenHalfHeightF = gameScreenHeightF/2.0f;

enum class GameState {
    START_NEW_GAME,
    LOAD_LEVEL,
    LOAD_LEVEL_FROM_SAVE,
    PLAY_LEVEL,
    DIALOGUE,
};

struct GameData {
    std::vector<int> party;
    std::string levelFileName;
    std::string currentLevelId;
    GameState state;
    WeaponData weaponData;
    CharacterData charData;
    SpriteData spriteData;
    NpcTemplateData npcTemplateData;
    DialogueData dialogueData;
    SettingsData settingsData;
    QuestData questData;

    // global save state
    std::unordered_map<std::string, LevelSaveState> levelState;
    std::unordered_map<std::string, QuestSaveState> questState;

    RenderTexture2D levelTarget;
    RenderTexture2D uiTarget;
};

#endif //SANDBOX_GAMEDATA_H
