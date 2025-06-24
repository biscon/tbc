//
// Created by bison on 13-03-25.
//

#ifndef SANDBOX_ANIMATIONDATA_H
#define SANDBOX_ANIMATIONDATA_H

#include "raylib.h"

enum class AnimationType {
    Blink,
    Attack,
    DamageNumber,
    Text,
    Death,
    BloodPool,
    SpeechBubble,
    Victory,
    FancyText,
};

struct BlinkAnimationState {
    int character;
    bool visible;
};

struct AttackAnimationState {
    int attacker;
    float startY;
    float endY;
    float currentY;
    float startX;
    float endX;
    float currentX;
    bool movingUp;
    float initialDelay;
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
    int character;
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

struct VictoryAnimationState {
    int character;
    float baseY;
    float jumpHeight;
    float jumpSpeed;
    float currentY;
    bool movingUp;
};

struct FancyTextAnimationState {
    char text[128];
    float y;
    float initialDelay;
    float letterPause;
    float fadeOutDuration;
    float alpha;

    int scrambleLength;     // How many characters are currently "scrambled"
    int finalRevealLength;  // How many characters are now real
    float timeSinceLastScramble;
    bool doneScrambling;
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
        VictoryAnimationState victory;
        FancyTextAnimationState fancyText;
    } state;

    bool IsDone() const {
        if(stay)
            return false;
        return time >= duration;
    }
};

#endif //SANDBOX_ANIMATIONDATA_H
