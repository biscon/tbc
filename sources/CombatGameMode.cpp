//
// Created by bison on 29-01-25.
//

#include "CombatGameMode.h"
#include "audio/SoundEffect.h"
#include "character/Character.h"
#include "combat/CombatState.h"
#include "ui/CombatScreen.h"
#include "graphics/BloodPool.h"
#include "rcamera.h"
#include "raymath.h"

static Character warrior;
static Character mage;
static std::vector<Character> playerCharacters;
static std::vector<Character> enemyCharacters = {
        {CharacterClass::Warrior, "Enemy1", "Fighter", 16,  16,  5, 3, 4, 0, 0, 0, 1, {}},
        {CharacterClass::Warrior, "Enemy2", "Fighter", 16,  16,  5, 3, 4, 0, 0, 0, 1, {}},
        //{CharacterClass::Warrior, "Enemy3", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
        //{CharacterClass::Warrior, "Enemy4", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
};
static CombatState combat;
static CombatUIState combatUIState;
static SpriteSheet tileSet;
static ParticleManager particleManager;
static GridState gridState{};


void CombatInit() {
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
    PlaySoundEffect(SoundEffectType::Ambience);

    CreateCharacter(warrior, CharacterClass::Warrior, "Player1", "Fighter");
    AssignSkill(warrior.skills, SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(warrior.skills, SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(warrior.sprite, "MaleWarrior", true);
    GiveWeapon(warrior, "Sword");

    LevelUp(warrior, true);
    LevelUp(warrior, true);
    LevelUp(warrior, true);
    LevelUp(warrior, true);
    playerCharacters.push_back(warrior);



    CreateCharacter(mage, CharacterClass::Mage, "Player2", "Fighter");
    AssignSkill(mage.skills, SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(mage.skills, SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(mage.sprite, "MaleBase", true);
    GiveWeapon(mage, "Bow");

    LevelUp(mage, true);
    LevelUp(mage, true);
    LevelUp(mage, true);
    LevelUp(mage, true);
    playerCharacters.push_back(mage);


    for(auto &character : enemyCharacters) {
        InitCharacterSprite(character.sprite, "MaleNinja", true);
        GiveWeapon(character, "Bow");
        LevelUp(character, true);
        LevelUp(character, true);
        LevelUp(character, true);
        LevelUp(character, true);
    }

    InitCombat(combat, playerCharacters, enemyCharacters);
    InitCombatUIState(combatUIState);

    //LoadSpriteSheet(tileSet, ASSETS_PATH"town_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_01.json", &tileSet);
    LoadSpriteSheet(tileSet, ASSETS_PATH"sewer_tiles.png", 16, 16);
    LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_02.json", &tileSet);
    //LoadSpriteSheet(tileSet, ASSETS_PATH"forest_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_03.json", &tileSet);
    CreateParticleManager(particleManager, {0, 0}, 480, 270);

    InitGrid(gridState, &particleManager);
    SetInitialGridPositions(gridState, combat);

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

void CombatDestroy() {
    DestroyParticleManager(particleManager);
    DestroyBloodRendering();
    DestroyCombatUIState(combatUIState);
    UnloadTileMap(combat.tileMap);     // Unload tile map (free memory
    UnloadSpriteSheet(tileSet);     // Unload sprite sheet
}

void CombatUpdate(float dt) {
    float snappiness = 5.0f; // Higher values make the camera react more quickly
    float threshold = 0.5f;   // Minimum distance to consider "arrived" at target

    if (combatUIState.cameraFollowing) {
        Vector2 targetPos = {
                combatUIState.cameraTarget.x - 240.0f, // Offset by half width
                combatUIState.cameraTarget.y - 135.0f  // Offset by half height
        };

        // Smoothly move towards the adjusted target position
        combatUIState.camera.target.x += (targetPos.x - combatUIState.camera.target.x) * snappiness * dt;
        combatUIState.camera.target.y += (targetPos.y - combatUIState.camera.target.y) * snappiness * dt;

        // Check if the camera is close enough to stop following
        if (fabs(combatUIState.camera.target.x - targetPos.x) < threshold &&
            fabs(combatUIState.camera.target.y - targetPos.y) < threshold) {
            combatUIState.camera.target = targetPos; // Snap to exact target
            combatUIState.cameraFollowing = false;
            TraceLog(LOG_INFO, "Camera arrived at target");
        }
    } else {
        // Regular movement if not following a target
        combatUIState.camera.target = Vector2Add(combatUIState.camera.target, combatUIState.cameraVelocity);
    }

    // Ceil camera target to prevent jittering
    combatUIState.camera.target.x = ceil(combatUIState.camera.target.x);
    combatUIState.camera.target.y = ceil(combatUIState.camera.target.y);

    UpdateParticleManager(particleManager, dt);
    UpdateCombatScreen(combat, combatUIState, gridState, dt);
    UpdateGrid(gridState, combat, dt);
}

void CombatHandleInput() {
    float dt = GetFrameTime();
    float speed = 6.5f;
    float accelerationTime = 0.5f; // Time to reach full speed
    float decelerationTime = 0.5f; // Time to stop when no input

    float acceleration = speed / accelerationTime;
    float deceleration = speed / decelerationTime;

    // Handle horizontal movement
    if (IsKeyDown(KEY_A)) {
        combatUIState.cameraVelocity.x -= acceleration * dt;
        if (combatUIState.cameraVelocity.x < -speed) {
            combatUIState.cameraVelocity.x = -speed;
        }
    } else if (IsKeyDown(KEY_D)) {
        combatUIState.cameraVelocity.x += acceleration * dt;
        if (combatUIState.cameraVelocity.x > speed) {
            combatUIState.cameraVelocity.x = speed;
        }
    } else {
        // Decelerate smoothly
        if (combatUIState.cameraVelocity.x > 0) {
            combatUIState.cameraVelocity.x -= deceleration * dt;
            if (combatUIState.cameraVelocity.x < 0) combatUIState.cameraVelocity.x = 0;
        } else if (combatUIState.cameraVelocity.x < 0) {
            combatUIState.cameraVelocity.x += deceleration * dt;
            if (combatUIState.cameraVelocity.x > 0) combatUIState.cameraVelocity.x = 0;
        }
    }

    // Handle vertical movement
    if (IsKeyDown(KEY_W)) {
        combatUIState.cameraVelocity.y -= acceleration * dt;
        if (combatUIState.cameraVelocity.y < -speed) {
            combatUIState.cameraVelocity.y = -speed;
        }
    } else if (IsKeyDown(KEY_S)) {
        combatUIState.cameraVelocity.y += acceleration * dt;
        if (combatUIState.cameraVelocity.y > speed) {
            combatUIState.cameraVelocity.y = speed;
        }
    } else {
        // Decelerate smoothly
        if (combatUIState.cameraVelocity.y > 0) {
            combatUIState.cameraVelocity.y -= deceleration * dt;
            if (combatUIState.cameraVelocity.y < 0) combatUIState.cameraVelocity.y = 0;
        } else if (combatUIState.cameraVelocity.y < 0) {
            combatUIState.cameraVelocity.y += deceleration * dt;
            if (combatUIState.cameraVelocity.y > 0) combatUIState.cameraVelocity.y = 0;
        }
    }
}


void CombatRender() {
    // Draw combat screen
    DisplayCombatScreen(combat, combatUIState, gridState);
    BeginMode2D(combatUIState.camera);
    DrawParticleManager(particleManager);
    EndMode2D();
}

void CombatPreRender() {
    PreRenderBloodPools(combat);
    PreRenderParticleManager(particleManager);
}

void CombatPause() {

}

void CombatResume() {

}

void SetupCombatGameMode() {
    CreateGameMode(GameModes::Combat, CombatInit, CombatUpdate, CombatHandleInput, CombatRender, CombatPreRender, CombatDestroy, CombatPause, CombatResume);
}
