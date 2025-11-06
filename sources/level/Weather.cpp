//
// Created by bison on 18-08-25.
//

#include "Weather.h"
#include "util/Random.h"
#include "util/MathUtil.h"

// Initialize weather system with max raindrops
void InitWeather(WeatherData &weather, int maxDrops, float mapWidth, float mapHeight) {
    weather.mapWidth = mapWidth;
    weather.mapHeight = mapHeight;
    weather.intensity = 1.0f;
    weather.drops.resize(maxDrops);

    weather.thunderTimer = 0.0f;
    weather.nextThunderTime = RandomFloat(5.0f, 15.0f);
    weather.lightningActive = false;
    weather.lightningStrength = 0.0f;

    for (auto &d : weather.drops) {
        d.position.x = GetRandomValue(0, (int)mapWidth);
        d.position.y = GetRandomValue(0, (int)mapHeight);
        d.velocity.x = (float)GetRandomValue(0, 100) / 100.0f * 60.0f; // slant px/s
        d.velocity.y = (float)GetRandomValue(300, 500);                // fall px/s
        d.length = (float)GetRandomValue(4, 8);
    }

    weather.weatherType = WeatherType::Rain;
}

// Update active raindrops based on dt and intensity
void UpdateWeather(WeatherData &weather, float dt) {
    if(weather.weatherType == WeatherType::Thunder || weather.weatherType == WeatherType::Rain) {
        int activeDrops = (int) (weather.drops.size() * weather.intensity);
        for (int i = 0; i < activeDrops; i++) {
            RainDrop &d = weather.drops[i];
            d.position.x += d.velocity.x * dt;
            d.position.y += d.velocity.y * dt;

            // recycle if offscreen
            if (d.position.x > weather.mapWidth || d.position.y > weather.mapHeight) {
                d.position.x = (float) GetRandomValue(0, (int) weather.mapWidth);
                d.position.y = 0.0f;
                d.velocity.x = (float) GetRandomValue(0, 100) / 100.0f * 60.0f;
                d.velocity.y = (float) GetRandomValue(300, 500);
                d.length = (float) GetRandomValue(4, 8);
            }
        }
    }

    if(weather.weatherType == WeatherType::Thunder) {
        // Thunder timing
        weather.thunderTimer += dt;
        if (weather.thunderTimer >= weather.nextThunderTime) {
            weather.lightningActive = true;
            weather.lightningStrength = 1.0f;
            weather.thunderTimer = 0.0f;
            weather.nextThunderTime = RandomFloat(5, 15);
            // Play thunder sound after delay here if you want
            TraceLog(LOG_INFO, "LIGHTNING STRIKE!!!");
        }

        // Lightning fade
        if (weather.lightningActive) {
            weather.lightningStrength -= dt * 3.0f; // ~0.3s fade
            if (weather.lightningStrength <= 0.0f) {
                weather.lightningStrength = 0.0f;
                weather.lightningActive = false;
            }
        }
    }
}

static Color RainColorWithAmbient(Color ambient) {
    // 1. Compute brightness of ambient (0.0 - 1.0)
    float brightness = (ambient.r * 0.3f + ambient.g * 0.59f + ambient.b * 0.11f) / 255.0f;

    // 2. Bias brightness so rain is always at least ~40%
    brightness = 0.4f + 0.6f * brightness;

    // 3. Base rain color (neutral gray)
    float base = 200.0f * brightness;

    // 4. Blend in a bit of the ambient hue
    float mix = 0.25f; // how much to tint by ambient color
    unsigned char r = (unsigned char)(base * (1.0f - mix) + ambient.r * mix);
    unsigned char g = (unsigned char)(base * (1.0f - mix) + ambient.g * mix);
    unsigned char b = (unsigned char)(base * (1.0f - mix) + ambient.b * mix);

    return { r, g, b, 255 };
}

static Color RainColorWithAmbient3(Color ambient) {
    // Convert ambient to grayscale brightness
    float brightness = (ambient.r * 0.3f + ambient.g * 0.59f + ambient.b * 0.11f) / 255.0f;

    // Clamp and bias so rain is always somewhat visible
    brightness = 0.4f + 0.6f * brightness; // 40–100% of base rain brightness

    unsigned char v = (unsigned char)(brightness * 200); // base ~200 gray
    return { v, v, v, 255 };
}


// Draw only active drops
void DrawWeather(const WeatherData &weather, Color ambientColor) {
    Color color = RainColorWithAmbient(ambientColor);

    if(weather.weatherType == WeatherType::Thunder) {
        // Ambient lightning flash overlay
        if (weather.lightningActive) {
            float s = weather.lightningStrength;
            color = LerpColor(color, {220, 220, 255, 255}, s);
        }
    }

    if(weather.weatherType == WeatherType::Thunder || weather.weatherType == WeatherType::Rain) {
        int activeDrops = (int) (weather.drops.size() * weather.intensity);
        for (int i = 0; i < activeDrops; i++) {
            const RainDrop &d = weather.drops[i];
            Vector2 end = {
                    d.position.x + d.velocity.x / d.velocity.y * d.length, // keep slant
                    d.position.y + d.length
            };
            DrawLineV(d.position, end, color);
        }
    }
}