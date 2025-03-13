//
// Created by bison on 24-01-25.
//

#ifndef SANDBOX_COMBATANIMATION_H
#define SANDBOX_COMBATANIMATION_H

#include "Level.h"

void PlayAttackAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender);
void PlayDefendAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender);
void PlayAttackDefendAnimation(SpriteData& spriteData, CharacterData& charData, Level &level, int attacker, int defender);
void PlayEnemyVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Level &level);
void PlayPlayerVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Level& level);
void RemoveAttackAnimations(Level &level);
int GetBloodIntensity(int dmg, int attackerAttack);

#endif //SANDBOX_COMBATANIMATION_H
