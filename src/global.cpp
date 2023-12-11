#include <global.h>

std::random_device rnd;

int random(int min, int max) {
    std::mt19937 rng(rnd());
    std::uniform_int_distribution<int> dist6(min, max);

    return dist6(rng);
}

float distance(const Vec2 &vec1, const Vec2 &vec2) {
    float dx = vec1.x - vec2.x;
    float dy = vec1.y - vec2.y;
    return sqrtf(dx * dx + dy * dy);
}
