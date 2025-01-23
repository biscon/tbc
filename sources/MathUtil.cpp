//
// Created by bison on 19-01-25.
//

#include "MathUtil.h"
#include "raymath.h"
#include <cmath>

int DistanceSquared(Vector2i a, Vector2i b) {
    int dx = a.x - b.x;
    int dy = a.y - b.y;
    return dx * dx + dy * dy;
}

float Distance(Vector2i start, Vector2i end) {
    int dx = end.x - start.x;
    int dy = end.y - start.y;
    return sqrtf(dx * dx + dy * dy);
}

Vector2 CalculateDirection(Vector2 start, Vector2 end) {
    return Vector2Normalize(Vector2Subtract(end, start));
}

Vector2 CalculateDirection(Vector2i start, Vector2i end) {
    Vector2 s = { static_cast<float>(start.x), static_cast<float>(start.y) };
    Vector2 e = { static_cast<float>(end.x), static_cast<float>(end.y) };
    return Vector2Normalize(Vector2Subtract(e, s));
}