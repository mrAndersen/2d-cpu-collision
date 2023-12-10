#include <global.h>

std::random_device rnd;

int random(int min, int max) {
    std::mt19937 rng(rnd());
    std::uniform_int_distribution<int> dist6(min, max);

    return dist6(rng);
}