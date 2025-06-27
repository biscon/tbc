//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_ANIMATION_H
#define SANDBOX_ANIMATION_H

#include "raylib.h"
#include "data/GameData.h"

void SetupBlinkAnimation(Animation &animation, int character, float duration);
void SetupAttackAnimation(Animation &animation, int attacker, float duration, float startY, float endY, float startX, float endX, float initialDelay = 0.0f);
void SetupDamageNumberAnimation(Animation &animation, const char *text, float x, float y, Color color, int fontSize);
void SetupTextAnimation(Animation &animation, const char *text, float y, float duration, float initialDelay = 0.0f);
void SetupDeathAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, int character, float duration);
void SetupBloodPoolAnimation(Animation &animation, Vector2 position, float duration);
void SetupSpeechBubbleAnimation(Animation &animation, const char *text, float x, float y, float duration, float initialDelay = 0.0f);
void SetupVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, int character, float duration, float jumpHeight, float jumpSpeed);
void SetupFancyTextAnimation(Animation &animation, const char *text, int fontSize, float y, float holdDuration, float initialDelay, float letterPause, float fadeOutDuration);
void UpdateAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, float dt);

#endif //SANDBOX_ANIMATION_H
