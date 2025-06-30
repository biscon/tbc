//
// Created by bison on 23-01-25.
//

#include <algorithm>
#include "StatusEffectRunner.h"
#include "Combat.h"
#include "graphics/CharacterSprite.h"
#include "character/StatusEffect.h"

static void ApplyStatusEffect(GameData& data, Level &combat, PlayField &gridState, int character, StatusEffect& effect) {
    SpriteData& spriteData = data.spriteData;
    CharacterData& charData = data.charData;
    Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);

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
            int damage = DealDamageStatusEffect(data, combat, character, (int) effect.value);
            if(damage > 0) {
                CreateExplosionEffect(*gridState.particleManager, {charPos.x, charPos.y}, 5, 10.0f, 0.2f);
            }
            break;
        }
        default:
            break;
    }
    // check if dead
    if(charData.stats[character].health <= 0) {
        KillCharacter(spriteData, charData, combat, character);
    }
}

void ApplyStatusEffects(GameData& data, Level &level, PlayField &playField) {
    for(auto& character : level.turnOrder) {
        // skip dead characters
        if(data.charData.stats[character].health <= 0) {
            continue;
        }
        for(auto& effect : data.charData.statusEffects[character]) {
            ApplyStatusEffect(data, level, playField, character, effect);
        }
    }
}

// runs when a round is over
void UpdateStatusEffects(CharacterData& charData, Level &level) {
    for(auto &character : level.turnOrder) {
        for(auto &effect : charData.statusEffects[character]) {
            if(effect.roundsLeft > 0) {
                effect.roundsLeft--;
            }
        }
        // Use erase-remove idiom to remove animations which are done
        charData.statusEffects[character].erase(
                std::remove_if(charData.statusEffects[character].begin(), charData.statusEffects[character].end(),
                               [&charData, &level, &character](const StatusEffect& effect) {
                                   if(effect.roundsLeft == 0) {
                                       std::string logMessage = charData.name[character] + " is no longer affected by " + GetStatusEffectName(effect.type);
                                       level.log.push_back(logMessage);
                                   }
                                   return effect.roundsLeft == 0;
                               }),
                charData.statusEffects[character].end()
        );
    }
}
