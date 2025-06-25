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
#include "raymath.h"
#include "util/GameEventQueue.h"
#include "level/CombatEngine.h"
#include "game/Dialogue.h"
#include "graphics/Animation.h"

static GameData* game;
static Level level;
static LevelScreen levelScreen;
static ParticleManager particleManager;
static PlayField playField{};
const Color BACKGROUND_GREY = Color{15, 15, 15, 255};
static GameEventQueue eventQueue;

static void moveParty(Vector2i target) {
    TraceLog(LOG_INFO, "MoveParty event,target: %d,%d", target.x, target.y);
    playField.activeMoves.clear();
    MoveCharacter(game->spriteData, game->charData, playField, level, level.partyCharacters[0], target);
    // move the rest partially
    for(int i = 1; i < (int)level.partyCharacters.size(); i++) {
        MoveCharacterPartial(game->spriteData, game->charData, playField, level, level.partyCharacters[i], target);
    }
}

static void processEvents() {
    GameEvent event{};
    while(GetNextEvent(eventQueue, event)) {
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
                InitiateDialogue(*game, event.initiateDialogueEvent.dialogueNodeId, event.initiateDialogueEvent.npcId, eventQueue);
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
                Animation textAnim{};
                SetupFancyTextAnimation(textAnim, TextFormat("'%s' started", quest.title.c_str()), 300, 2.0f, 0.5f, 0.05f, 1.0f);
                level.animations.push_back(textAnim);
                break;
            }
            default:
                break;
        }
    }
}

static void handleCameraMovementOLD() {
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

// These can be placed at file scope or made configurable:
const int EDGE_SCROLL_ZONE_X = 16;  // Wider horizontal edge zone
const int EDGE_SCROLL_ZONE_Y = 9;  // Narrower vertical edge zone

static void handleCameraMovement() {
    float dt = GetFrameTime();
    float speed = 8.0f;
    float accelerationTime = 0.75f;
    float decelerationTime = 0.75f;

    float acceleration = speed / accelerationTime;
    float deceleration = speed / decelerationTime;

    Vector2 mouse = GetMousePosition();
    int screenWidth = gameScreenWidth;
    int screenHeight = gameScreenHeight;

    bool scrollLeft = mouse.x <= EDGE_SCROLL_ZONE_X;
    bool scrollRight = mouse.x >= screenWidth - EDGE_SCROLL_ZONE_X;
    bool scrollUp = mouse.y <= EDGE_SCROLL_ZONE_Y;
    bool scrollDown = mouse.y >= screenHeight - EDGE_SCROLL_ZONE_Y;

    // X-axis movement (A/D + mouse edge scroll)
    if (!level.camera.cameraLockX) {
        if (IsKeyDown(KEY_A) || scrollLeft) {
            level.camera.cameraVelocity.x -= acceleration * dt;
            if (level.camera.cameraVelocity.x < -speed)
                level.camera.cameraVelocity.x = -speed;
        } else if (IsKeyDown(KEY_D) || scrollRight) {
            level.camera.cameraVelocity.x += acceleration * dt;
            if (level.camera.cameraVelocity.x > speed)
                level.camera.cameraVelocity.x = speed;
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.x > 0.0f) {
                level.camera.cameraVelocity.x -= deceleration * dt;
                if (level.camera.cameraVelocity.x < 0.0f)
                    level.camera.cameraVelocity.x = 0.0f;
            } else if (level.camera.cameraVelocity.x < 0.0f) {
                level.camera.cameraVelocity.x += deceleration * dt;
                if (level.camera.cameraVelocity.x > 0.0f)
                    level.camera.cameraVelocity.x = 0.0f;
            }
        }
    } else {
        level.camera.cameraVelocity.x = 0.0f;
    }

    // Y-axis movement (W/S + mouse edge scroll)
    if (!level.camera.cameraLockY) {
        if (IsKeyDown(KEY_W) || scrollUp) {
            level.camera.cameraVelocity.y -= acceleration * dt;
            if (level.camera.cameraVelocity.y < -speed)
                level.camera.cameraVelocity.y = -speed;
        } else if (IsKeyDown(KEY_S) || scrollDown) {
            level.camera.cameraVelocity.y += acceleration * dt;
            if (level.camera.cameraVelocity.y > speed)
                level.camera.cameraVelocity.y = speed;
        } else {
            // Decelerate when no input
            if (level.camera.cameraVelocity.y > 0.0f) {
                level.camera.cameraVelocity.y -= deceleration * dt;
                if (level.camera.cameraVelocity.y < 0.0f)
                    level.camera.cameraVelocity.y = 0.0f;
            } else if (level.camera.cameraVelocity.y < 0.0f) {
                level.camera.cameraVelocity.y += deceleration * dt;
                if (level.camera.cameraVelocity.y > 0.0f)
                    level.camera.cameraVelocity.y = 0.0f;
            }
        }
    } else {
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
    CreateLevelScreen(levelScreen, &eventQueue);
    CreateParticleManager(particleManager, {0, 0}, gameScreenWidth, gameScreenHeight);

    CreatePlayField(playField, &particleManager, &eventQueue);

    InitBloodRendering();

}

void LevelDestroy() {
    DestroyParticleManager(particleManager);
    DestroyBloodRendering();
    DestroyLevelScreen(levelScreen);
    DestroyLevel(game->spriteData.sheet, level);
}

void LevelUpdate(float dt) {
    UpdateCamera(level.camera, dt);

    UpdateCombat(*game, level, playField, dt);
    UpdateParticleManager(particleManager, dt);
    UpdateLevelScreen(game->spriteData, game->charData, level, levelScreen, dt);
    UpdatePlayField(game->spriteData, game->charData, playField, level, dt);
    UpdateDialogue(*game, dt);

    UpdateVisibilityMap(*game, level);
    UpdateVisibilityTexture(level.lighting);
    //PropagateLight(level.lighting, level.tileMap);
}

void LevelHandleInput() {
    processEvents();
    if(game->state != GameState::DIALOGUE) {
        handleCameraMovement();
    } else {
        HandleDialogueInput(*game, eventQueue);
    }
    HandleInputPlayField(*game, playField, level);
    HandleInputLevelScreen(game->spriteData, game->charData, levelScreen, level);

    if (IsKeyPressed(KEY_ESCAPE)) {
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
        DrawPlayField(game->spriteData, game->charData, playField, level);
        EndScissorMode();
    }
}

void LevelRenderUi() {
    DrawLevelScreen(*game, level, levelScreen, playField);
    RenderDialogueUI(*game);
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
