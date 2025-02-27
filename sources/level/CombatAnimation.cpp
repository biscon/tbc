//
// Created by bison on 24-01-25.
//

#include "CombatAnimation.h"
#include "raymath.h"

void PlayAttackAnimation(CharacterData& charData, Level &level, int attacker, int defender) {
    float attackerX = GetCharacterSpritePosX(charData.sprite[attacker]);
    float defenderX = GetCharacterSpritePosX(charData.sprite[defender]);
    float attackerY = GetCharacterSpritePosY(charData.sprite[attacker]);
    float defenderY = GetCharacterSpritePosY(charData.sprite[defender]);
    Animation attackerAnim{};
    SetupAttackAnimation(attackerAnim, attacker, 0.4f, attackerY, defenderY, attackerX, defenderX);
    level.animations.push_back(attackerAnim);
    switch(charData.orientation[attacker]) {
        case Orientation::Up:
            PlayCharacterSpriteAnimRestart(charData.sprite[attacker], SpriteAnimationType::AttackUp, false);
            break;
        case Orientation::Down:
            PlayCharacterSpriteAnimRestart(charData.sprite[attacker], SpriteAnimationType::AttackDown, false);
            break;
        case Orientation::Left:
            PlayCharacterSpriteAnimRestart(charData.sprite[attacker], SpriteAnimationType::AttackLeft, false);
            break;
        case Orientation::Right:
            PlayCharacterSpriteAnimRestart(charData.sprite[attacker], SpriteAnimationType::AttackRight, false);
            break;
    }
}

void PlayDefendAnimation(CharacterData& charData, Level &level, int attacker, int defender) {
    if(charData.stats[defender].health <= 0) {
        return;
    }
    float defenderX = GetCharacterSpritePosX(charData.sprite[defender]);
    float defenderY = GetCharacterSpritePosY(charData.sprite[defender]);

    Vector2 orientationVector = Vector2Normalize(Vector2Subtract(GetCharacterSpritePos(charData.sprite[defender]), GetCharacterSpritePos(charData.sprite[attacker])));
    // invert vector
    orientationVector = Vector2Scale(orientationVector, 5);
    Vector2 point = Vector2Add(GetCharacterSpritePos(charData.sprite[defender]), orientationVector);

    Animation defenderAnim{};
    SetupAttackAnimation(defenderAnim, defender, 0.30f, defenderY, point.y, defenderX, point.x, 0.20f);
    level.animations.push_back(defenderAnim);
}

void PlayAttackDefendAnimation(CharacterData& charData, Level &level, int attacker, int defender) {
    PlayAttackAnimation(charData, level, attacker, defender);
    PlayDefendAnimation(charData, level, attacker, defender);
}

void PlayEnemyVictoryAnimation(CharacterData& charData, Level &level) {
    for(auto &c : level.allCharacters) {
        // skip dead
        if(charData.stats[c].health <= 0 || charData.faction[c] != CharacterFaction::Enemy) {
            continue;
        }
        PlayCharacterSpriteAnim(charData.sprite[c], SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(charData, anim, c, 10.0f, 12, 80.0f);
        level.animations.push_back(anim);
    }
}

void PlayPlayerVictoryAnimation(CharacterData& charData, Level &level) {
    for(auto &c : level.partyCharacters) {
        // skip dead
        if(charData.stats[c].health <= 0) {
            continue;
        }
        PlayCharacterSpriteAnim(charData.sprite[c], SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(charData, anim, c, 2.0f, 16, 80.0f);
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
