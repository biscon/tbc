//
// Created by bison on 09-01-25.
//

#include <random>
#include "Random.h"

// Simple random number generator (you could expand this if needed)
int RandomInRange(int min, int max) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}