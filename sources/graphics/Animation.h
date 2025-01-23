//
// Created by bison on 10-01-25.
//

#ifndef SANDBOX_ANIMATION_H
#define SANDBOX_ANIMATION_H

#include "character/Character.h"
#include "raylib.h"

enum class AnimationType {
    Blink,
    Attack,
    DamageNumber,
    Text,
    Death,
    BloodPool,
    SpeechBubble
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

struct DeathAnimationState {
    struct Character* character;
    float startY;
    float endY;
    float startX;
    float endX;
    float bounceY;
    float rotation;
};

struct BloodPoolAnimationState {
    Vector2 position;
    float radius1;
    float endRadius1;
    float radius2;
    float endRadius2;
    float radius3;
    float endRadius3;
    Vector2 offset1;
    Vector2 offset2;
    Vector2 offset3;
    float duration;
    float time;
};

struct SpeechBubbleAnimationState {
    char text[50];
    float x;
    float y;
    float alpha;
    float initialDelay;
};

struct Animation {
    AnimationType type;
    float duration;
    float time;
    bool stay; // if true, the animation will stay on the last frame

    union {
        BlinkAnimationState blink;
        AttackAnimationState attack;
        DamageNumberAnimationState damageNumber;
        TextAnimationState text;
        DeathAnimationState death;
        BloodPoolAnimationState bloodPool;
        SpeechBubbleAnimationState speechBubble;
    } state;

    bool IsDone() const {
        if(stay)
            return false;
        return time >= duration;
    }
};

void SetupBlinkAnimation(Animation &animation, Character *character, float duration);
void SetupAttackAnimation(Animation &animation, Character *attacker, float duration, float startY, float endY, float startX, float endX);
void SetupDamageNumberAnimation(Animation &animation, const char *text, float x, float y, Color color, int fontSize);
void SetupTextAnimation(Animation &animation, const char *text, float y, float duration, float initialDelay = 0.0f);
void SetupDeathAnimation(Animation &animation, Character *character, float duration);
void SetupBloodPoolAnimation(Animation &animation, Vector2 position, float duration);
void SetupSpeechBubbleAnimation(Animation &animation, const char *text, float x, float y, float duration, float initialDelay = 0.0f);
void UpdateAnimation(Animation &animation, float dt);

#endif //SANDBOX_ANIMATION_H
