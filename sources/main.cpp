/*******************************************************************************************
*
*   raylib [core] example - window scale letterbox (and virtual mouse)
*
*   Example originally created with raylib 2.5, last time updated with raylib 4.0
*
*   Example contributed by Anata (@anatagawa) and reviewed by Ramon Santamaria (@raysan5)
*
*   Example licensed under an unmodified zlib/libpng license, which is an OSI-certified,
*   BSD-like license that allows static linking with closed source software
*
*   Copyright (c) 2019-2024 Anata (@anatagawa) and Ramon Santamaria (@raysan5)
*
********************************************************************************************/

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

    SpriteAnimationManager spriteAnimationManager;
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkRight", SpriteAnimationType::WalkRight, &baseCharSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkLeft", SpriteAnimationType::WalkLeft, &baseCharSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkUp", SpriteAnimationType::WalkUp, &baseCharSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "BaseWalkDown", SpriteAnimationType::WalkDown, &baseCharSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkRight", SpriteAnimationType::WalkRight, &warriorSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkLeft", SpriteAnimationType::WalkLeft, &warriorSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkUp", SpriteAnimationType::WalkUp, &warriorSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "WarriorWalkDown", SpriteAnimationType::WalkDown, &warriorSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkRight", SpriteAnimationType::WalkRight, &ninjaSpriteSheet,
                          {3, 4, 5}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkLeft", SpriteAnimationType::WalkLeft, &ninjaSpriteSheet,
                          {9, 10, 11}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkUp", SpriteAnimationType::WalkUp, &ninjaSpriteSheet,
                          {0, 1, 2}, {0.15f, 0.15f, 0.15f}, {16, 30});
    CreateSpriteAnimation(spriteAnimationManager, "NinjaWalkDown", SpriteAnimationType::WalkDown, &ninjaSpriteSheet,
                          {6, 7, 8}, {0.15f, 0.15f, 0.15f}, {16, 30});

    // Sample player and enemy data
    std::vector<Character> playerCharacters = {
            {"Player1", "Fighter", 120, 120, 20, 10, 10, 0, 0, 0, {}, {
                {SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0},
                {SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 1, 5},
            }},
            {"Player2", "Fighter", 80,  80,  25, 5,  20, 0, 0, 0, {}, {
                {SkillType::Dodge, "Dodge", 3, true, true, 0, 0, 0},
                {SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1},
            }, {{StatusEffectType::ThreatModifier, -1, 0.75f}}},
            /*
            {"Player3", "Fighter", 100,  100,  15, 15,  5, 0, 0, 0, {}, {
                        {SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3},
                        {SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3},
                }, {
            }}
                */
    };
    std::vector<Character> enemyCharacters = {
            {"Enemy1", "Fighter", 50,  50,  15, 2, 5, 0, 0, {}},
            {"Enemy2", "Fighter", 20, 20, 25, 2, 15, 0, 0, {}},
            {"Enemy3", "Fighter", 30, 30, 18, 5, 5, 0, 0, {}},
            {"Enemy4", "Fighter", 40, 40,  18, 5, 5, 0, 0, {}},
            {"Enemy5", "Fighter", 50, 50, 25, 7, 5, 0, 0, {}},
            {"Enemy6", "Fighter", 60, 60, 25, 7, 5, 0, 0, {}},
    };

    for(auto &character : playerCharacters) {
        InitCharacterSprite(character.sprite, spriteAnimationManager, "WarriorWalkUp", "WarriorWalkDown", "WarriorWalkLeft", "WarriorWalkRight");
    }
    InitCharacterSprite(playerCharacters[0].sprite, spriteAnimationManager, "BaseWalkUp", "BaseWalkDown", "BaseWalkLeft", "BaseWalkRight");
    for(auto &character : enemyCharacters) {
        //InitCharacterSprite(character.sprite, spriteAnimationManager, "BaseWalkUp", "BaseWalkDown", "BaseWalkLeft", "BaseWalkRight");
        InitCharacterSprite(character.sprite, spriteAnimationManager, "NinjaWalkUp", "NinjaWalkDown", "NinjaWalkLeft", "NinjaWalkRight");
    }

    CombatState combat;
    InitCombat(combat, playerCharacters, enemyCharacters);
    CombatUIState combatUIState = {};
    InitCombatUIState(combatUIState);

    SpriteSheet tileSet;
    //LoadSpriteSheet(tileSet, ASSETS_PATH"town_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_01.json", &tileSet);
    LoadSpriteSheet(tileSet, ASSETS_PATH"sewer_tiles.png", 16, 16);
    LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_02.json", &tileSet);

    //LoadSpriteSheet(tileSet, ASSETS_PATH"forest_tiles.png", 16, 16);
    //LoadTileMap(combat.tileMap, ASSETS_PATH"test_map_03.json", &tileSet);

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

    CloseAudioDevice();     // Close audio device

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------

    return 0;
}