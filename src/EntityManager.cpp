#include "EntityManager.h"
#include <ServiceContainer.h>
#include <raylib.h>
#include <string>
#include <global.h>
#include <format>
#include <thread>


EntityManager::EntityManager(ServiceContainer *pContainer) : pContainer(pContainer) {
    initBuckets();
}

void EntityManager::initBuckets() {
    int index = 0;
    buckets.clear();

    bucketWidth = pContainer->width / cols;
    bucketHeight = pContainer->height / rows;

    for (int i = 0; i < pContainer->width; i += bucketWidth) {
        for (int j = 0; j < pContainer->height; j += bucketHeight) {
            Bucket b;

            b.rect = Rectangle((float) i, (float) j, (float) bucketWidth, (float) bucketHeight);
            buckets.emplace(index, b);

            index++;
        }
    }

    for (int i = 0; i < threads; ++i) {
        elements.emplace(i, std::vector<Entity>());
    }
}


void EntityManager::update() {
    auto start = GetTime();
    auto mouse = GetMousePosition();
    auto dt = GetFrameTime() * 1000;

    Rectangle screen(0, 0, pContainer->width, pContainer->height);

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (bolderRadius == 0) {
            bolderStartDrag = mouse;
        }

        bolderRadius += dt * 0.1;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT)) {
        auto angle = std::atan2(mouse.y - bolderStartDrag.y, mouse.x - bolderStartDrag.x) * (180 / PI);
        angle = ((int) angle + 180) % 360;

        Entity e;
        e.position = {bolderStartDrag.x, bolderStartDrag.y};
        e.radius = bolderRadius;
        e.direction = (int) angle;
        e.speed = 2;

        if (bolderRadius >= 30) {
            e.isBullet = true;
        }

        add(e);
        bolderRadius = 0;
    }

    if (IsKeyPressed((KEY_SPACE))) {
        updating = !updating;
    }

    if (IsKeyPressed(KEY_F3)) {
        debug++;

        if (debug > maxDebug) {
            debug = 0;
        }
    }

    if (IsWindowResized()) {
        initBuckets();
    }

    if (!updating) {
        return;
    }

    for (auto &b: buckets) {
        auto pBucket = &b;
        pBucket->second.elements.clear();
    }

    for (auto &pair: elements) {
        for (auto &e: pair.second) {
            auto pElement = &e;
            pElement->updateBucketIndex(cols, rows, bucketWidth, bucketHeight);

            for (auto &i: pElement->buckets) {
                buckets.at(i).elements.emplace_back(pElement);
            }
        }
    }

    updates = 0;

    for (int i = 0; i < threads; ++i) {
        auto portion = &elements.at(i);

        std::thread worker([&, portion]() {
//            LockGuard l(elementMutex);

            for (auto &e: *portion) {
                auto pElement = &e;
                std::vector<Entity *> nearby = {};

                for (auto &index: pElement->buckets) {
                    auto target = buckets.at(index);
                    nearby.insert(nearby.end(), target.elements.begin(), target.elements.end());
                }

                if (!pElement->isDeleted) {
                    pElement->update(nearby, screen, GetFrameTime() * 1000);
                }
            }

            updates++;
        });

        worker.detach();
    }

    while (updates.load() < threads) {
        usleep(100);
    }

    elementCount = 0;

    for (auto &b: elements) {
        auto it = b.second.begin();

        while (it != b.second.end()) {
            if (it->isDeleted) {
                it = b.second.erase(it);
            } else {
                it++;
                elementCount++;
            }
        }
    }

    updateMs = (GetTime() - start) * 1000;
}

void EntityManager::add(const Entity &e) {
    LockGuard l(elementMutex);

    elements.at(currentInsertIndex).emplace_back(e);
    currentInsertIndex++;

    if (currentInsertIndex >= threads) {
        currentInsertIndex = 0;
    }
}

void EntityManager::render() {
    auto start = GetTime();
    auto mouse = GetMousePosition();

    for (auto &pair: elements) {
        for (auto &e: pair.second) {
            auto pElement = &e;

            if (pElement->isDeleted) {
                continue;
            }

            Color color = e.isBullet ? MAGENTA : RAYWHITE;
            DrawCircle((int) pElement->position.x, (int) pElement->position.y, pElement->radius, color);

            if (debug >= 2) {
                std::string indexes;

                for (auto &b: pElement->buckets) {
                    indexes += std::to_string(b);
                }

                DrawText(indexes.c_str(), (int) pElement->position.x, (int) pElement->position.y, 16, GREEN);
            }
        }
    }

    DrawText(std::to_string(GetFPS()).c_str(), 20, 20, 20, GREEN);
    DrawText(std::format("Update = {:.2f}", updateMs).c_str(), 20, 40, 20, GREEN);
    DrawText(std::format("Render = {:.2f}", renderMs).c_str(), 20, 60, 20, GREEN);
    DrawText(std::format("Elements = {}", elementCount).c_str(), 20, 80, 20, GREEN);

    if (debug >= 1) {
        for (auto &b: buckets) {
            DrawText(std::to_string(b.second.elements.size()).c_str(), b.second.rect.x + 5, b.second.rect.y + 5, 20,
                     YELLOW);
            DrawRectangleLinesEx(b.second.rect, 1, WHITE);
        }
    }

    if (bolderRadius > 0) {
        DrawCircle(bolderStartDrag.x, bolderStartDrag.y, bolderRadius, MAGENTA);
        DrawLine(mouse.x, mouse.y, bolderStartDrag.x, bolderStartDrag.y, MAGENTA);
    }

    renderMs = (GetTime() - start) * 1000;
}

void EntityManager::addRandom() {
    Entity e;
    e.radius = (float) random(3, 5);
    e.direction = random(0, 360);
    e.position = {
            (float) random(0, (int) (pContainer->width - e.radius * 2)),
            (float) random(0, (int) (pContainer->height - e.radius * 2))
    };

    add(e);
}

void EntityManager::addAtPosition(const Vector2 &p, int direction) {
    Entity e;
    e.radius = (float) random(3, 5);
    e.direction = direction;
    e.position = Vec2(p.x, p.y);

    add(e);
}

