//
// Created by bison on 09-01-25.
//

#include "Random.h"

// Simple random number generator (you could expand this if needed)
int RandomInRange(int min, int max) {
    std::uniform_int_distribution<> dis(min, max);
    return dis(gen);
}

