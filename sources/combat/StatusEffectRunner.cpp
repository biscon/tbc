//
// Created by bison on 23-01-25.
//

#include <algorithm>
#include "StatusEffectRunner.h"

static void ApplyStatusEffect(CombatState &combat, GridState &gridState, Character& character, StatusEffect& effect) {
    Vector2 charPos = GetCharacterSpritePos(character.sprite);

    switch(effect.type) {
        case StatusEffectType::DamageReduction:
            break;
        case StatusEffectType::ThreatModifier:
            break;
        case StatusEffectType::Stun:
            break;
        case StatusEffectType::Poison:
            break;
        case StatusEffectType::Regeneration:
            break;
        case StatusEffectType::Bleed:
            break;
        case StatusEffectType::Confusion:
            break;
        case StatusEffectType::Fear:
            break;
        case StatusEffectType::Burning: {
            // Burning effect
            int damage = DealDamageStatusEffect(combat, character, (int) effect.value);
            if(damage > 0) {
                CreateExplosionEffect(*gridState.particleManager, {charPos.x, charPos.y}, 5, 10.0f, 0.2f);
            }
            break;
        }
        default:
            break;
    }
    // check if dead
    if(character.health <= 0) {
        KillCharacter(combat, character);
    }
}

// runs when a round is started
void ApplyStatusEffects(CombatState &combat, GridState &gridState) {
    for(auto& character : combat.turnOrder) {
        // skip dead characters
        if(character->health <= 0) {
            continue;
        }
        for(auto& effect : character->statusEffects) {
            ApplyStatusEffect(combat, gridState, *character, effect);
        }
    }
}

// runs when a round is over
void UpdateStatusEffects(CombatState &combat) {
    for(auto &character : combat.turnOrder) {
        for(auto &effect : character->statusEffects) {
            if(effect.roundsLeft > 0) {
                effect.roundsLeft--;
            }
        }
        // Use erase-remove idiom to remove animations which are done
        character->statusEffects.erase(
                std::remove_if(character->statusEffects.begin(), character->statusEffects.end(),
                               [&combat, &character](const StatusEffect& effect) {
                                   if(effect.roundsLeft == 0) {
                                       std::string logMessage = character->name + " is no longer affected by " + GetStatusEffectName(effect.type);
                                       combat.log.push_back(logMessage);
                                   }
                                   return effect.roundsLeft == 0;
                               }),
                character->statusEffects.end()
        );
    }
}
