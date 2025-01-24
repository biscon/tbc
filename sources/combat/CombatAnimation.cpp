//
// Created by bison on 24-01-25.
//

#include "CombatAnimation.h"
#include "raymath.h"

void PlayAttackAnimation(CombatState &combat, Character &attacker, Character &defender) {
    float attackerX = attacker.sprite.player.position.x;
    float defenderX = defender.sprite.player.position.x;
    float attackerY = attacker.sprite.player.position.y;
    float defenderY = defender.sprite.player.position.y;
    Animation attackerAnim{};
    SetupAttackAnimation(attackerAnim, &attacker, 0.4f, attackerY, defenderY, attackerX, defenderX);
    combat.animations.push_back(attackerAnim);
}

void PlayDefendAnimation(CombatState &combat, Character &attacker, Character &defender) {
    if(defender.health <= 0) {
        return;
    }
    float defenderX = defender.sprite.player.position.x;
    float defenderY = defender.sprite.player.position.y;
    Vector2 orientationVector = Vector2Normalize(Vector2Subtract(defender.sprite.player.position, attacker.sprite.player.position));
    // invert vector
    orientationVector = Vector2Scale(orientationVector, 5);
    Vector2 point = Vector2Add(defender.sprite.player.position, orientationVector);

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
        PlaySpriteAnimation(c->sprite.player, GetCharacterAnimation(c->sprite, SpriteAnimationType::WalkDown), true);
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
        PlaySpriteAnimation(c->sprite.player, GetCharacterAnimation(c->sprite, SpriteAnimationType::WalkDown), true);
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
