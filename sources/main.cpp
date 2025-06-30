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
#include "ui/Dialogue.h"
#include "game/Quest.h"
#include "game/Settings.h"
#include "game/Items.h"

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
    game.levelTarget = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(game.levelTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use

    game.uiTarget = LoadRenderTexture(gameScreenWidth, gameScreenHeight);
    SetTextureFilter(game.uiTarget.texture, TEXTURE_FILTER_POINT);  // Texture scale filter to use

    //Font font = LoadFont(ASSETS_PATH"small.fnt");

    game.smallFont1 = LoadFontEx(ASSETS_PATH"pixel-3x5.ttf", 5, nullptr, 0);

    //--------------------------------------------------------------------------------------

    // Create AI
    CreateFighterAi("Fighter");

    InitDialogueData(game.dialogueData, ASSETS_PATH"dialogue.json");
    InitQuestData(game, ASSETS_PATH"quests.json");
    InitSpriteAnimationData(game.spriteData, ASSETS_PATH"animations.json");
    InitWeaponData(game.weaponData, ASSETS_PATH"weapons.json");
    InitItemData(game, ASSETS_PATH"items.json");

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

        // Render level to level target
        BeginTextureMode(game.levelTarget);
            RenderLevelGameMode();
        EndTextureMode();

        // Render to UI target
        BeginTextureMode(game.uiTarget);
            ClearBackground(BLANK);
            RenderUiGameMode();
            if(game.settingsData.showFPS) {
                DrawTextEx(game.smallFont1, TextFormat("FPS: %i", GetFPS()), (Vector2) {1, 1}, 5, 1, GREEN);
                DrawTextEx(game.smallFont1, TextFormat("ScreenWidth: %i", GetScreenWidth()), (Vector2) {1, 10}, 5, 1, YELLOW);
                DrawTextEx(game.smallFont1, TextFormat("ScreenHeight: %i", GetScreenHeight()), (Vector2) {1, 16}, 5, 1, YELLOW);
                DrawTextEx(game.smallFont1, TextFormat("Mouse: %i,%i", GetMouseX(), GetMouseY()), (Vector2) {1, 22}, 5, 1, YELLOW);
                //DrawTextEx(font2, TextFormat("MouseOff: %f,%f", finalX, finalY), (Vector2) {1, 28}, 5, 1, YELLOW);
                //DrawTextEx(font2, TextFormat("MouseScale: %f,%f", mouseScaleX, mouseScaleY), (Vector2) {1, 36}, 5, 1, YELLOW);
            }
        EndTextureMode();

        BeginDrawing();
            ClearBackground(BLACK);     // Clear screen background
            DrawTexturePro(
                    game.levelTarget.texture,
                    (Rectangle){ 0, 0, gameScreenWidth, -gameScreenHeight },   // Render texture source (Y flipped)
                    (Rectangle){ finalX, finalY, (float) renderWidth, (float) renderHeight }, // Upscaled target rectangle
                    (Vector2){ 0, 0 },
                    0.0f,
                    WHITE
            );

            DrawTexturePro(
                    game.uiTarget.texture,
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
    UnloadRenderTexture(game.levelTarget);
    UnloadRenderTexture(game.uiTarget);

    //UnloadFont(font);                   // Unload custom font
    UnloadFont(game.smallFont1);                   // Unload custom font

    DestroySpriteAnimationData(game.spriteData);
    DestroySoundEffectManager();
    CloseAudioDevice();     // Close audio device

    CloseWindow();                      // Close window and OpenGL context
    //--------------------------------------------------------------------------------------
    DestroyGame(game);
    return 0;
}