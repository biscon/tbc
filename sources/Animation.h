//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_ANIMATION_H
#define SANDBOX_ANIMATION_H

#include "Character.h"
#include "raylib.h"

enum class AnimationType {
    Blink,
    Attack,
    DamageNumber,
    Text,
};

struct BlinkAnimationState {
    struct Character* character; // Assuming Character is defined elsewhere
    bool visible;
};

struct AttackAnimationState {
    struct Character* attacker;
    float startY;
    float endY;
    float currentY;
    float startX;
    float endX;
    float currentX;
    bool movingUp;
};

struct DamageNumberAnimationState {
    char text[50];
    Color color;
    float x;
    float y;
    int fontSize;
};

struct TextAnimationState {
    char text[50];
    float y;
    float alpha;
    float veilAlpha;
    float initialDelay;
};

struct Animation {
    AnimationType type;
    float duration;
    float time;

    union {
        BlinkAnimationState blink;
        AttackAnimationState attack;
        DamageNumberAnimationState damageNumber;
        TextAnimationState text;
    } state;

    bool IsDone() const {
        return time >= duration;
    }
};

void SetupBlinkAnimation(Animation &animation, Character *character, float duration);
void SetupAttackAnimation(Animation &animation, Character *attacker, float duration, float startY, float endY, float startX, float endX);
void SetupDamageNumberAnimation(Animation &animation, const char *text, float x, float y, Color color, int fontSize);
void SetupTextAnimation(Animation &animation, const char *text, float y, float duration, float initialDelay = 0.0f);
void UpdateAnimation(Animation &animation, float dt);

#endif //SANDBOX_ANIMATION_H
