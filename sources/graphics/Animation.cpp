//
// Created by bison on 10-01-25.
//

#include <cstring>
#include "Animation.h"
#include "raylib.h"
#include "reasings.h"
#include "raymath.h"
#include "util/Random.h"
#include "CharacterSprite.h"

void SetupBlinkAnimation(Animation &animation, int character, float duration) {
    animation.type = AnimationType::Blink;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
    animation.state.blink.character = character;
}

void SetupAttackAnimation(Animation &animation, int attacker, float duration, float startY, float endY, float startX, float endX, float initialDelay) {
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
    animation.state.attack.initialDelay = initialDelay;
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

void SetupDeathAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, int character, float duration) {
    Vector2 charPos = GetCharacterSpritePos(spriteData, charData.sprite[character]);
    animation.type = AnimationType::Death;
    animation.duration = duration;
    animation.time = 0;
    animation.stay = false;
    animation.state.death.character = character;
    animation.state.death.startY = charPos.y;
    int bounceY = RandomInRange(10, 15);
    animation.state.death.bounceY = charPos.y - (float) bounceY; // Bounce up
    int fallY = RandomInRange(-3,3);
    animation.state.death.endY = charPos.y + (float) fallY;    // Fall slightly lower
    animation.state.death.startX = charPos.x;
    animation.state.death.endX = charPos.x + (float) RandomInRange(-5, 3); // Random horizontal position
    animation.state.death.rotation = 0; // Start with no rotation
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

void SetupVictoryAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, int character, float duration, float jumpHeight, float jumpSpeed) {
    animation.type = AnimationType::Victory;
    animation.duration = duration;
    animation.time = 0.0f;
    animation.stay = false;

    VictoryAnimationState &state = animation.state.victory;
    state.character = character;
    state.baseY = GetCharacterSpritePosY(spriteData, charData.sprite[character]); // Assuming `position.y` is the character's initial vertical position
    state.jumpHeight = jumpHeight;
    state.jumpSpeed = jumpSpeed;
    state.currentY = state.baseY;
    state.movingUp = true;
}

void SetupFancyTextAnimation(Animation &animation, const char *text, float y, float holdDuration, float initialDelay, float letterPause, float fadeOutDuration) {
    animation.type = AnimationType::FancyText;
    animation.time = 0.0f;
    animation.stay = false;

    FancyTextAnimationState &state = animation.state.fancyText;
    strncpy(state.text, text, sizeof(state.text));
    state.text[sizeof(state.text) - 1] = '\0';

    int textLen = (int) strlen(state.text);

    state.y = y;
    state.initialDelay = initialDelay;
    state.letterPause = letterPause;
    state.fadeOutDuration = fadeOutDuration;
    state.alpha = 1.0f;

    state.scrambleLength = 0;
    state.finalRevealLength = 0;
    state.timeSinceLastScramble = 0.0f;
    state.doneScrambling = false;

    float scrambleDuration = textLen * letterPause;
    float descrambleDuration = textLen * letterPause;

    animation.duration = initialDelay + scrambleDuration + descrambleDuration + holdDuration + fadeOutDuration;

    TraceLog(LOG_INFO, "FancyText setup: total duration = %.2f", animation.duration);
}


void UpdateAnimation(SpriteData& spriteData, CharacterData& charData, Animation &animation, float dt) {
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
            // Check and decrement initial delay
            animation.state.attack.initialDelay -= dt;
            if (animation.state.attack.initialDelay > 0) {
                animation.time = 0.0f; // Reset animation time while waiting
                animation.state.attack.currentX = animation.state.attack.startX; // Keep at start position
                animation.state.attack.currentY = animation.state.attack.startY;
                break;
            }

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
            CharacterSprite& charSprite = charData.sprite[animation.state.death.character];
            Vector2 charPos = GetCharacterSpritePos(spriteData, charSprite);
            if (animation.time <= bounceDuration) {
                // Bounce phase (upwards motion)
                SetCharacterSpritePosY(spriteData, charSprite, EaseQuadOut(animation.time, animation.state.death.startY, animation.state.death.bounceY - animation.state.death.startY, bounceDuration));
            } else {
                // Fall phase (downwards motion)
                float fallTime = animation.time - bounceDuration;
                Vector2 newPos = {
                    EaseQuadIn(fallTime, animation.state.death.startX, animation.state.death.endX - animation.state.death.startX, fallDuration),
                    EaseQuadIn(fallTime, animation.state.death.bounceY, animation.state.death.endY - animation.state.death.bounceY, fallDuration)
                };
                SetCharacterSpritePos(spriteData, charSprite, newPos);
            }
            // Rotate the character
            SetCharacterSpriteRotation(spriteData, charSprite, EaseQuadOut(animation.time, 0, 90, animation.duration));
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
        case AnimationType::Victory: {
            VictoryAnimationState &state = animation.state.victory;

            // Update jump logic
            if (state.movingUp) {
                state.currentY -= state.jumpSpeed * dt;
                if (state.currentY <= state.baseY - state.jumpHeight) {
                    state.currentY = state.baseY - state.jumpHeight;
                    state.movingUp = false;
                }
            } else {
                state.currentY += state.jumpSpeed * dt;
                if (state.currentY >= state.baseY) {
                    state.currentY = state.baseY;
                    state.movingUp = true;
                }
            }

            // Update character's position
            SetCharacterSpritePosY(spriteData, charData.sprite[state.character], state.currentY);
            break;
        }
        case AnimationType::FancyText: {
            FancyTextAnimationState &state = animation.state.fancyText;
            float t = animation.time;

            const char* sourceText = state.text;
            int totalLen = (int) strlen(sourceText);

            float scrambleStart = state.initialDelay;
            float scrambleEnd = scrambleStart + (totalLen * state.letterPause);
            float decodeStart = scrambleEnd;
            float decodeEnd = decodeStart + (totalLen * state.letterPause);
            float fadeOutStart = animation.duration - state.fadeOutDuration;

            // Full opacity unless fading
            state.alpha = 1.0f;

            if (t < scrambleStart) {
                state.scrambleLength = 0;
                state.finalRevealLength = 0;
                break;
            }

            // Scramble phase (random letters appear)
            if (t < scrambleEnd) {
                float progress = (t - scrambleStart) / (totalLen * state.letterPause);
                state.scrambleLength = (int)(progress * totalLen);
                if (state.scrambleLength > totalLen)
                    state.scrambleLength = totalLen;

                // Throttle randomness to 10 updates per second
                state.timeSinceLastScramble += dt;
                if (state.timeSinceLastScramble >= 0.1f) {
                    state.timeSinceLastScramble = 0.0f;
                }
            } else {
                state.scrambleLength = totalLen;
            }

            // Decode phase (revealing final characters)
            if (t >= decodeStart && t < decodeEnd) {
                float progress = (t - decodeStart) / (totalLen * state.letterPause);
                state.finalRevealLength = (int)(progress * totalLen);
                if (state.finalRevealLength > totalLen)
                    state.finalRevealLength = totalLen;
            } else if (t >= decodeEnd) {
                state.finalRevealLength = totalLen;
                state.doneScrambling = true;
            }

            // Fade-out phase
            if (t >= fadeOutStart) {
                float fadeProgress = (t - fadeOutStart) / state.fadeOutDuration;
                if (fadeProgress > 1.0f)
                    fadeProgress = 1.0f;
                state.alpha = 1.0f - fadeProgress;
            }

            break;
        }
    }
}




