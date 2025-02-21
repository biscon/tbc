//
// Created by bison on 29-01-25.
//

#include "LevelGameMode.h"
#include "audio/SoundEffect.h"
#include "character/Character.h"
#include "level/Level.h"
#include "level/LevelScreen.h"
#include "graphics/BloodPool.h"
#include "rcamera.h"
#include "raymath.h"
#include "util/GameEventQueue.h"

static Game* game;

/*
static std::vector<Character> enemyCharacters = {
        {CharacterClass::Warrior, "Enemy1", "Fighter", 16,  16,  5, 3, 4, 0, 0, 0, 1, {}},
        {CharacterClass::Warrior, "Enemy2", "Fighter", 16,  16,  5, 3, 4, 0, 0, 0, 1, {}},
        //{CharacterClass::Warrior, "Enemy3", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
        //{CharacterClass::Warrior, "Enemy4", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
};
 */
static Level level;
static LevelScreen levelScreen;
static ParticleManager particleManager;
static PlayField playField{};
const Color BACKGROUND_GREY = Color{25, 25, 25, 255};
static GameEventQueue eventQueue;

static void moveParty(Vector2i target) {
    TraceLog(LOG_INFO, "MoveParty event,target: %d,%d", target.x, target.y);
    playField.activeMoves.clear();
    MoveCharacter(playField, level, level.playerCharacters[0], target);
    // move the rest partially
    for(int i = 1; i < (int)level.playerCharacters.size(); i++) {
        MoveCharacterPartial(playField, level, level.playerCharacters[i], target);
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
            default:
                break;
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


    /*
    for(auto &character : enemyCharacters) {
        InitCharacterSprite(character.sprite, "MaleNinja", true);
        GiveWeapon(character, "Bow");
        LevelUp(character, true);
        LevelUp(character, true);
        LevelUp(character, true);
        LevelUp(character, true);
    }
    */

    CreateLevel(level);
    CreateLevelScreen(levelScreen, &eventQueue);

    //LoadSpriteSheet(tileSet, ASSETS_PATH"sewer_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_02.json", &tileSet);
    //LoadSpriteSheet(tileSet, ASSETS_PATH"forest_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_03.json", &tileSet);
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
    DestroyLevel(level);
}

void LevelUpdate(float dt) {
    UpdateCamera(level.camera, dt);
    if (!level.camera.cameraPanning) {
        level.camera.camera.target = Vector2Add(level.camera.camera.target, level.camera.cameraVelocity);
    }
    UpdateParticleManager(particleManager, dt);
    UpdateLevelScreen(level, levelScreen, playField, dt);
    UpdatePlayField(playField, level, dt);
}

void LevelHandleInput() {
    processEvents();

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
    HandleInputPlayField(playField, level);
    HandleInputLevelScreen(levelScreen, level);
}

void LevelRender() {
    // Clear screen with a background color (dark gray)
    ClearBackground(BACKGROUND_GREY);
    DrawPlayField(playField, level);
    DrawLevelScreen(level, levelScreen, playField);
    DrawParticleManager(particleManager);
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
        LoadLevel(level, game->levelFileName);
        AddPartyToLevel(level, game->party);
        game->state = GameState::PLAY_LEVEL;
        playField.mode = PlayFieldMode::Move;
    }
}

void SetupLevelGameMode(Game* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Level, LevelInit, LevelUpdate, LevelHandleInput, LevelRender, LevelPreRender, LevelDestroy, LevelPause, LevelResume);
}
