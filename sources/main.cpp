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
    CreateGame(game, "indoor_level.json");
    SetupMenuGameMode(&game);
    SetupLevelGameMode(&game);

    //SetConfigFlags(FLAG_WINDOW_UNDECORATED);
    //SetConfigFlags(FLAG_BORDERLESS_WINDOWED_MODE);
    // Enable config flags for resizable window and vertical synchro
    SetConfigFlags(FLAG_VSYNC_HINT);
    InitWindow(gameScreenWidth, gameScreenHeight, "RPG");

    SetExitKey(0);

    InitSettings(game.settingsData, "settings.json");
    ApplySettings(game.settingsData);


    InitAudioDevice();      // Initialize audio device
    InitSoundEffectManager();



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


    InitGameMode();
    PushGameMode(GameModes::Menu);

    SettingsData& settings = game.settingsData;

    // Main game loop
    while (!WindowShouldClose())        // Detect window close button or ESC key
    {
        GameModeFlags flags = GetGameModeFlags();
        if (flags.quitGame) {
            break;
        }

        int windowWidth = GetScreenWidth();     // might be 1920 in borderless
        int windowHeight = GetScreenHeight();   // might be 1080 in borderless

        int renderWidth = settings.availableResolutions[settings.selectedResolutionIndex].width;  // e.g., 1440
        int renderHeight = settings.availableResolutions[settings.selectedResolutionIndex].height; // e.g., 810

        //float scaleX = (float)renderWidth / 480.0f;
        //float scaleY = (float)renderHeight / 270.0f;

        float finalX = (float) (windowWidth - renderWidth) * 0.5f;
        float finalY = (float) (windowHeight - renderHeight) * 0.5f;


        SetMouseOffset((int) -finalX, (int) -finalY);
        float mouseScaleX = gameScreenWidthF / (float) renderWidth;
        float mouseScaleY = gameScreenHeightF / (float) renderHeight;
        SetMouseScale(mouseScaleX, mouseScaleY);

        // handle input
        HandleInputGameMode();
        float dt = GetFrameTime();

        // Update
        UpdateSoundEffects(dt);
        UpdateGameMode(dt);


        PreRenderGameMode();

        BeginTextureMode(target);
        RenderGameMode();
        if(game.settingsData.showFPS) {
            DrawTextEx(font2, TextFormat("FPS: %i", GetFPS()), (Vector2) {1, 1}, 5, 1, GREEN);
            DrawTextEx(font2, TextFormat("ScreenWidth: %i", GetScreenWidth()), (Vector2) {1, 10}, 5, 1, YELLOW);
            DrawTextEx(font2, TextFormat("ScreenHeight: %i", GetScreenHeight()), (Vector2) {1, 16}, 5, 1, YELLOW);
            DrawTextEx(font2, TextFormat("Mouse: %i,%i", GetMouseX(), GetMouseY()), (Vector2) {1, 22}, 5, 1, YELLOW);
            //DrawTextEx(font2, TextFormat("MouseOff: %f,%f", finalX, finalY), (Vector2) {1, 28}, 5, 1, YELLOW);
            //DrawTextEx(font2, TextFormat("MouseScale: %f,%f", mouseScaleX, mouseScaleY), (Vector2) {1, 36}, 5, 1, YELLOW);
        }
        EndTextureMode();

        BeginDrawing();

        ClearBackground(BLACK);     // Clear screen background

        DrawTexturePro(
                target.texture,
                (Rectangle){ 0, 0, gameScreenWidth, -gameScreenHeight },   // Render texture source (Y flipped)
                (Rectangle){ finalX, finalY, (float) renderWidth, (float) renderHeight }, // Upscaled target rectangle
                (Vector2){ 0, 0 },
                0.0f,
                WHITE
        );
        EndDrawing();
        //--------------------------------------------------------------------------------------
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