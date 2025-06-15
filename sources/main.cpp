#include "raylib.h"

#include "raymath.h"        // Required for: Vector2Clamp()
#include "graphics/SpriteAnimation.h"
#include "ai/FighterAi.h"
#include "audio/SoundEffect.h"
#include "character/Weapon.h"
#include "LevelGameMode.h"
#include "MenuGameMode.h"
#include "game/Game.h"
#include "character/Npc.h"
#include "game/Dialogue.h"
#include "game/Quest.h"
#include "game/Settings.h"

#define MAX(a, b) ((a)>(b)? (a) : (b))
#define MIN(a, b) ((a)<(b)? (a) : (b))

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main() {
    GameData game;
    CreateGame(game, "town_level.json");
    SetupMenuGameMode(&game);
    SetupLevelGameMode(&game);

    int windowWidth = 960;
    int windowHeight = 540;

    SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    //SetConfigFlags(FLAG_BORDERLESS_WINDOWED_MODE);
    // Enable config flags for resizable window and vertical synchro
    //SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(windowWidth, windowHeight, "RPG");

    // Get monitor size in points (not pixels!)
    int monitor = GetCurrentMonitor();
    int monitorWidth = GetMonitorWidth(monitor);
    int monitorHeight = GetMonitorHeight(monitor);
    TraceLog(LOG_INFO, "Monitor res: %i x %i", monitorWidth, monitorHeight);
    TraceLog(LOG_INFO, "Screen res: %i x %i", GetScreenWidth(), GetScreenHeight());
    TraceLog(LOG_INFO, "Render res: %i x %i", GetRenderWidth(), GetRenderHeight());
    /*
    Vector2 dpi = GetWindowScaleDPI();
    TraceLog(LOG_INFO, "WindowScaleDPI = %f,%f", dpi.x, dpi.y);

    float scaledWidth = dpi.x * (float) windowWidth;
    float scaledHeight = dpi.y * (float) windowHeight;
    TraceLog(LOG_INFO, "Scaled res: %i x %i", (int) scaledWidth, (int) scaledHeight);
    SetWindowSize((int) scaledWidth, (int) scaledHeight);
    SetWindowPosition(0, 0);

    // Important: put window on top so macOS puts it over the dock/menu bar
    SetWindowState(FLAG_WINDOW_TOPMOST);
    */


    //ToggleBorderlessWindowed();


    //SetWindowMinSize(320, 240);
    SetExitKey(0);

    InitSettings(game.settingsData, "settings.json");

    InitAudioDevice();      // Initialize audio device

    InitSoundEffectManager();


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

    InitDialogueData(game.dialogueData, ASSETS_PATH"dialogue.json");
    InitQuestData(game, ASSETS_PATH"quests.json");
    InitSpriteAnimationData(game.spriteData, ASSETS_PATH"animations.json");
    InitWeaponTemplateData(game.weaponData.templateData, ASSETS_PATH"weapons.json");
    InitNpcTemplateData(game.npcTemplateData, ASSETS_PATH"npcs.json");

    //SetGameMode(GameModes::Combat);
    //SetGameMode(GameModes::Dungeon);
    InitGameMode();
    PushGameMode(GameModes::Menu);

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        GameModeFlags flags = GetGameModeFlags();
        if (flags.quitGame) {
            break;
        }

        // Setup Mouse scaling and offset
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


        // handle input
        HandleInputGameMode();
        if (IsKeyPressed(KEY_F)) {
            fullScreenToggled = true;
        }
        float dt = GetFrameTime();

        // Update
        UpdateSoundEffects(dt);
        UpdateGameMode(dt);


        PreRenderGameMode();

        BeginTextureMode(target);
        RenderGameMode();
        DrawTextEx(font2, TextFormat("FPS: %i", GetFPS()), (Vector2) {1, 1}, 5, 1, GREEN);
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

    DestroyGameMode();

    // De-Initialization
    //--------------------------------------------------------------------------------------
    UnloadRenderTexture(target);        // Unload render texture

    //UnloadFont(font);                   // Unload custom font
    UnloadFont(font2);                   // Unload custom font

    DestroySpriteAnimationData(game.spriteData);
    DestroySoundEffectManager();
    CloseAudioDevice();     // Close audio device

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    DestroyGame(game);
    return 0;
}