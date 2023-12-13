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

            b.rect = Rectangle({(float) i, (float) j, (float) bucketWidth, (float) bucketHeight});
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

    Rectangle screen({0, 0, (float) pContainer->width, (float) pContainer->height});

    if (IsKeyPressed(KEY_KP_6)) {
        elasticity += 0.1;
    }

    if (IsKeyPressed(KEY_KP_3)) {
        elasticity -= 0.1;
    }

    if (elasticity <= 0) {
        elasticity = 0.1;
    }

    if (elasticity >= 1) {
        elasticity = 1;
    }

    if (IsKeyPressed(KEY_KP_ADD)) {
        for (int i = 0; i < 1024; ++i) {
            addRandom();
        }
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT) || IsMouseButtonDown(MOUSE_BUTTON_RIGHT)) {
        if (bolderRadius == 0) {
            bolderStartDrag = mouse;
            bolderType = IsMouseButtonDown(MOUSE_BUTTON_LEFT) ? 0 : 1;
        }

        bolderRadius += dt * 0.1;
    }

    if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT) || IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
        auto power = distance({mouse.x, mouse.y}, {bolderStartDrag.x, bolderStartDrag.y});
        auto angle = std::atan2(mouse.y - bolderStartDrag.y, mouse.x - bolderStartDrag.x) * (180 / PI);
        angle = ((int) angle + 180) % 360;

        Entity e;
        e.position = {bolderStartDrag.x, bolderStartDrag.y};
        e.radius = bolderRadius;
        e.direction = (int) angle;
        e.speed = power / 20;

        if (IsMouseButtonReleased(MOUSE_BUTTON_RIGHT)) {
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

    if (IsKeyPressed(KEY_F4)) {
        drawType++;

        if (drawType == 2) {
            drawType = 0;
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

    elementCount = 0;

    for (auto &b: elements) {
        auto it = b.second.begin();

        while (it != b.second.end()) {
            for (auto &i: it->buckets) {
                buckets[i].elements.emplace_back(&(*it));
            }

            if (it->isDeleted) {
                it = b.second.erase(it);
            } else {
                it++;
                elementCount++;
            }
        }
    }

    updates = 0;
    threadThrottle = 0;

    for (int i = 0; i < threads; ++i) {
        std::thread worker([&, i]() {
            auto portion = &elements.at(i);

            for (auto &e: *portion) {
                auto pElement = &e;
                std::vector<Entity *> nearby = {};

                for (auto &index: pElement->buckets) {
                    auto target = buckets.at(index);
                    nearby.insert(nearby.end(), target.elements.begin(), target.elements.end());
                }

                if (!pElement->isDeleted) {
                    pElement->update(elasticity, nearby, screen, GetFrameTime() * 1000);
                    pElement->updateBucketIndex(cols, rows, bucketWidth, bucketHeight);
                }
            }

            updates++;
        });

        worker.detach();
    }

    while (updates.load() < threads) {
        timespec t = {};
        t.tv_sec = 0;
        t.tv_nsec = 5000;
        nanosleep(&t, nullptr);
        threadThrottle += 5;
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
    auto bolderColor = bolderType == 0 ? RAYWHITE : RED;

    for (auto &pair: elements) {
        for (auto &e: pair.second) {
            auto pElement = &e;

            if (pElement->isDeleted) {
                continue;
            }

            Color color = e.isBullet ? bolderColor : RAYWHITE;

            if (drawType == 0) {
                DrawRectangle(
                        (int) (pElement->position.x - pElement->radius / 2),
                        (int) (pElement->position.y - pElement->radius / 2),
                        (int) pElement->radius,
                        (int) pElement->radius,
                        color
                );
            }

            if (drawType == 1) {
                DrawCircle((int) pElement->position.x, (int) pElement->position.y, pElement->radius, color);
            }

            if (debug >= 2) {
                std::string indexes;

                for (auto &b: pElement->buckets) {
                    indexes += std::to_string(b);
                }

                DrawText(indexes.c_str(), (int) pElement->position.x, (int) pElement->position.y, 16, GREEN);
            }
        }
    }

    DrawRectangle(10, 10, 400, 200, Color({0, 0, 0, 200}));
    currentDebugLineIndex = 0;

    renderDebugLine(std::format("FPS = {}", GetFPS()));
    renderDebugLine(std::format("UPS = {:.0f}", (1000.0f / updateMs)));
    renderDebugLine(std::format("Elements = {}", elementCount));
    renderDebugLine(std::format("Update = {:.2f}", updateMs));
    renderDebugLine(std::format("Render = {:.2f}", renderMs));
    renderDebugLine(std::format("Throttle = {}", threadThrottle));
    renderDebugLine(std::format("Threads = {}", threads));
    renderDebugLine(std::format("Elasticity = {}", elasticity));

    if (debug >= 1) {
        for (auto &b: buckets) {
            DrawText(std::to_string(b.second.elements.size()).c_str(), b.second.rect.x + 5, b.second.rect.y + 5, 20,
                     YELLOW);
            DrawRectangleLinesEx(b.second.rect, 1, WHITE);
        }
    }

    if (bolderRadius > 0) {
        if (drawType == 0) {
            DrawRectangle(
                    (int) (bolderStartDrag.x - bolderRadius / 2),
                    (int) (bolderStartDrag.y - bolderRadius / 2),
                    (int) bolderRadius,
                    (int) bolderRadius,
                    bolderColor
            );
        }

        if (drawType == 1) {
            DrawCircle(
                    (int) bolderStartDrag.x,
                    (int) bolderStartDrag.y,
                    bolderRadius,
                    bolderColor
            );
        }

        DrawLine(
                (int) mouse.x, +
                        (int) mouse.y,
                (int) bolderStartDrag.x,
                (int) bolderStartDrag.y,
                bolderColor
        );
    }

    renderMs = (GetTime() - start) * 1000;
}

void EntityManager::renderDebugLine(const std::string &text) {
    currentDebugLineIndex++;
    DrawText(text.c_str(), 20, currentDebugLineIndex * 20, 20, ORANGE);
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
    e.position = Vec2({p.x, p.y});

    add(e);
}

