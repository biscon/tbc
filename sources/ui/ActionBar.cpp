//
// Created by bison on 01-07-25.
//

#include "ActionBar.h"
#include "character/Character.h"
#include "game/Items.h"
#include "Icons.h"

static const Rectangle actionBarRect = {187, 295, 215, 54};
static const float padding = 2.0f;
static const float iconWidth = 24.0f;
static const float iconHeight = 24.0f;
static const int dividerX = (int) (actionBarRect.x + padding + (4 * (iconWidth + padding)) + padding);

void InitActionBar(GameData &data) {
    // Init action icons
    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        icon.enabled = false;
        if(i < 4) { // upper row
            icon.region = CreateClickRegion({actionBarRect.x + padding + ((float) i * (iconWidth + padding)), actionBarRect.y + padding, iconWidth, iconHeight});
        } else {    // lower row
            icon.region = CreateClickRegion({actionBarRect.x + padding + ((float) (i-4) * (iconWidth + padding)), actionBarRect.y + padding + iconHeight + padding, iconWidth, iconHeight});
        }
    }
    // Init mode icons
    for(int i = 0; i < data.ui.actionBar.modeIcons.size(); i++) {
        auto& icon = data.ui.actionBar.modeIcons.at(i);
        icon.enabled = false;
        icon.region = CreateClickRegion({(float) dividerX + padding + ((float) i * (iconWidth + padding)) + 1, actionBarRect.y + padding + iconHeight + padding, iconWidth, iconHeight});
    }
}

// Render action icon grid
static void RenderActionIcons(GameData &data) {
    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        DrawRectangleLinesEx(icon.region.rect, 1.0f, icon.enabled ? DARKGRAY : ColorAlpha(DARKGRAY, 0.5f));
    }
}

static void RenderModeIcons(GameData &data) {
    for(int i = 0; i < data.ui.actionBar.modeIcons.size(); i++) {
        auto& icon = data.ui.actionBar.modeIcons.at(i);
        DrawRectangleLinesEx(icon.region.rect, 1.0f, icon.enabled ? DARKGRAY : ColorAlpha(DARKGRAY, 0.5f));
    }
}

static void RenderWeaponInfo(GameData& data) {
    int selectedSlot = data.charData.selectedWeaponSlot[data.ui.selectedCharacter];
    int itemId = GetEquippedItem(data, data.ui.selectedCharacter, static_cast<ItemEquipSlot>(selectedSlot));
    std::string itemName = "Unarmed";
    int baseDmg = 0;
    int currentAmmo = 0;
    int magSize = 0;
    if(itemId != -1) {
        ItemTemplate& tmpl = data.itemData.templateData[GetItemTemplateId(data, itemId)];
        itemName = tmpl.name;
        int weaponTplId = GetItemTypeTemplateId(data, itemId);
        WeaponTemplate& weaponTpl = data.weaponData.templateData[weaponTplId];
        baseDmg = weaponTpl.baseDamage;
        if(weaponTpl.rangeDataId != -1) {
            WeaponRanged& weaponRanged = data.weaponData.rangedData[weaponTpl.rangeDataId];
            currentAmmo = weaponRanged.currentAmmo;
            magSize = weaponRanged.magazineSize;
        }
    }
    DrawTextEx(data.smallFont1, itemName.c_str(), { dividerX + padding + 1, actionBarRect.y + padding + 1}, 5, 1, WHITE);

    DrawTextEx(data.smallFont1, "Damage:", { dividerX + padding + 1, actionBarRect.y + padding + 9}, 5, 1, GRAY);
    std::string value = TextFormat("%d", baseDmg);
    Vector2 textDims = MeasureTextEx(data.smallFont1, value.c_str(), 5, 1);
    int valueX = dividerX + (3 * (iconWidth + padding));
    DrawTextEx(data.smallFont1, value.c_str(), { (float) valueX - textDims.x, actionBarRect.y + padding + 9}, 5, 1, LIGHTGRAY);

    DrawTextEx(data.smallFont1, "Ammo:", { dividerX + padding + 1, actionBarRect.y + padding + 17}, 5, 1, GRAY);
    if(magSize == 0) {
        value = "N/A";
    } else {
        value = TextFormat("%d/%d", currentAmmo, magSize);
    }

    textDims = MeasureTextEx(data.smallFont1, value.c_str(), 5, 1);
    DrawTextEx(data.smallFont1, value.c_str(), { (float) valueX - textDims.x, actionBarRect.y + padding + 17}, 5, 1, LIGHTGRAY);

    DrawIcon(data, valueX + padding + 5, actionBarRect.y + padding + 5, LIGHTGRAY, ICON_WEAPON_SWAP);

}

void RenderActionBarUI(GameData &data) {
    DrawRectangleRec(actionBarRect, Color{15, 15, 15, 200});
    DrawRectangleRoundedLinesEx(actionBarRect, 0.1f, 4, 1.0f, DARKGRAY);

    RenderActionIcons(data);
    DrawLine(dividerX, (int) (actionBarRect.y + padding), dividerX, (int) (actionBarRect.y + actionBarRect.height - padding), DARKGRAY);
    RenderModeIcons(data);
    RenderWeaponInfo(data);
}

void UpdateActionBar(GameData &data, float dt) {

}

bool HandleActionBarInput(GameData &data) {
    return false;
}
