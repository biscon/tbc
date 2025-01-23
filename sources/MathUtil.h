//
// Created by bison on 19-01-25.
//

#ifndef SANDBOX_MATHUTIL_H
#define SANDBOX_MATHUTIL_H

#include "raylib.h"

struct Vector2i {
    int x, y;
    Vector2i(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const Vector2i& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Vector2i& other) const { return !(*this == other); }
};

int DistanceSquared(Vector2i a, Vector2i b);
float Distance(Vector2i start, Vector2i end);
Vector2 CalculateDirection(Vector2 start, Vector2 end);
Vector2 CalculateDirection(Vector2i start, Vector2i end);

#endif //SANDBOX_MATHUTIL_H
