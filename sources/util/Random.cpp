//
// Created by bison on 09-01-25.
//

#include "Random.h"

// Simple random number generator (you could expand this if needed)
int RandomInRange(int min, int max) {
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

float GetRandomFloat01() {
    static std::mt19937 rng(rd());                    // Mersenne Twister RNG
    static std::uniform_real_distribution<float> dist(0.0f, 1.0f); // [0.0, 1.0)
    return dist(rng);
}

float RandomFloat(float min, float max) {
    static std::mt19937 rng(rd());                    // Mersenne Twister RNG
    static std::uniform_real_distribution<float> dist(min, max); // [0.0, 1.0)
    return dist(rng);
}

