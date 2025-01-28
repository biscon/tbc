//
// Created by bison on 24-01-25.
//

#include "CombatAnimation.h"
#include "raymath.h"

void PlayAttackAnimation(CombatState &combat, Character &attacker, Character &defender) {
    float attackerX = GetCharacterSpritePosX(attacker.sprite);
    float defenderX = GetCharacterSpritePosX(defender.sprite);
    float attackerY = GetCharacterSpritePosY(attacker.sprite);
    float defenderY = GetCharacterSpritePosY(defender.sprite);
    Animation attackerAnim{};
    SetupAttackAnimation(attackerAnim, &attacker, 0.4f, attackerY, defenderY, attackerX, defenderX);
    combat.animations.push_back(attackerAnim);
    switch(attacker.orientation) {
        case Orientation::Up:
            PlayCharacterSpriteAnimRestart(attacker.sprite, SpriteAnimationType::AttackUp, false);
            break;
        case Orientation::Down:
            PlayCharacterSpriteAnimRestart(attacker.sprite, SpriteAnimationType::AttackDown, false);
            break;
        case Orientation::Left:
            PlayCharacterSpriteAnimRestart(attacker.sprite, SpriteAnimationType::AttackLeft, false);
            break;
        case Orientation::Right:
            PlayCharacterSpriteAnimRestart(attacker.sprite, SpriteAnimationType::AttackRight, false);
            break;
    }
}

void PlayDefendAnimation(CombatState &combat, Character &attacker, Character &defender) {
    if(defender.health <= 0) {
        return;
    }
    float defenderX = GetCharacterSpritePosX(defender.sprite);
    float defenderY = GetCharacterSpritePosY(defender.sprite);

    Vector2 orientationVector = Vector2Normalize(Vector2Subtract(GetCharacterSpritePos(defender.sprite), GetCharacterSpritePos(attacker.sprite)));
    // invert vector
    orientationVector = Vector2Scale(orientationVector, 5);
    Vector2 point = Vector2Add(GetCharacterSpritePos(defender.sprite), orientationVector);

    Animation defenderAnim{};
    SetupAttackAnimation(defenderAnim, &defender, 0.30f, defenderY, point.y, defenderX, point.x, 0.20f);
    combat.animations.push_back(defenderAnim);
}

void PlayAttackDefendAnimation(CombatState &combat, Character &attacker, Character &defender) {
    PlayAttackAnimation(combat, attacker, defender);
    PlayDefendAnimation(combat, attacker, defender);
}

void PlayEnemyVictoryAnimation(CombatState &combat) {
    for(auto &c : combat.enemyCharacters) {
        // skip dead
        if(c->health <= 0) {
            continue;
        }
        PlayCharacterSpriteAnim(c->sprite, SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(anim, c, 10.0f, 12, 80.0f);
        combat.animations.push_back(anim);
    }
}

void PlayPlayerVictoryAnimation(CombatState &combat) {
    for(auto &c : combat.playerCharacters) {
        // skip dead
        if(c->health <= 0) {
            continue;
        }
        PlayCharacterSpriteAnim(c->sprite, SpriteAnimationType::WalkDown, true);
        Animation anim{};
        SetupVictoryAnimation(anim, c, 10.0f, 16, 80.0f);
        combat.animations.push_back(anim);
    }
}

void RemoveAttackAnimations(CombatState &combat) {
    for(auto it = combat.animations.begin(); it != combat.animations.end(); ) {
        if(it->type == AnimationType::Attack) {
            it = combat.animations.erase(it);
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
