#pragma once

#include <vector>
#include <Entity.h>
#include <random>
#include <raylib.h>
#include <unordered_map>
#include <mutex>
#include <atomic>
#include <condition_variable>

struct Bucket {
    Rectangle rect = {};

    std::vector<Entity *> elements = {};
};

class ServiceContainer;

class EntityManager {
protected:
    ServiceContainer *pContainer;

    std::unordered_map<int, Bucket> buckets = {};

    std::unordered_map<int, std::vector<Entity>> elements = {};

    int threads = 8;

    int currentInsertIndex = 0;

    int bucketWidth = 0;

    int bucketHeight = 0;

    int rows = 4;

    int cols = 4;

    int debug = 0;

    int maxDebug = 2;

    float updateMs = 0;

    float renderMs = 0;

    bool updating = true;

    std::atomic<int> updates = 0;

    void add(const Entity &e);

    void initBuckets();
public:
    explicit EntityManager(ServiceContainer *pContainer);

    void update();

    void render();

    void addRandom();

    void addAtPosition(const Vector2 &p, int direction);
};
