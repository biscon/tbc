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

static GameData* game;
static Level level;
static LevelScreen levelScreen;
static ParticleManager particleManager;
static PlayField playField{};
const Color BACKGROUND_GREY = Color{25, 25, 25, 255};
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
                game->dialogueData.currentNpc = event.initiateDialogueEvent.npcId;
                game->dialogueData.currentDialogueNode = event.initiateDialogueEvent.dialogueNodeId;
                game->dialogueData.idleAnimPlayer = CreateSpriteAnimationPlayer(game->spriteData);
                int idleAnim = GetSpriteAnimation(game->spriteData, "SerDonaldPortraitTalkTalk");
                PlaySpriteAnimation(game->spriteData, game->dialogueData.idleAnimPlayer, idleAnim, true);
                break;
            }
            case GameEventType::EndDialogue: {
                TraceLog(LOG_INFO, "EndDialogue: npcId = %i", event.endDialogueEvent.npcId);
                playField.mode = PlayFieldMode::Explore;
                game->state = GameState::PLAY_LEVEL;
                break;
            }
            default:
                break;
        }
    }
}

static void handleCameraMovement() {
    float dt = GetFrameTime();
    float speed = 6.5f;
    float accelerationTime = 0.5f; // Time to reach full speed
    float decelerationTime = 0.5f; // Time to stop when no input

    float acceleration = speed / accelerationTime;
    float deceleration = speed / decelerationTime;

    // Handle horizontal movement
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
        // Decelerate smoothly
        if (level.camera.cameraVelocity.x > 0) {
            level.camera.cameraVelocity.x -= deceleration * dt;
            if (level.camera.cameraVelocity.x < 0) level.camera.cameraVelocity.x = 0;
        } else if (level.camera.cameraVelocity.x < 0) {
            level.camera.cameraVelocity.x += deceleration * dt;
            if (level.camera.cameraVelocity.x > 0) level.camera.cameraVelocity.x = 0;
        }
    }

    // Handle vertical movement
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
        // Decelerate smoothly
        if (level.camera.cameraVelocity.y > 0) {
            level.camera.cameraVelocity.y -= deceleration * dt;
            if (level.camera.cameraVelocity.y < 0) level.camera.cameraVelocity.y = 0;
        } else if (level.camera.cameraVelocity.y < 0) {
            level.camera.cameraVelocity.y += deceleration * dt;
            if (level.camera.cameraVelocity.y > 0) level.camera.cameraVelocity.y = 0;
        }
    }
}

void LevelInit() {
    //LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"music/ambience_cave.ogg", true);
    LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"sound/ambient_forest_01.ogg", true);
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
    CreateParticleManager(particleManager, {0, 0}, 480, 270);

    CreatePlayField(playField, &particleManager, &eventQueue);
    //SetInitialGridPositions(playField, level);

    InitBloodRendering();

    // Create effects

    //CreateBloodSplatter(particleManager, {100, 150}, 10, 20.0f);
    //CreateSmokeEffect(particleManager, {380, 150}, -1, 50);      // Longer smoke, fewer particles
    //CreateExplosionEffect(particleManager, {100, 30}, 10, 50.0f);


    /*
    Animation bloodAnim{};
    SetupBloodPoolAnimation(bloodAnim, {75,120}, 5.0f);
    combat.animations.push_back(bloodAnim);
    */
}

void LevelDestroy() {
    DestroyParticleManager(particleManager);
    DestroyBloodRendering();
    DestroyLevelScreen(levelScreen);
    DestroyLevel(game->spriteData.sheet, level);
}

void LevelUpdate(float dt) {
    UpdateCamera(level.camera, dt);
    if (!level.camera.cameraPanning) {
        level.camera.camera.target = Vector2Add(level.camera.camera.target, level.camera.cameraVelocity);
        // Ceil camera target to prevent jittering
        level.camera.camera.target.x = ceilf(level.camera.camera.target.x);
        level.camera.camera.target.y = ceilf(level.camera.camera.target.y);
    }
    UpdateCombat(*game, level, playField, dt);
    UpdateParticleManager(particleManager, dt);
    UpdateLevelScreen(game->spriteData, game->charData, level, levelScreen, dt);
    UpdatePlayField(game->spriteData, game->charData, playField, level, dt);
    UpdateDialogue(*game, dt);
}

void LevelHandleInput() {
    processEvents();
    if(game->state != GameState::DIALOGUE) {
        handleCameraMovement();
    } else {
        HandleDialogueInput(*game, eventQueue);
    }
    HandleInputPlayField(game->spriteData, game->charData, playField, level);
    HandleInputLevelScreen(game->spriteData, game->charData, levelScreen, level);

    if (IsKeyPressed(KEY_ESCAPE)) {
        PopGameMode();
        return;
    }
}

void LevelRender() {
    // Clear screen with a background color (dark gray)
    ClearBackground(BACKGROUND_GREY);
    DrawPlayField(game->spriteData, game->charData, playField, level);
    DrawLevelScreen(*game, level, levelScreen, playField);
    RenderDialogueUI(*game);
}

void LevelPreRender() {
    PreRenderBloodPools(level);
    PreRenderParticleManager(particleManager, level.camera.camera);
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
    CreateGameMode(GameModes::Level, LevelInit, LevelUpdate, LevelHandleInput, LevelRender, LevelPreRender, LevelDestroy, LevelPause, LevelResume);
}
