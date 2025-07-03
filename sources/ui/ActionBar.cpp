//
// Created by bison on 01-07-25.
//

#include "ActionBar.h"
#include "character/Character.h"
#include "game/Items.h"
#include "Icons.h"
#include "UI.h"
#include "util/StringUtil.h"

static const float actionBarWidth = 215;
static const float actionBarHeight = 54;
static const Rectangle actionBarRect = {ceilf(gameScreenHalfWidthF - (actionBarWidth/2)), 301, actionBarWidth, actionBarHeight};
static const float padding = 2.0f;
static const float iconWidth = 24.0f;
static const float iconHeight = 24.0f;
static const int dividerX = (int) (actionBarRect.x + padding + (4 * (iconWidth + padding)) + padding);
static const int weaponRightMargin = dividerX + (3 * (iconWidth + padding));

void InitActionBar(GameData &data) {
    // Init action icons
    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        icon.enabled = false;
        icon.selectable = false;
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
        icon.selectable = false;
        icon.region = CreateClickRegion({(float) dividerX + padding + ((float) i * (iconWidth + padding)) + 1, actionBarRect.y + padding + iconHeight + padding, iconWidth, iconHeight});
    }
    data.ui.actionBar.switchWeapons.rect = {weaponRightMargin + padding + 5, actionBarRect.y + padding + 5, 16, 16};
}

static void RenderIcon(GameData& data, ActionBarIcon& icon, bool selected) {
    bool hovered = icon.region.hovered;
    Color color = icon.enabled ? (selected ? YELLOW : (hovered ? WHITE : DARKGRAY)) : ColorAlpha(DARKGRAY, 0.5f);
    Color textColor = icon.enabled ? (selected ? YELLOW : (hovered ? WHITE : GRAY)) : ColorAlpha(DARKGRAY, 0.5f);
    DrawRectangleLinesEx(icon.region.rect, 1.0f, color);
    if(!icon.enabled)
        return;
    if(icon.icon == -1) {
        Vector2 textDims = MeasureTextEx(data.smallFont1, icon.text.c_str(), 5, 1);
        DrawTextEx(data.smallFont1, icon.text.c_str(), {
                floorf((icon.region.rect.x + (icon.region.rect.width / 2)) - (textDims.x / 2)),
                icon.region.rect.y + 10}, 5, 1, textColor);
    } else {
        DrawIcon(data, icon.region.rect.x + 4, icon.region.rect.y + 4, textColor, icon.icon);
    }
    if(icon.apCost != -1) {
        DrawTextEx(data.smallFont1, TextFormat("%d", icon.apCost), {
                icon.region.rect.x + icon.region.rect.width - 6,
                icon.region.rect.y + icon.region.rect.height - 8}, 5, 1, GREEN);
    }
}

// Render action icon grid
static void RenderActionIcons(GameData &data) {
    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        RenderIcon(data, icon, data.ui.actionBar.selectedActionIdx == i);
    }
}

static void RenderModeIcons(GameData &data) {
    for(int i = 0; i < data.ui.actionBar.modeIcons.size(); i++) {
        auto& icon = data.ui.actionBar.modeIcons.at(i);
        RenderIcon(data, icon, data.ui.actionBar.selectedModeIdx == i);
    }
}

static void RenderToolTips(GameData& data) {
    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        if(icon.region.showToolTip && icon.enabled) {
            DrawToolTip(data.smallFont1, 5, 1, icon.tooltip);
        }
    }
    for(int i = 0; i < data.ui.actionBar.modeIcons.size(); i++) {
        auto& icon = data.ui.actionBar.modeIcons.at(i);
        if(icon.region.showToolTip && icon.enabled) {
            DrawToolTip(data.smallFont1, 5, 1, icon.tooltip);
        }
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

    DrawTextEx(data.smallFont1, value.c_str(), {(float) weaponRightMargin - textDims.x, actionBarRect.y + padding + 9}, 5, 1, LIGHTGRAY);

    DrawTextEx(data.smallFont1, "Ammo:", { dividerX + padding + 1, actionBarRect.y + padding + 17}, 5, 1, GRAY);
    if(magSize == 0) {
        value = "N/A";
    } else {
        value = TextFormat("%d/%d", currentAmmo, magSize);
    }

    textDims = MeasureTextEx(data.smallFont1, value.c_str(), 5, 1);
    DrawTextEx(data.smallFont1, value.c_str(), {(float) weaponRightMargin - textDims.x, actionBarRect.y + padding + 17}, 5, 1, LIGHTGRAY);

    DrawIcon(data, weaponRightMargin + padding + 5, actionBarRect.y + padding + 5, data.ui.actionBar.switchWeapons.hovered ? YELLOW : LIGHTGRAY, ICON_WEAPON_SWAP);
    if(data.ui.actionBar.switchWeapons.showToolTip) {
        DrawToolTip(data.smallFont1, 5, 1, "Swap weapons");
    }
}

static void RenderActionPoints(GameData& data) {
    CharacterStats& stats = data.charData.stats[data.ui.selectedCharacter];
    int ap = data.charData.stats[data.ui.selectedCharacter].AP;
    int maxAp = CalculateCharMaxAP(stats);
    Rectangle apRect = { actionBarRect.x, actionBarRect.y-11, (float) maxAp * 10, 10};
    apRect.x = ceilf(actionBarRect.x + (actionBarRect.width/2) - (apRect.width/2));

    int previewAp = data.ui.actionBar.previewApUse;
    //DrawRectangleLinesEx(apRect, 1.0f, WHITE);
    float x = apRect.x + 1;
    for(int i = 0; i < maxAp; i++) {
        if(previewAp > 0) {
            if(previewAp > ap) {
                DrawIcon(data, x, apRect.y + 1, WHITE, ICON_RED_AP);
            } else {
                DrawIcon(data, x, apRect.y + 1, WHITE, i < previewAp ? ICON_RED_AP : ( i < ap ? ICON_GREEN_AP : ICON_GRAY_AP));
            }
        } else {
            DrawIcon(data, x, apRect.y + 1, WHITE, i < ap ? ICON_GREEN_AP : ICON_GRAY_AP);
        }

        x += 10;
    }
}

void RenderActionBarUI(GameData &data) {
    DrawRectangleRec(actionBarRect, Color{15, 15, 15, 200});
    //DrawRectangleRounded(actionBarRect, 0.1f, 4, Color{15, 15, 15, 200});
    DrawRectangleRoundedLinesEx(actionBarRect, 0.1f, 4, 1.0f, DARKGRAY);

    RenderActionIcons(data);
    DrawLine(dividerX, (int) (actionBarRect.y + padding), dividerX, (int) (actionBarRect.y + actionBarRect.height - padding), ColorAlpha(DARKGRAY, 0.5f));
    RenderModeIcons(data);
    RenderWeaponInfo(data);
    RenderActionPoints(data);
    RenderToolTips(data);
}

static void UpdateActionBarActions(GameData& data) {
    auto& icons = data.ui.actionBar.actionIcons;
    icons[0].enabled = true;
    icons[0].selectable = true;
    icons[0].icon = ICON_MOVE;
    icons[0].text = "Move";
    icons[0].tooltip = "Move to a new location.";
    icons[0].action = ActionBarAction::Move;

    icons[1].enabled = true;
    icons[1].selectable = true;
    icons[1].icon = ICON_ATTACK;
    icons[1].text = "Atk";
    icons[1].tooltip = "Attack enemy character if in range.";
    icons[1].action = ActionBarAction::Attack;

    int i = 2;

    WeaponRanged* ranged = GetSelectedRangedTemplate(data, data.ui.selectedCharacter);
    if(ranged) {
        if(ranged->currentAmmo == 0) {
            icons[i].enabled = true;
            icons[i].selectable = false;
            icons[i].icon = ICON_RELOAD;
            icons[i].text = "Rel";
            icons[i].tooltip = "Reload the currently selected weapon.";
            icons[i].action = ActionBarAction::Reload;
            i++;
        }
    }

    i = 7;
    icons[i].enabled = true;
    icons[i].selectable = false;
    icons[i].icon = ICON_END_TURN;
    icons[i].text = "End";
    icons[i].tooltip = "End the characters current turn.";
    icons[i].action = ActionBarAction::EndTurn;
}

static void UpdateModes(GameData& data) {
    for(auto& icon : data.ui.actionBar.modeIcons) {
        icon.enabled = false;
        icon.apCost = -1;
    }
    WeaponTemplate* weaponTpl = GetSelectedWeaponTemplate(data, data.ui.selectedCharacter);
    auto& modeIcons = data.ui.actionBar.modeIcons;
    if(weaponTpl != nullptr) {
        if(weaponTpl->rangeDataId != -1) {
            WeaponRanged &ranged = data.weaponData.rangedData[weaponTpl->rangeDataId];
            for(int i = 0; i < ranged.fireModes.size(); i++) {
                FireMode& fm = ranged.fireModes.at(i);
                modeIcons[i].enabled = true;
                modeIcons[i].icon = fm.icon;
                modeIcons[i].selectable = true;
                modeIcons[i].text = TruncateWithEllipsis(fm.name, 3);
                modeIcons[i].tooltip = TextFormat("%s shot (AP: %d)", fm.name.c_str(), fm.apCost);
                modeIcons[i].apCost = fm.apCost;
            }
        }
    }
}

void UpdateActionBar(GameData &data, float dt) {
    UpdateActionBarActions(data);
    UpdateModes(data);

}

void ExecuteAction(GameData& data, ActionBarAction action, Level& level, PlayField& playField, bool wasSelected) {
    switch(action) {
        case ActionBarAction::Move: {
            if(wasSelected) {
                level.turnState = TurnState::SelectDestination;
                playField.mode = PlayFieldMode::SelectingTile;
            }
            break;
        }
        case ActionBarAction::Reload:
            break;
        case ActionBarAction::EndTurn: {
            level.turnState = TurnState::EndTurn;
            playField.mode = PlayFieldMode::None;
            break;
        }
        default:
            break;
    }
}

bool HandleActionBarInput(GameData &data, Level& level, PlayField& playField) {
    Vector2 mouse = GetMousePosition();
    data.ui.actionBar.switchWeapons.Update(mouse);

    for(int i = 0; i < data.ui.actionBar.actionIcons.size(); i++) {
        auto& icon = data.ui.actionBar.actionIcons.at(i);
        icon.region.Update(mouse);
        if(icon.region.ConsumeClick()) {
            bool wasSelected = false;
            if(icon.selectable) {
                if(data.ui.actionBar.selectedActionIdx != i) {
                    data.ui.actionBar.selectedActionIdx = i;
                    wasSelected = true;
                }
            }
            ExecuteAction(data, icon.action, level, playField, wasSelected);
        }
    }
    for(int i = 0; i < data.ui.actionBar.modeIcons.size(); i++) {
        auto& icon = data.ui.actionBar.modeIcons.at(i);
        icon.region.Update(mouse);
        if(icon.region.ConsumeClick()) {
            if(icon.selectable) {
                data.ui.actionBar.selectedModeIdx = i;
            }
        }
    }
    data.ui.actionBar.hovered = false;
    if (!CheckCollisionPointRec(mouse, actionBarRect)) return false;
    data.ui.actionBar.hovered = true;
    int charId = data.ui.selectedCharacter;

    if(data.ui.actionBar.switchWeapons.ConsumeClick()) {
        SwapWeapons(data, charId);
        data.ui.actionBar.selectedModeIdx = 0;
    }
    return true;
}
