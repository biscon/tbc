#include "MenuGameMode.h"
#include "raylib.h"
#include <memory>
#include <vector>
#include <string>
#include <functional>
#include <stack>
#include "data/SaveData.h"
#include "game/Settings.h"
#include "ai/PathFinding.h"

static GameData* game;

const Color MENU_BG_COLOR = Color{25, 25, 25, 255};

// Forward declaration
struct Menu;

using MenuBuilder = std::function<std::shared_ptr<Menu>()>;

struct MenuItem {
    std::string text;
    bool isSubmenu = false;
    std::function<void()> action;             // only used if !isSubmenu
    MenuBuilder submenuBuilder = nullptr;
    Color color = LIGHTGRAY;
    bool enabled = true;
};

struct Menu {
    std::string title;
    std::string hint;
    std::vector<MenuItem> items;
    int selected = 0;
};


//static std::stack<std::shared_ptr<Menu>> menuStack;
static std::stack<std::function<std::shared_ptr<Menu>()>> menuStack;


static void StartNewGame() {
    int id = CreateCharacter(game->charData, CharacterClass::Warrior, CharacterFaction::Player, "Player1", "Fighter");
    AssignSkill(game->charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
    AssignSkill(game->charData.skills[id], SkillType::Stun, "Stunning Blow", 1, false, false, 0, 3, 1);
    InitCharacterSprite(game->spriteData, game->charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(game->spriteData, game->weaponData, game->charData, id, "Sword");
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    game->party.emplace_back(id);

    id = CreateCharacter(game->charData, CharacterClass::Mage, CharacterFaction::Player, "Player2", "Fighter");
    AssignSkill(game->charData.skills[id], SkillType::Dodge, "Dodge", 1, true, true, 0, 0, 0);
    AssignSkill(game->charData.skills[id], SkillType::FlameJet, "Burning Hands", 1, false, false, 0, 3, 5);
    InitCharacterSprite(game->spriteData, game->charData.sprite[id], "MaleWarrior", true);
    GiveWeapon(game->spriteData, game->weaponData, game->charData, id, "Bow");
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    LevelUp(game->charData, id, true);
    game->party.emplace_back(id);

    game->state = GameState::LOAD_LEVEL;
    PushGameMode(GameModes::Level);
}

static void loadGame() {
    SaveData saveData;
    if(!LoadGameData(saveData, "savegame.json")) {
        TraceLog(LOG_DEBUG, "No existing savegame found");
        return;
    }
    game->levelFileName = saveData.currentLevel;
    game->state = GameState::LOAD_LEVEL_FROM_SAVE;
    game->levelState = saveData.levels;

    ClearAllCharacters(game->charData);
    game->spriteData.player.animationIdx.clear();
    game->spriteData.player.animData.clear();
    game->spriteData.player.renderData.clear();
    game->party.clear();
    for(auto& ch : saveData.party) {
        int id = CreateCharacter(game->charData, ch.characterClass, ch.faction, ch.name, ch.ai);
        //AssignSkill(game->charData.skills[id], SkillType::Taunt, "Howling Scream", 1, false, true, 0, 3, 0);
        InitCharacterSprite(game->spriteData, game->charData.sprite[id], ch.spriteTemplate, true);
        GiveWeapon(game->spriteData, game->weaponData, game->charData, id, ch.weaponTemplate);
        game->charData.stats[id] = ch.stats;
        Vector2i savedPos = { ch.tilePosX, ch.tilePosY};
        SetCharacterGridPosI(game->spriteData, game->charData.sprite[id], savedPos);
        /*
        LevelUp(game->charData, id, true);
        LevelUp(game->charData, id, true);
        LevelUp(game->charData, id, true);
        LevelUp(game->charData, id, true);
         */
        game->party.emplace_back(id);
    }


    PushGameMode(GameModes::Level);
}

static void saveGame() {
    SaveData saveData;
    saveData.currentLevel = game->levelFileName;
    saveData.levels = game->levelState;
    saveData.quests = game->quests;

    for(auto& id : game->party) {
        PartyCharacter pc;
        pc.name = game->charData.name[id];
        pc.faction = game->charData.faction[id];
        pc.characterClass = game->charData.characterClass[id];
        pc.ai = game->charData.ai[id];
        pc.stats = game->charData.stats[id];
        // save sprite template
        pc.spriteTemplate = game->charData.sprite[id].spriteTemplate;

        // save weapon template
        auto weaponId = game->charData.weaponIdx[id];
        auto weaponTplId = game->weaponData.instanceData.weaponTemplateIdx[weaponId];
        pc.weaponTemplate = game->weaponData.templateData.name[weaponTplId];
        // save position
        Vector2i pos = GetCharacterGridPosI(game->spriteData, game->charData.sprite[id]);
        pc.tilePosX = pos.x;
        pc.tilePosY = pos.y;

        saveData.party.push_back(pc);
    }

    SaveGameData(saveData, "savegame.json");
}

static std::shared_ptr<Menu> createResolutionMenu() {
    // Resolution submenu
    auto resolutionMenu = std::make_shared<Menu>();
    resolutionMenu->title = "Resolution";

    for (size_t i = 0; i < game->settingsData.availableResolutions.size(); ++i) {
        auto& availRes = game->settingsData.availableResolutions[i];
        MenuItem res;
        bool selected = (int)i == game->settingsData.selectedResolutionIndex;
        res.text = (selected ? "< " : "  ") +
                   std::to_string(availRes.width) + " x " + std::to_string(availRes.height) + (selected ? " > " : "");

        res.color = selected ? WHITE : LIGHTGRAY;
        res.isSubmenu = false;
        res.action = [i] {
            SettingsData& settings = game->settingsData;
            settings.selectedResolutionIndex = (int) i;
            settings.needsApply = true;
            ApplySettings(settings);
            SaveSettings(settings);
        };
        resolutionMenu->items.push_back(res);
    }

    MenuItem back;
    back.text = "Back";
    back.isSubmenu = false;
    back.action = [] {
        if (!menuStack.empty()) menuStack.pop();
    };
    resolutionMenu->items.push_back(back);
    return resolutionMenu;
}

static std::shared_ptr<Menu> createDisplayModeMenu() {
    // Resolution submenu
    auto menu = std::make_shared<Menu>();
    menu->title = "Display Mode";
    menu->hint = "Fullscreen is buggy AF, consider yourself warned.";
    MenuItem i1;
    i1.text = game->settingsData.displayMode == DisplayMode::Windowed ? "< Windowed >" : "Windowed";
    i1.color = game->settingsData.displayMode == DisplayMode::Windowed ? WHITE : LIGHTGRAY;
    i1.isSubmenu = false;
    i1.action = [] {
        game->settingsData.displayMode = DisplayMode::Windowed;
        game->settingsData.needsApply = true;
        ApplySettings(game->settingsData);
        SaveSettings(game->settingsData);
    };
    menu->items.push_back(i1);

    MenuItem i2;
    i2.text = game->settingsData.displayMode == DisplayMode::Fullscreen ? "< Fullscreen >" : "Fullscreen";
    i2.color = game->settingsData.displayMode == DisplayMode::Fullscreen ? WHITE : LIGHTGRAY;
    i2.isSubmenu = false;
    i2.action = [] {
        game->settingsData.displayMode = DisplayMode::Fullscreen;
        game->settingsData.needsApply = true;
        ApplySettings(game->settingsData);
        SaveSettings(game->settingsData);
    };
    i2.enabled = false;
    menu->items.push_back(i2);

    MenuItem i3;
    i3.text = game->settingsData.displayMode == DisplayMode::Borderless ? "< Borderless >" : "Borderless";
    i3.color = game->settingsData.displayMode == DisplayMode::Borderless ? WHITE : LIGHTGRAY;
    i3.isSubmenu = false;
    i3.action = [] {
        game->settingsData.displayMode = DisplayMode::Borderless;
        game->settingsData.needsApply = true;
        ApplySettings(game->settingsData);
        SaveSettings(game->settingsData);
    };
    menu->items.push_back(i3);

    MenuItem back;
    back.text = "Back";
    back.isSubmenu = false;
    back.action = [] {
        if (!menuStack.empty()) menuStack.pop();
    };
    menu->items.push_back(back);
    return menu;
}

static std::shared_ptr<Menu> createDebugMenu() {
    auto debugMenu = std::make_shared<Menu>();
    debugMenu->title = "Debug Options";

    MenuItem toggleFPS;
    if(game->settingsData.showFPS) {
        toggleFPS.text = "Disable FPS";
    } else {
        toggleFPS.text = "Enable FPS";
    }
    toggleFPS.isSubmenu = false;
    toggleFPS.action = [] {
        game->settingsData.showFPS = !game->settingsData.showFPS;
        SaveSettings(game->settingsData);
    };
    debugMenu->items.push_back(toggleFPS);

    MenuItem back;
    back.text = "Back";
    back.isSubmenu = false;
    back.action = [] {
        if (!menuStack.empty()) menuStack.pop();
    };
    debugMenu->items.push_back(back);
    return debugMenu;
}

static std::shared_ptr<Menu> createSettingsMenu() {
    // Settings submenu
    auto settingsMenu = std::make_shared<Menu>();
    settingsMenu->title = "Settings";
    MenuItem resolution;
    resolution.text = "Resolution";
    resolution.isSubmenu = true;
    resolution.submenuBuilder = createResolutionMenu;
    settingsMenu->items.push_back(resolution);

    MenuItem displayMode;
    displayMode.text = "Display Mode";
    displayMode.isSubmenu = true;
    displayMode.submenuBuilder = createDisplayModeMenu;
    settingsMenu->items.push_back(displayMode);

    MenuItem debugOptions;
    debugOptions.text = "Debug Options";
    debugOptions.isSubmenu = true;
    debugOptions.submenuBuilder = createDebugMenu;
    settingsMenu->items.push_back(debugOptions);


    MenuItem toggleFPSLock;
    if(game->settingsData.fpsLock) {
        toggleFPSLock.text = "Unlock FPS";
    } else {
        toggleFPSLock.text = "Lock FPS (60)";
    }
    toggleFPSLock.isSubmenu = false;
    toggleFPSLock.action = [] {
        game->settingsData.fpsLock = !game->settingsData.fpsLock;
        ApplySettings(game->settingsData);
        SaveSettings(game->settingsData);
    };
    settingsMenu->items.push_back(toggleFPSLock);


    MenuItem back;
    back.text = "Back";
    back.isSubmenu = false;
    back.action = [] {
        if (!menuStack.empty()) menuStack.pop();
    };
    settingsMenu->items.push_back(back);
    return settingsMenu;
}

static std::shared_ptr<Menu> createMainMenu() {
    auto menu = std::make_shared<Menu>();
    menu->title = "Main Menu";

    if (game->state == GameState::START_NEW_GAME) {
        MenuItem item;
        item.text = "Start New Game";
        item.isSubmenu = false;
        item.action = StartNewGame;
        menu->items.push_back(item);
    } else {
        MenuItem resume;
        resume.text = "Resume";
        resume.isSubmenu = false;
        resume.action = [] { PushGameMode(GameModes::Level); };
        menu->items.push_back(resume);

        MenuItem save;
        save.text = "Save Game";
        save.isSubmenu = false;
        save.action = saveGame;
        menu->items.push_back(save);
    }

    MenuItem load;
    load.text = "Load Game";
    load.isSubmenu = false;
    load.action = loadGame;
    menu->items.push_back(load);


    // Main Menu Settings item
    MenuItem settings;
    settings.text = "Settings";
    settings.isSubmenu = true;
    settings.submenuBuilder = createSettingsMenu;
    menu->items.push_back(settings);

    // Main Menu Quit item
    MenuItem quit;
    quit.text = "Quit";
    quit.isSubmenu = false;
    quit.action = [] {
        PopGameMode();
    };
    menu->items.push_back(quit);

    return menu;
}

void MenuInit() {
    menuStack = std::stack<std::function<std::shared_ptr<Menu>()>>();
    menuStack.push(&createMainMenu);
}

void MenuUpdate(float dt) {
}

void MenuRenderUi() {
    ClearBackground(MENU_BG_COLOR);
    if (menuStack.empty()) return;

    //auto& menu = *menuStack.top();
    std::shared_ptr<Menu> menu = menuStack.top()(); // call the builder
    float menuX = gameScreenWidthF / 2.0f;
    float menuY = gameScreenHeightF / 2.0f;
    float spacing = 20.0f;
    float itemWidth = 200;
    float itemHeight = 18;

    if(!menu->title.empty()) {
        DrawText(menu->title.c_str(), menuX - (MeasureText(menu->title.c_str(), 20) / 2), 16, 20, WHITE);
    }

    if(!menu->hint.empty()) {
        DrawText(menu->hint.c_str(), menuX - (MeasureText(menu->hint.c_str(), 10) / 2), 255, 10, LIGHTGRAY);
    }

    for (int i = 0; i < (int)menu->items.size(); ++i) {
        bool enabled = menu->items[i].enabled;
        float x = menuX - itemWidth / 2;
        float y = menuY + (i - menu->items.size() / 2.0f) * spacing;
        Rectangle rect = {x, y, itemWidth, itemHeight};

        bool hovered = enabled && CheckCollisionPointRec(GetMousePosition(), rect);
        bool clicked = hovered && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

        DrawRectangleRec(rect, hovered ? Fade(WHITE, 0.1f) : Fade(WHITE, 0.05f));
        DrawRectangleLinesEx(rect, 1.0f, hovered ? YELLOW : DARKGRAY);
        if(enabled) {
            DrawText(menu->items[i].text.c_str(), (int) (x + 10), (int) (y + 4), 10,
                     hovered ? YELLOW : menu->items[i].color);
        } else {
            DrawText(menu->items[i].text.c_str(), (int) (x + 10), (int) (y + 4), 10,DARKGRAY);
        }

        if (clicked) {
            auto& item = menu->items[i];
            if (item.isSubmenu && item.submenuBuilder) {
                menuStack.push(item.submenuBuilder);
            } else if (item.action) {
                item.action();
            }
        }
    }
}

void MenuRenderLevel() {

}

void MenuHandleInput() {
    if (IsKeyPressed(KEY_ESCAPE)) {
        if (menuStack.size() > 1) {
            menuStack.pop();
        } else {
            PopGameMode();
        }
    }
}

void MenuPreRender() { }
void MenuDestroy() { }
void MenuPause() { }
void MenuResume() { }

void SetupMenuGameMode(GameData* gameState) {
    game = gameState;
    CreateGameMode(GameModes::Menu, MenuInit, MenuUpdate, MenuHandleInput, MenuRenderLevel, MenuRenderUi, MenuPreRender, MenuDestroy, MenuPause, MenuResume);
}
