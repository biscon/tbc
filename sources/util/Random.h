//
// Created by bison on 09-01-25.
//

#ifndef SANDBOX_RANDOM_H
#define SANDBOX_RANDOM_H

#include <stdexcept>
#include <random>

inline static std::random_device rd;  // C++17+
inline static std::mt19937 gen(rd());

int RandomInRange(int min, int max);

// Returns a reference to a random element in the vector.
// Throws std::out_of_range if the vector is empty.
template <typename T>
T& GetRandomElement(std::vector<T>& vec) {
    if (vec.empty()) {
        throw std::out_of_range("getRandomElement: vector is empty");
    }
    std::uniform_int_distribution<size_t> dist(0, vec.size() - 1);
    return vec[dist(gen)];
}

float GetRandomFloat01();
float RandomFloat(float min, float max);

#endif //SANDBOX_RANDOM_H
