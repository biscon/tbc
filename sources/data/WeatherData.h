//
// Created by bison on 18-08-25.
//

#ifndef SANDBOX_WEATHERDATA_H
#define SANDBOX_WEATHERDATA_H

#include <vector>
#include "raylib.h"

enum class WeatherType {
    Sunny,
    Overcast,
    Rain,
    Thunder
};

struct RainDrop {
    Vector2 position;
    Vector2 velocity;
    float length;
};

struct WeatherData {
    std::vector<RainDrop> drops; // fixed-size vector, preallocated
    float mapWidth;
    float mapHeight;
    float intensity; // 0.0 = no rain, 1.0 = full rain

    // Thunder
    float thunderTimer;
    float nextThunderTime;
    bool lightningActive;
    float lightningStrength; // 0–1 fade

    WeatherType weatherType;
};

#endif //SANDBOX_WEATHERDATA_H
