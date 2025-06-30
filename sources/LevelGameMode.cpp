//
// Created by bison on 29-01-25.
//

#include <cassert>
#include "LevelGameMode.h"
#include "audio/SoundEffect.h"
#include "character/Character.h"
#include "level/Level.h"
#include "level/LevelScreen.h"
#include "graphics/BloodPool.h"
#include "rcamera.h"
#include "util/GameEventQueue.h"
#include "level/CombatEngine.h"
#include "ui/Dialogue.h"
#include "graphics/Animation.h"
#include "ui/PartySideBar.h"
#include "ui/Inventory.h"

static GameData* game;
static Level level;
static ParticleManager particleManager;
static PlayField playField{};

static void moveParty(Vector2i target) {
    TraceLog(LOG_INFO, "MoveParty event,target: %d,%d", target.x, target.y);
    playField.activeMoves.clear();
    MoveCharacter(*game, playField, level, game->ui.selectedCharacter, target);
    // move the rest partially
    for(int i = 0; i < (int)level.partyCharacters.size(); i++) {
        if(level.partyCharacters[i] != game->ui.selectedCharacter) {
            MoveCharacterPartial(*game, playField, level, level.partyCharacters[i], target);
        }
    }
}

static void processEvents() {
    GameEvent event{};
    while(GetNextEvent(game->ui.eventQueue, event)) {
        switch(event.type) {
            case GameEventType::MoveParty: {
                StartCameraPanToTilePos(level.camera, event.moveParty.target, 250.0f);
                moveParty(event.moveParty.target);
                PlaySoundEffect(SoundEffectType::Select);
                PlaySoundEffect(SoundEffectType::Footstep);
                /*
                Animation anim{};
                SetupFancyTextAnimation(anim,
                                        "The system is compromised!",
                                        300,
                                        2.0f,   // holdDuration
                                        0.5f,     // initialDelay
                                        0.05f,    // letterPause (reveal speed)
                                        1.0f);    // fadeOut
                level.animations.push_back(anim);
                 */
                break;
            }
            case GameEventType::PartySpotted: {
                playField.mode = PlayFieldMode::None;
                StartCombat(game->spriteData, game->charData, level, event.partySpotted.spotter, 5);
                break;
            }
            case GameEventType::EndCombat: {
                level.turnState = TurnState::None;
                playField.mode = PlayFieldMode::Explore;
                for(auto& c : level.partyCharacters) {
                    // Set initial animation to paused
                    CharacterSprite& sprite = game->charData.sprite[c];
                    StartPausedCharacterSpriteAnim(game->spriteData, sprite, SpriteAnimationType::WalkDown, true);
                    game->charData.orientation[c] = Orientation::Down;
                    game->charData.statusEffects[c].clear();
                    if(game->charData.stats[c].health <= 0) {
                        game->charData.stats[c].health = 1;
                        SetCharacterSpriteRotation(game->spriteData, sprite, 0);
                    }
                }
                break;
            }
            case GameEventType::ExitLevel: {
                TraceLog(LOG_INFO, "ExitLevel: %s, spawnPoint: %s", event.exitLevelEvent.levelFile, event.exitLevelEvent.spawnPoint);
                game->levelFileName = std::string(event.exitLevelEvent.levelFile);
                ResetPlayField(playField);
                LoadLevel(*game, level, game->levelFileName);
                std::string spawnPoint = std::string(event.exitLevelEvent.spawnPoint);
                AddPartyToLevel(game->spriteData, game->charData, level, game->party, spawnPoint);
                StartCameraPanToTargetCharTime(game->spriteData, game->charData, level.camera, game->party[0], 0.01f);
                game->state = GameState::PLAY_LEVEL;
                playField.mode = PlayFieldMode::Explore;
                break;
            }
            case GameEventType::InitiateDialogue: {
                playField.mode = PlayFieldMode::None;
                game->state = GameState::DIALOGUE;
                TraceLog(LOG_INFO, "InitiateDialogue: npcId = %i, dialogueNodeId = %i", event.initiateDialogueEvent.npcId, event.initiateDialogueEvent.dialogueNodeId);
                InitiateDialogue(*game, event.initiateDialogueEvent.dialogueNodeId, event.initiateDialogueEvent.npcId);
                break;
            }
            case GameEventType::EndDialogue: {
                TraceLog(LOG_INFO, "EndDialogue: npcId = %i", event.endDialogueEvent.npcId);
                playField.mode = PlayFieldMode::Explore;
                game->state = GameState::PLAY_LEVEL;
                break;
            }
            case GameEventType::StartQuest: {
                const Quest& quest = game->questData.quests[event.startQuestEvent.questId];
                Animation textAnim1{};
                Animation textAnim2{};
                SetupFancyTextAnimation(textAnim1, "Quest started:", 10, 285, 2.0f, 0.5f, 0.05f, 1.0f);
                SetupFancyTextAnimation(textAnim2, quest.title.c_str(), 20, 300, 2.0f, 1.0f, 0.05f, 1.0f);
                level.animations.push_back(textAnim1);
                level.animations.push_back(textAnim2);
                break;
            }
            case GameEventType::OpenInventory: {
                playField.mode = PlayFieldMode::None;
                game->state = GameState::INVENTORY;
                game->ui.selectedCharacter = event.openInventoryEvent.charId;
                //InitInventory(*game);
                break;
            }
            case GameEventType::CloseInventory: {
                playField.mode = PlayFieldMode::Explore;
                game->state = GameState::PLAY_LEVEL;
                break;
            }
            case GameEventType::OpenMenu: {
                PopGameMode();
                break;
            }
            default:
                break;
        }
    }
}

static void handleCameraMovement() {
    float dt = GetFrameTime();
    float speed = 8.0f;
    float accelerationTime = 0.75f;
    float decelerationTime = 0.75f;

    float acceleration = speed / accelerationTime;
    float deceleration = speed / decelerationTime;

    // X-axis movement (A/D)
    if (!level.camera.cameraLockX) {
        if (IsKeyDown(KEY_A)) {
            level.camera.cameraVelocity.x -= acceleration * dt;
            if (level.camera.cameraVelocity.x < -speed) {
                level.camera.cameraVelocity.x = -speed;
            }
        } else if (IsKeyDown(KEY_D)) {
            level.camera.cameraVelocity.x += acceleration * dt;
            if (level.camera.cameraVelocity.x > speed) {
                level.camera.cameraVelocity.x = speed;
            }
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.x > 0.0f) {
                level.camera.cameraVelocity.x -= deceleration * dt;
                if (level.camera.cameraVelocity.x < 0.0f) level.camera.cameraVelocity.x = 0.0f;
            } else if (level.camera.cameraVelocity.x < 0.0f) {
                level.camera.cameraVelocity.x += deceleration * dt;
                if (level.camera.cameraVelocity.x > 0.0f) level.camera.cameraVelocity.x = 0.0f;
            }
        }
    } else {
        // Reset velocity if axis is locked
        level.camera.cameraVelocity.x = 0.0f;
    }

    // Y-axis movement (W/S)
    if (!level.camera.cameraLockY) {
        if (IsKeyDown(KEY_W)) {
            level.camera.cameraVelocity.y -= acceleration * dt;
            if (level.camera.cameraVelocity.y < -speed) {
                level.camera.cameraVelocity.y = -speed;
            }
        } else if (IsKeyDown(KEY_S)) {
            level.camera.cameraVelocity.y += acceleration * dt;
            if (level.camera.cameraVelocity.y > speed) {
                level.camera.cameraVelocity.y = speed;
            }
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.y > 0.0f) {
                level.camera.cameraVelocity.y -= deceleration * dt;
                if (level.camera.cameraVelocity.y < 0.0f) level.camera.cameraVelocity.y = 0.0f;
            } else if (level.camera.cameraVelocity.y < 0.0f) {
                level.camera.cameraVelocity.y += deceleration * dt;
                if (level.camera.cameraVelocity.y > 0.0f) level.camera.cameraVelocity.y = 0.0f;
            }
        }
    } else {
        // Reset velocity if axis is locked
        level.camera.cameraVelocity.y = 0.0f;
    }
}

void LevelInit() {
    LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"music/ambience_cave.ogg", true);
    //LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"sound/ambient_forest_01.ogg", true);
    SetVolumeSoundEffect(SoundEffectType::Ambience, 0.75f);
    LoadSoundEffect(SoundEffectType::Footstep, ASSETS_PATH"sound/footstep_dirt_03.wav", true, 0.075f);
    SetVolumeSoundEffect(SoundEffectType::Footstep, 0.75f);
    LoadSoundEffect(SoundEffectType::Select, ASSETS_PATH"sound/select_01.wav", false);
    LoadSoundEffect(SoundEffectType::MeleeHit, ASSETS_PATH"sound/melee_hit_01.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeHit, ASSETS_PATH"sound/melee_hit_02.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeHit, ASSETS_PATH"sound/melee_hit_03.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeHit, ASSETS_PATH"sound/melee_hit_04.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeCrit, ASSETS_PATH"sound/melee_crit_01.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeCrit, ASSETS_PATH"sound/melee_crit_02.ogg", false);
    LoadSoundEffect(SoundEffectType::MeleeMiss, ASSETS_PATH"sound/melee_miss_01.wav", false);
    LoadSoundEffect(SoundEffectType::MeleeMiss, ASSETS_PATH"sound/melee_miss_02.wav", false);
    LoadSoundEffect(SoundEffectType::MeleeMiss, ASSETS_PATH"sound/melee_miss_03.wav", false);
    SetVolumeSoundEffect(SoundEffectType::MeleeMiss, 0.65f);
    LoadSoundEffect(SoundEffectType::HumanDeath, ASSETS_PATH"sound/human_die_01.wav", false);
    LoadSoundEffect(SoundEffectType::HumanDeath, ASSETS_PATH"sound/human_die_02.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_01.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_02.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_03.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_04.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_05.wav", false);
    LoadSoundEffect(SoundEffectType::HumanPain, ASSETS_PATH"sound/human_pain_06.wav", false);
    SetVolumeSoundEffect(SoundEffectType::HumanPain, 0.50f);
    LoadSoundEffect(SoundEffectType::Victory, ASSETS_PATH"sound/jingle_victory.wav", false);
    LoadSoundEffect(SoundEffectType::Defeat, ASSETS_PATH"sound/jingle_defeat.wav", false);
    LoadSoundEffect(SoundEffectType::StartRound, ASSETS_PATH"sound/start_round.wav", false);
    SetVolumeSoundEffect(SoundEffectType::StartRound, 0.75f);
    LoadSoundEffect(SoundEffectType::Burning, ASSETS_PATH"sound/burning_01.ogg", false);
    //PlaySoundEffect(SoundEffectType::Ambience);

    CreateLevel(level);
    CreateLevelScreen(*game);
    CreateParticleManager(particleManager, {0, 0}, gameScreenWidth, gameScreenHeight);

    CreatePlayField(playField, &particleManager);

    InitBloodRendering();
    InitInventory(*game);

}

void LevelDestroy() {
    DestroyParticleManager(particleManager);
    DestroyBloodRendering();
    DestroyLevelScreen(*game);
    DestroyLevel(game->spriteData.sheet, level);
}

void LevelUpdate(float dt) {
    UpdateCamera(level.camera, dt);

    UpdateCombat(*game, level, playField, dt);
    UpdateParticleManager(particleManager, dt);
    UpdateLevelScreen(*game, level, dt);
    UpdatePlayField(*game, playField, level, dt);
    UpdateDialogue(*game, dt);
    UpdatePartySideBar(*game, dt);
    UpdateInventory(*game, dt);

    UpdateVisibilityMap(*game, level);
    UpdateVisibilityTexture(level.lighting);
    //PropagateLight(level.lighting, level.tileMap);
}

void LevelHandleInput() {
    processEvents();
    if(game->state != GameState::DIALOGUE && game->state != GameState::INVENTORY) {
        handleCameraMovement();
        if(!HandlePartySideBarInput(*game)) {
            HandleInputPlayField(*game, playField, level);
        }
        HandleInputLevelScreen(*game, level);
    } else {
        if(game->state == GameState::DIALOGUE) HandleDialogueInput(*game);
        if(game->state == GameState::INVENTORY) {
            HandleInventoryInput(*game);
            HandlePartySideBarInput(*game);
        }
    }

    if (IsKeyPressed(KEY_ESCAPE) && (game->state != GameState::INVENTORY)) {
        PopGameMode();
        return;
    }
}

void LevelRenderLevel() {
    /*
    int charId = game->party[0];
    Vector2i pos = GetCharacterGridPosI(game->spriteData, game->charData.sprite[charId]);
    MoveLight(level.lighting.lights[0], pos.x, pos.y);
     */

    ClearBackground(BLACK);

    Vector2 topLeftWorld = { 0, 0 };
    Vector2 bottomRightWorld = {
            (float)level.camera.worldWidth,
            (float)level.camera.worldHeight
    };

// Convert world bounds to screen coordinates using the camera
    Vector2 topLeftScreen = GetWorldToScreen2D(topLeftWorld, level.camera.camera);
    Vector2 bottomRightScreen = GetWorldToScreen2D(bottomRightWorld, level.camera.camera);

// Compute screen-space rectangle
    int scissorX = (int)topLeftScreen.x;
    int scissorY = (int)topLeftScreen.y;
    int scissorW = (int)(bottomRightScreen.x - topLeftScreen.x);
    int scissorH = (int)(bottomRightScreen.y - topLeftScreen.y);

// Clamp to stay within screen bounds
    if (scissorX < 0) {
        scissorW += scissorX;
        scissorX = 0;
    }
    if (scissorY < 0) {
        scissorH += scissorY;
        scissorY = 0;
    }
    if (scissorX + scissorW > gameScreenWidth) {
        scissorW = gameScreenWidth - scissorX;
    }
    if (scissorY + scissorH > gameScreenHeight) {
        scissorH = gameScreenHeight - scissorY;
    }

    // Only apply scissor if there's a valid area
    if (scissorW > 0 && scissorH > 0) {
        BeginScissorMode(scissorX, scissorY, scissorW, scissorH);
        DrawPlayField(*game, playField, level);
        EndScissorMode();
    }
}

void LevelRenderUi() {
    DrawLevelScreen(*game, level, playField);
    RenderPartySideBarUI(*game);
    RenderDialogueUI(*game);
    if(game->state == GameState::INVENTORY)
        RenderInventoryUI(*game);
}

void LevelPreRender() {
    PreRenderBloodPools(level);
    PreRenderParticleManager(level.lighting, particleManager, level.camera.camera);
}

void LevelPause() {

}

void LevelResume() {
    TraceLog(LOG_INFO, "LevelResume");
    if(game->state == GameState::LOAD_LEVEL) {
        LoadLevel(*game, level, game->levelFileName);
        AddPartyToLevel(game->spriteData, game->charData, level, game->party, "default");
        game->state = GameState::PLAY_LEVEL;
        playField.mode = PlayFieldMode::Explore;
    }
    if(game->state == GameState::LOAD_LEVEL_FROM_SAVE) {
        LoadLevel(*game, level, game->levelFileName);
        AddPartyToLevelNoPositioning(game->spriteData, game->charData, level, game->party);
        StartCameraPanToTargetCharTime(game->spriteData, game->charData, level.camera, game->party[0], 0.01f);
        game->state = GameState::PLAY_LEVEL;
        playField.mode = PlayFieldMode::Explore;
    }
}

void SetupLevelGameMode(GameData* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Level, LevelInit, LevelUpdate, LevelHandleInput, LevelRenderLevel, LevelRenderUi, LevelPreRender, LevelDestroy, LevelPause, LevelResume);
}
