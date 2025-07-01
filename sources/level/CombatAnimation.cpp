//
// Created by bison on 24-01-25.
//

#include "CombatAnimation.h"
#include "raymath.h"
#include "graphics/Animation.h"
#include "graphics/CharacterSprite.h"

void PlayAttackAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender) {
    float attackerX = GetCharacterSpritePosX(spriteData, charData.sprite[attacker]);
    float defenderX = GetCharacterSpritePosX(spriteData, charData.sprite[defender]);
    float attackerY = GetCharacterSpritePosY(spriteData, charData.sprite[attacker]);
    float defenderY = GetCharacterSpritePosY(spriteData, charData.sprite[defender]);
    Animation attackerAnim{};
    SetupAttackAnimation(attackerAnim, attacker, 0.4f, attackerY, defenderY, attackerX, defenderX);
    level.animations.push_back(attackerAnim);
    switch(charData.orientation[attacker]) {
        case Orientation::Up:
            PlayCharacterSpriteAnimRestart(spriteData, charData.sprite[attacker], SpriteAnimationType::AttackUp, false);
            break;
        case Orientation::Down:
            PlayCharacterSpriteAnimRestart(spriteData, charData.sprite[attacker], SpriteAnimationType::AttackDown, false);
            break;
        case Orientation::Left:
            PlayCharacterSpriteAnimRestart(spriteData, charData.sprite[attacker], SpriteAnimationType::AttackLeft, false);
            break;
        case Orientation::Right:
            PlayCharacterSpriteAnimRestart(spriteData, charData.sprite[attacker], SpriteAnimationType::AttackRight, false);
            break;
    }
}

void PlayDefendAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender) {
    if(charData.stats[defender].HP <= 0) {
        return;
    }
    float defenderX = GetCharacterSpritePosX(spriteData, charData.sprite[defender]);
    float defenderY = GetCharacterSpritePosY(spriteData, charData.sprite[defender]);

    Vector2 orientationVector = Vector2Normalize(Vector2Subtract(GetCharacterSpritePos(spriteData, charData.sprite[defender]), GetCharacterSpritePos(spriteData, charData.sprite[attacker])));
    // invert vector
    orientationVector = Vector2Scale(orientationVector, 5);
    Vector2 point = Vector2Add(GetCharacterSpritePos(spriteData, charData.sprite[defender]), orientationVector);

    Animation defenderAnim{};
    SetupAttackAnimation(defenderAnim, defender, 0.30f, defenderY, point.y, defenderX, point.x, 0.20f);
    level.animations.push_back(defenderAnim);
}

void PlayAttackDefendAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender) {
    PlayAttackAnimation(spriteData, charData, level, attacker, defender);
    PlayDefendAnimation(spriteData, charData, level, attacker, defender);
}

void PlayEnemyVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Level &level) {
    for(auto &c : level.allCharacters) {
        // skip dead
        if(charData.stats[c].HP <= 0 || charData.faction[c] != CharacterFaction::Enemy) {
            continue;
        }
        PlayCharacterSpriteAnim(spriteData, charData.sprite[c], SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(spriteData, charData, anim, c, 10.0f, 12, 80.0f);
        level.animations.push_back(anim);
    }
}

void PlayPlayerVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Level &level) {
    for(auto &c : level.partyCharacters) {
        // skip dead
        if(charData.stats[c].HP <= 0) {
            continue;
        }
        PlayCharacterSpriteAnim(spriteData, charData.sprite[c], SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(spriteData, charData, anim, c, 2.0f, 16, 80.0f);
        level.animations.push_back(anim);
    }
}

void RemoveAttackAnimations(Level &level) {
    for(auto it = level.animations.begin(); it != level.animations.end(); ) {
        if(it->type == AnimationType::Attack) {
            it = level.animations.erase(it);
        } else {
            ++it;
        }
    }
}

int GetBloodIntensity(int dmg, int attackerAttack) {
    // Define dynamic thresholds based on attacker's attack stat
    int lowThreshold = attackerAttack / 2;     // Low baseAttack threshold (e.g., half of attack)
    int highThreshold = attackerAttack * 3 / 2; // High baseAttack threshold (e.g., 1.5x attack)

    // Clamp baseAttack within the calculated range
    dmg = Clamp(dmg, lowThreshold, highThreshold);

    // Scale blood intensity between 10 and 50
    if (dmg <= attackerAttack) {
        // Interpolate from 10 to 30 (low to medium baseAttack)
        float t = (float)(dmg - lowThreshold) / (attackerAttack - lowThreshold);
        return (int)Lerp(10, 30, t);
    } else {
        // Interpolate from 30 to 50 (medium to high baseAttack)
        float t = (float)(dmg - attackerAttack) / (highThreshold - attackerAttack);
        return (int)Lerp(30, 50, t);
    }
}
