#include "raylib.h"

#include "raymath.h"        // Required for: Vector2Clamp()
#include "character/Character.h"
#include "combat/Combat.h"
#include "ui/CombatScreen.h"
#include "graphics/Sprite.h"
#include "ai/FighterAi.h"
#include "ui/UI.h"
#include "graphics/BloodPool.h"
#include "graphics/ParticleSystem.h"
#include "audio/SoundEffect.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))


//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void) {
    int windowWidth = 1920;
    int windowHeight = 1080;

    // Enable config flags for resizable window and vertical synchro
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(windowWidth, windowHeight, "RPG");
    SetWindowMinSize(320, 240);

    InitAudioDevice();      // Initialize audio device

    InitSoundEffectManager();
    //LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"music/ambience_cave.ogg", true);
    LoadSoundEffect(SoundEffectType::Ambience, ASSETS_PATH"sound/ambient_forest_01.ogg", true);
    //SetVolumeSoundEffect(SoundEffectType::Ambience, 0.75f);
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

    int gameScreenWidth = 480;
    int gameScreenHeight = 270;
    bool fullScreenToggled = false;

    // Render texture initialization, used to hold the rendering result so we can easily resize it
    RenderTexture2D target = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(target.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use

    //Font font = LoadFont(ASSETS_PATH"small.fnt");

    Font font2 = LoadFontEx(ASSETS_PATH"pixel-3x5.ttf", 5, nullptr, 0);


    SetTargetFPS(60);                   // Set our game to run at 60 frames-per-second
    //--------------------------------------------------------------------------------------

    // Create AI
    CreateFighterAi("Fighter");

    // Load sprite sheet and create animations
    SpriteSheet baseCharSpriteSheet;
    LoadSpriteSheet(baseCharSpriteSheet, ASSETS_PATH"base_char_32x32.png", 32, 32);

    SpriteSheet warriorSpriteSheet;
    LoadSpriteSheet(warriorSpriteSheet, ASSETS_PATH"warrior_male_16x16.png", 32, 32);

    SpriteSheet ninjaSpriteSheet;
    LoadSpriteSheet(ninjaSpriteSheet, ASSETS_PATH"ninja_male_32x32.png", 32, 32);

    SpriteSheet swordSpriteSheet;
    LoadSpriteSheet(swordSpriteSheet, ASSETS_PATH"weapon_sword_32x32.png", 32, 32);

    SpriteSheet staffSpriteSheet;
    LoadSpriteSheet(staffSpriteSheet, ASSETS_PATH"weapon_staff_32x32.png", 32, 32);

    float attackSpeed = 0.25f;

    SpriteAnimationManager spriteAnimationManager;
    // weapon walk
    CreateSpriteAnimation(spriteAnimationManager, "SwordWalkRight", SpriteAnimationType::WalkRight, &swordSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordWalkLeft", SpriteAnimationType::WalkLeft, &swordSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordWalkUp", SpriteAnimationType::WalkUp, &swordSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordWalkDown", SpriteAnimationType::WalkDown, &swordSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    // weapon attack
    CreateSpriteAnimation(spriteAnimationManager, "SwordAttackUp", SpriteAnimationType::AttackUp, &swordSpriteSheet,
                          {12, 13, 14}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordAttackRight", SpriteAnimationType::AttackRight, &swordSpriteSheet,
                          {15, 16, 17}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordAttackDown", SpriteAnimationType::AttackDown, &swordSpriteSheet,
                          {18, 19, 20}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "SwordAttackLeft", SpriteAnimationType::AttackLeft, &swordSpriteSheet,
                          {21, 22, 23}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});


    // weapon walk
    CreateSpriteAnimation(spriteAnimationManager, "StaffWalkRight", SpriteAnimationType::WalkRight, &staffSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffWalkLeft", SpriteAnimationType::WalkLeft, &staffSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffWalkUp", SpriteAnimationType::WalkUp, &staffSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffWalkDown", SpriteAnimationType::WalkDown, &staffSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    // weapon attack
    CreateSpriteAnimation(spriteAnimationManager, "StaffAttackUp", SpriteAnimationType::AttackUp, &staffSpriteSheet,
                          {12, 13, 14}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffAttackRight", SpriteAnimationType::AttackRight, &staffSpriteSheet,
                          {15, 16, 17}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffAttackDown", SpriteAnimationType::AttackDown, &staffSpriteSheet,
                          {18, 19, 20}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "StaffAttackLeft", SpriteAnimationType::AttackLeft, &staffSpriteSheet,
                          {21, 22, 23}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    

    // base
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkRight", SpriteAnimationType::WalkRight, &baseCharSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkLeft", SpriteAnimationType::WalkLeft, &baseCharSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkUp", SpriteAnimationType::WalkUp, &baseCharSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkDown", SpriteAnimationType::WalkDown, &baseCharSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    CreateSpriteAnimation(spriteAnimationManager, "BaseAttackUp", SpriteAnimationType::AttackUp, &baseCharSpriteSheet,
                          {12, 13, 14}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseAttackRight", SpriteAnimationType::AttackRight, &baseCharSpriteSheet,
                          {15, 16, 17}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseAttackDown", SpriteAnimationType::AttackDown, &baseCharSpriteSheet,
                          {18, 19, 20}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseAttackLeft", SpriteAnimationType::AttackLeft, &baseCharSpriteSheet,
                          {21, 22, 23}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});

    // warrior
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkRight", SpriteAnimationType::WalkRight, &warriorSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkLeft", SpriteAnimationType::WalkLeft, &warriorSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkUp", SpriteAnimationType::WalkUp, &warriorSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkDown", SpriteAnimationType::WalkDown, &warriorSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorAttackUp", SpriteAnimationType::AttackUp, &warriorSpriteSheet,
                          {12, 13, 14}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorAttackRight", SpriteAnimationType::AttackRight, &warriorSpriteSheet,
                          {15, 16, 17}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorAttackDown", SpriteAnimationType::AttackDown, &warriorSpriteSheet,
                          {18, 19, 20}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorAttackLeft", SpriteAnimationType::AttackLeft, &warriorSpriteSheet,
                          {21, 22, 23}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});


    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkRight", SpriteAnimationType::WalkRight, &ninjaSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkLeft", SpriteAnimationType::WalkLeft, &ninjaSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkUp", SpriteAnimationType::WalkUp, &ninjaSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkDown", SpriteAnimationType::WalkDown, &ninjaSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaAttackUp", SpriteAnimationType::AttackUp, &ninjaSpriteSheet,
                          {12, 13, 14}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaAttackRight", SpriteAnimationType::AttackRight, &ninjaSpriteSheet,
                          {15, 16, 17}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaAttackDown", SpriteAnimationType::AttackDown, &ninjaSpriteSheet,
                          {18, 19, 20}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaAttackLeft", SpriteAnimationType::AttackLeft, &ninjaSpriteSheet,
                          {21, 22, 23}, {attackSpeed, attackSpeed, attackSpeed}, {16, 30});

    // Sample player and enemy data
    /*
    std::vector<Character> playerCharacters = {
            {"Player1", "Fighter", 120, 120, 20, 10, 10, 0, 0, 0, {}, {
                {SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0},
                {SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 1, 5},
            }},
            {"Player2", "Fighter", 80,  80,  25, 5,  20, 0, 0, 0, {}, {
                {SkillType::Dodge, "Dodge", 3, true, true, 0, 0, 0},
                {SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1},
            }, {{StatusEffectType::ThreatModifier, -1, 0.75f}}},
            {"Player3", "Fighter", 100,  100,  15, 15,  5, 0, 0, 0, {}, {
                        {SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3},
                        {SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3},
                }, {
            }}
    };
    std::vector<Character> enemyCharacters = {
            {"Enemy1", "Fighter", 50,  50,  15, 2, 5, 0, 0, {}},
            {"Enemy2", "Fighter", 20, 20, 25, 2, 15, 0, 0, {}},
            {"Enemy3", "Fighter", 30, 30, 18, 5, 5, 0, 0, {}},
            {"Enemy4", "Fighter", 40, 40,  18, 5, 5, 0, 0, {}},
            {"Enemy5", "Fighter", 50, 50, 25, 7, 5, 0, 0, {}},
            {"Enemy6", "Fighter", 60, 60, 25, 7, 5, 0, 0, {}},
    };
    */

    Character warrior;
    CreateCharacter(warrior, CharacterClass::Warrior, "Player1", "Fighter");
    AssignSkill(warrior.skills, SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(warrior.skills, SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(warrior.sprite, spriteAnimationManager, "Warrior", "Sword", true);
    LevelUp(warrior, true);

    Character mage;
    CreateCharacter(mage, CharacterClass::Mage, "Player2", "Fighter");
    AssignSkill(mage.skills, SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(mage.skills, SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(mage.sprite, spriteAnimationManager, "Base", "Staff", true);
    LevelUp(mage, true);

    std::vector<Character> playerCharacters = {warrior, mage};
    
    //LevelUp(playerCharacters[0], true);

    std::vector<Character> enemyCharacters = {
            {CharacterClass::Warrior, "Enemy1", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
            {CharacterClass::Warrior, "Enemy2", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
            //{CharacterClass::Warrior, "Enemy3", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
            //{CharacterClass::Warrior, "Enemy4", "Fighter", 20,  20,  5, 3, 4, 0, 0, 0, 1, {}},
    };

    //InitCharacterSprite(playerCharacters[0].sprite, spriteAnimationManager, "BaseWalkUp", "BaseWalkDown", "BaseWalkLeft", "BaseWalkRight");
    for(auto &character : enemyCharacters) {
        InitCharacterSprite(character.sprite, spriteAnimationManager, "Ninja", "Staff", true);
        //InitCharacterSprite(character.sprite, spriteAnimationManager, "NinjaWalkUp", "NinjaWalkDown", "NinjaWalkLeft", "NinjaWalkRight");
    }

    CombatState combat;
    InitCombat(combat, playerCharacters, enemyCharacters);
    CombatUIState combatUIState = {};
    InitCombatUIState(combatUIState);

    SpriteSheet tileSet;
    //LoadSpriteSheet(tileSet, ASSETS_PATH"town_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_01.json", &tileSet);
    //LoadSpriteSheet(tileSet, ASSETS_PATH"sewer_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_02.json", &tileSet);

    LoadSpriteSheet(tileSet, ASSETS_PATH"forest_tiles.png", 16, 16);
    LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_03.json", &tileSet);

    ParticleManager particleManager;
    CreateParticleManager(particleManager, {0, 0}, 480, 270);

    GridState gridState{};
    InitGrid(gridState, spriteAnimationManager, &particleManager);
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

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        if (IsKeyPressed(KEY_F)) {
            fullScreenToggled = true;
        }

        // Update
        UpdateSoundEffects(GetFrameTime());
        UpdateCombatScreen(combat, combatUIState, gridState, GetFrameTime());
        UpdateGrid(gridState, combat, GetFrameTime());


        //----------------------------------------------------------------------------------
        // Compute required framebuffer scaling
        float scale = MIN((float) GetScreenWidth() / gameScreenWidth, (float) GetScreenHeight() / gameScreenHeight);

        // Update virtual mouse (clamped mouse value behind game screen)
        Vector2 mouse = GetMousePosition();
        Vector2 virtualMouse = {0};
        virtualMouse.x = (mouse.x - (GetScreenWidth() - (gameScreenWidth * scale)) * 0.5f) / scale;
        virtualMouse.y = (mouse.y - (GetScreenHeight() - (gameScreenHeight * scale)) * 0.5f) / scale;
        virtualMouse = Vector2Clamp(virtualMouse, (Vector2) {0, 0},
                                    (Vector2) {(float) gameScreenWidth, (float) gameScreenHeight});

        // Apply the same transformation as the virtual mouse to the real mouse (i.e. to work with raygui)
        SetMouseOffset(-(GetScreenWidth() - (gameScreenWidth * scale)) * 0.5f,
                       -(GetScreenHeight() - (gameScreenHeight * scale)) * 0.5f);
        SetMouseScale(1 / scale, 1 / scale);
        //----------------------------------------------------------------------------------

        PreRenderBloodPools(combat);

        UpdateParticleManager(particleManager, GetFrameTime());
        PreRenderParticleManager(particleManager);
        // Draw
        //----------------------------------------------------------------------------------
        // Draw everything in the render texture, note this will not be rendered on screen, yet
        BeginTextureMode(target);
        // Draw combat screen
        DisplayCombatScreen(combat, combatUIState, gridState);

        DrawParticleManager(particleManager);
        //DrawTextEx(font2, "Hello, World!. You cannot dickfucksoul to highen GI Joe 50%", (Vector2){10, 30}, (float) font2.baseSize, 1.0f, WHITE);


        //DrawText("If executed inside a window,\nyou can resize the window,\nand see the screen scaling!", 10, 25, 20, WHITE);
        //DrawText(TextFormat("Default Mouse: [%i , %i]", (int)mouse.x, (int)mouse.y), 50, 25, 10, GREEN);
        //DrawText(TextFormat("Virtual Mouse: [%i , %i]", (int)virtualMouse.x, (int)virtualMouse.y), 50, 55, 10, YELLOW);

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);     // Clear screen background

        // Draw render texture to screen, properly scaled
        DrawTexturePro(target.texture,
                       (Rectangle) {0.0f, 0.0f, (float) target.texture.width, (float) -target.texture.height},
                       (Rectangle) {(GetScreenWidth() - ((float) gameScreenWidth * scale)) * 0.5f,
                                    (GetScreenHeight() - ((float) gameScreenHeight * scale)) * 0.5f,
                                    (float) gameScreenWidth * scale, (float) gameScreenHeight * scale},
                       (Vector2) {0, 0}, 0.0f, WHITE);
        EndDrawing();
        //--------------------------------------------------------------------------------------
        if (fullScreenToggled) {
            int monitor = GetCurrentMonitor();
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            TraceLog(LOG_INFO, "Toggled Fullscreen resolution: %i x %i", GetMonitorWidth(monitor),
                     GetMonitorHeight(monitor));
            ToggleFullscreen();
            fullScreenToggled = false;
        }
    }

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);        // Unload render texture

    DestroyParticleManager(particleManager);
    DestroyBloodRendering();

    DestroyCombatUIState(combatUIState);

    //UnloadFont(font);                   // Unload custom font
    UnloadFont(font2);                   // Unload custom font
    UnloadSpriteSheet(baseCharSpriteSheet);     // Unload sprite sheet
    UnloadSpriteSheet(warriorSpriteSheet);     // Unload sprite sheet
    UnloadSpriteSheet(ninjaSpriteSheet);     // Unload sprite sheet

    UnloadTileMap(combat.tileMap);     // Unload tile map (free memory
    UnloadSpriteSheet(tileSet);     // Unload sprite sheet

    DestroySoundEffectManager();

    CloseAudioDevice();     // Close audio device

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}