//
// Created by bison on 10-01-25.
//

#include <cstring>
#include "Animation.h"
#include "raylib.h"
#include "reasings.h"
#include "raymath.h"
#include "util/Random.h"

void SetupBlinkAnimation(Animation &animation, Character *character, float duration) {
    animation.type = AnimationType::Blink;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
    animation.state.blink.character = character;
    TraceLog(LOG_INFO, "SetupBlinkAnimation: %s", character->name.c_str());
    //TraceLog(LOG_INFO, "hej per %i", character->health);
}

void SetupAttackAnimation(Animation &animation, Character *attacker, float duration, float startY, float endY, float startX, float endX) {
    animation.type = AnimationType::Attack;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
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
    animation.stay = false;
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
    animation.stay = false;
    strncpy(animation.state.text.text, text, 50);
    animation.state.text.y = y;
    animation.state.text.alpha = 1.0f;
    animation.state.text.veilAlpha = 0.0f;
    animation.state.text.initialDelay = initialDelay;
    TraceLog(LOG_INFO, "SetupTextAnimation: %s", text);
}

void SetupDeathAnimation(Animation &animation, Character *character, float duration) {
    animation.type = AnimationType::Death;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
    animation.state.death.character = character;
    animation.state.death.startY = character->sprite.player.position.y;
    int bounceY = RandomInRange(10, 15);
    animation.state.death.bounceY = character->sprite.player.position.y - (float) bounceY; // Bounce up
    int fallY = RandomInRange(-3,3);
    animation.state.death.endY = character->sprite.player.position.y + (float) fallY;    // Fall slightly lower
    animation.state.death.startX = character->sprite.player.position.x;
    animation.state.death.endX = character->sprite.player.position.x + (float) RandomInRange(-5, 3); // Random horizontal position
    animation.state.death.rotation = 0; // Start with no rotation
    TraceLog(LOG_INFO, "SetupDeathAnimation: %s", character->name.c_str());
}

void SetupBloodPoolAnimation(Animation &animation, Vector2 position, float duration) {
    animation.type = AnimationType::BloodPool;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = true;
    animation.state.bloodPool.radius1 = 0;
    animation.state.bloodPool.radius2 = 0;
    animation.state.bloodPool.radius3 = 0;
    animation.state.bloodPool.endRadius1 = (float) RandomInRange(20, 35);
    animation.state.bloodPool.endRadius2 = (float) RandomInRange(15, 25);
    animation.state.bloodPool.endRadius3 = (float) RandomInRange(8, 15);
    animation.state.bloodPool.position = position;
    int range = 3;
    animation.state.bloodPool.offset1 = {0,0};
    animation.state.bloodPool.offset2 = {(float) RandomInRange(-range, range), (float) RandomInRange(-range, range)};
    animation.state.bloodPool.offset3 = {(float) RandomInRange(-range, range), (float) RandomInRange(-range, range)};
    animation.state.bloodPool.duration = duration;
    TraceLog(LOG_INFO, "SetupBloodPoolAnimation: %f, %f", position.x, position.y);
}

void SetupSpeechBubbleAnimation(Animation &animation, const char *text, float x, float y, float duration,
                                float initialDelay) {
    animation.type = AnimationType::SpeechBubble;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
    strncpy(animation.state.speechBubble.text, text, 50);
    animation.state.speechBubble.x = x;
    animation.state.speechBubble.y = y;
    animation.state.speechBubble.alpha = 1.0f;
    animation.state.speechBubble.initialDelay = initialDelay;
    TraceLog(LOG_INFO, "SetupSpeechBubbleAnimation: %s", text);
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
        case AnimationType::Death: {
            // Duration is split into two phases: bounce and fall
            float bounceDuration = animation.duration * 0.3f; // 30% of duration for bounce
            float fallDuration = animation.duration * 0.7f;   // 70% of duration for fall
            if (animation.time <= bounceDuration) {
                // Bounce phase (upwards motion)
                animation.state.death.character->sprite.player.position.y = EaseQuadOut(animation.time, animation.state.death.startY, animation.state.death.bounceY - animation.state.death.startY, bounceDuration);
            } else {
                // Fall phase (downwards motion)
                float fallTime = animation.time - bounceDuration;
                animation.state.death.character->sprite.player.position.y = EaseQuadIn(fallTime, animation.state.death.bounceY, animation.state.death.endY - animation.state.death.bounceY, fallDuration);
                animation.state.death.character->sprite.player.position.x = EaseQuadIn(fallTime, animation.state.death.startX, animation.state.death.endX - animation.state.death.startX, fallDuration);
            }
            // Rotate the character
            animation.state.death.character->sprite.player.rotation = EaseQuadOut(animation.time, 0, 90, animation.duration); // Rotate up to 90 degrees
            break;
        }
        case AnimationType::BloodPool: {
            // Calculate the radius of the blood pool
            if(animation.time <= animation.duration) {
                // Ease out the radius from 0 to 20 (max radius
                float progress = animation.time / animation.duration;
                animation.state.bloodPool.radius1 = EaseQuadOut(progress, 0, animation.state.bloodPool.endRadius1, animation.duration);
                animation.state.bloodPool.radius2 = EaseQuadOut(progress, 0, animation.state.bloodPool.endRadius2, animation.duration);
                animation.state.bloodPool.radius3 = EaseQuadOut(progress, 0, animation.state.bloodPool.endRadius3, animation.duration);
            }
            break;
        }
        case AnimationType::SpeechBubble: {
            const float fadeDuration = 0.5f; // Half a second for fade in/out
            const float fullAlphaDuration = animation.duration - 2 * fadeDuration;
            animation.state.speechBubble.initialDelay -= dt;
            if (animation.state.speechBubble.initialDelay > 0) {
                animation.time = 0;
                animation.state.speechBubble.alpha = 0.0f;
                break;
            }

            if (animation.time <= fadeDuration) {
                // Fade in
                float progress = animation.time / fadeDuration;
                animation.state.speechBubble.alpha = progress;                  // Text fades in
            } else if (animation.time <= fadeDuration + fullAlphaDuration) {
                // Fully visible
                animation.state.speechBubble.alpha = 1.0f;                     // Text is fully visible
            } else {
                // Fade out
                float fadeOutStartTime = fadeDuration + fullAlphaDuration;
                float progress = (animation.time - fadeOutStartTime) / fadeDuration;
                animation.state.speechBubble.alpha = 1.0f - progress;          // Text fades out
            }
            break;
        }
    }
}




