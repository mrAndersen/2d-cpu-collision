#pragma once

#include <random>
#include "raylib.h"

struct Vec2 {
    float x = 0;
    float y = 0;

    Vec2 operator+(const Vec2 &other) const {
        return {x + other.x, y + other.y};
    }

    Vec2 operator-(const Vec2 &other) const {
        return {x - other.x, y - other.y};
    }

    Vec2 operator*(float scalar) const {
        return {x * scalar, y * scalar};
    }

    float magnitude() const {
        return std::sqrt(x * x + y * y);
    }

    Vec2 normalize() const {
        float mag = magnitude();
        return {x / mag, y / mag};
    }
};


int random(int min, int max);