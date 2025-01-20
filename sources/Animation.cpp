//
// Created by bison on 10-01-25.
//

#include <cstring>
#include "Animation.h"
#include "raylib.h"
#include "reasings.h"
#include "raymath.h"

void SetupBlinkAnimation(Animation &animation, Character *character, float duration) {
    animation.type = AnimationType::Blink;
    animation.duration = duration;
    animation.time = 0;
    animation.state.blink.character = character;
    TraceLog(LOG_INFO, "SetupBlinkAnimation: %s", character->name.c_str());
    //TraceLog(LOG_INFO, "hej per %i", character->health);
}

void SetupAttackAnimation(Animation &animation, Character *attacker, float duration, float startY, float endY, float startX, float endX) {
    animation.type = AnimationType::Attack;
    animation.duration = duration;
    animation.time = 0;
    animation.state.attack.attacker = attacker;
    animation.state.attack.startY = startY;
    animation.state.attack.endY = endY;
    animation.state.attack.currentY = 0;
    animation.state.attack.movingUp = true;
    animation.state.attack.startX = startX;
    animation.state.attack.endX = endX;
    animation.state.attack.currentX = 0;

    TraceLog(LOG_INFO, "SetupAttackAnimation: %s", attacker->name.c_str());
}

void SetupDamageNumberAnimation(Animation &animation, const char *text, float x, float y, Color color, int fontSize) {
    animation.type = AnimationType::DamageNumber;
    animation.duration = 2.0f;
    animation.time = 0;
    strncpy(animation.state.damageNumber.text, text, 50);
    animation.state.damageNumber.color = color;
    animation.state.damageNumber.x = x;
    animation.state.damageNumber.y = y;
    animation.state.damageNumber.fontSize = fontSize;
    TraceLog(LOG_INFO, "SetupDamageNumberAnimation: %s", text);
}

void SetupTextAnimation(Animation &animation, const char *text, float y, float duration, float initialDelay) {
    animation.type = AnimationType::Text;
    animation.duration = duration;
    animation.time = 0;
    strncpy(animation.state.text.text, text, 50);
    animation.state.text.y = y;
    animation.state.text.alpha = 1.0f;
    animation.state.text.veilAlpha = 0.0f;
    animation.state.text.initialDelay = initialDelay;
    TraceLog(LOG_INFO, "SetupTextAnimation: %s", text);
}

void UpdateAnimation(Animation &animation, float dt) {
    animation.time += dt;
    switch(animation.type) {
        case AnimationType::Blink: {
            // set visible so it's true for 100 ms and false for 100 ms
            /*
             * 5 would slow it to 2.5 blinks per second.
             * 20 would double the speed to 10 blinks per second.
             */
            animation.state.blink.visible = (int) (animation.time * 5) % 2 == 0;
            break;
        }
        case AnimationType::Attack: {
            // Calculate current position
            if (animation.state.attack.movingUp) {
                animation.state.attack.currentX = EaseQuadOut(animation.time, animation.state.attack.startX, animation.state.attack.endX - animation.state.attack.startX, animation.duration);
                animation.state.attack.currentY = EaseQuadOut(animation.time, animation.state.attack.startY, animation.state.attack.endY - animation.state.attack.startY, animation.duration);
            } else {
                animation.state.attack.currentX = EaseQuadIn(animation.time, animation.state.attack.endX, animation.state.attack.startX - animation.state.attack.endX, animation.duration);
                animation.state.attack.currentY = EaseQuadIn(animation.time, animation.state.attack.endY, animation.state.attack.startY - animation.state.attack.endY, animation.duration);
            }

            // Check if the animation phase is done
            if (animation.time >= animation.duration && animation.state.attack.movingUp) {
                animation.time = 0.0f;          // Reset time
                animation.state.attack.movingUp = !animation.state.attack.movingUp; // Reverse direction
            }
            break;
        }
        case AnimationType::DamageNumber: {
            // Move the number up
            animation.state.damageNumber.y -= dt * 50.0f;
            break;
        }
        case AnimationType::Text: {
            const float fadeDuration = 0.5f; // Half a second for fade in/out
            const float fullAlphaDuration = animation.duration - 2 * fadeDuration;
            animation.state.text.initialDelay -= dt;
            if (animation.state.text.initialDelay > 0) {
                animation.time = 0;
                animation.state.text.alpha = 0.0f;
                animation.state.text.veilAlpha = 0.0f;
                break;
            }

            if (animation.time <= fadeDuration) {
                // Fade in
                float progress = animation.time / fadeDuration;
                animation.state.text.alpha = progress;                  // Text fades in
                animation.state.text.veilAlpha = progress * 0.75f;       // Veil fades to 0.5
            } else if (animation.time <= fadeDuration + fullAlphaDuration) {
                // Fully visible
                animation.state.text.alpha = 1.0f;                     // Text is fully visible
                animation.state.text.veilAlpha = 0.75f;                 // Veil stays at 0.5
            } else {
                // Fade out
                float fadeOutStartTime = fadeDuration + fullAlphaDuration;
                float progress = (animation.time - fadeOutStartTime) / fadeDuration;
                animation.state.text.alpha = 1.0f - progress;          // Text fades out
                animation.state.text.veilAlpha = (1.0f - progress) * 0.75f; // Veil fades to 0.0
            }
            break;
        }
    }
}


