#pragma once

#include <raylib.h>
#include <cmath>
#include <global.h>

class Entity {
public:
    int bucket = 0;

    float speed = 0.25;

    float radius = 5.0;

    float acceleration = 5.0;

    bool isBullet = false;

    bool isDeleted = false;

    int direction = 0;

    Vec2 position = {};

    std::vector<int> buckets = {};

    Entity() {
        this->buckets.reserve(4);
    }

    [[nodiscard]] Rectangle getAsRect() const {
        return {position.x - radius, position.y - radius, radius * 2, radius * 2};
    }

    [[nodiscard]] Vec2 getVelocity() const {
        float radian = direction * M_PI / 180.0;
        return {std::cos(radian) * speed, std::sin(radian) * speed};
    }

    void setFromVelocity(const Vec2 &velocity) {
        speed = velocity.magnitude();
        direction = std::atan2(velocity.y, velocity.x) * 180.0 / M_PI;
    }

    int getRandomOppositeDirection() const {
        auto target = direction + 180;

        if (random(0, 1) == 0) {
            target += random(0, 30);
        } else {
            target -= random(0, 30);
        }

        return target % 360;
    }

    int getOppositeDirection() {
        auto target = direction + 180;
        return target % 360;
    }

    void move(float distance, int someDirection) {
        position.x += distance * std::cos((float) someDirection * PI / 180);
        position.y += distance * std::sin((float) someDirection * PI / 180);
    }

    void updateBucketIndex(int cols, int rows, int bucketWidth, int bucketHeight) {
        buckets.clear();

        int startCol = std::fmax(0, (position.x - radius) / bucketWidth);
        int endCol = std::fmin(cols - 1, (position.x + radius) / bucketWidth);
        int startRow = std::fmax(0, (position.y - radius) / bucketHeight);
        int endRow = std::fmin(rows - 1, (position.y + radius) / bucketHeight);

        for (int row = startRow; row <= endRow; ++row) {
            for (int col = startCol; col <= endCol; ++col) {
                buckets.push_back(row * cols + col);
            }
        }
    }

    void update(const std::vector<Entity *> &nearby, Rectangle screen, float dt) {
        float restitution = 0.5f; // Adjust as needed

        move(dt * speed, direction);

        {
            //collision
            if (!nearby.empty()) {
                for (auto &target: nearby) {
                    if (target == this) {
                        continue;
                    }

                    float min = radius + target->radius;
                    float dx = position.x - target->position.x;
                    float dy = position.y - target->position.y;
                    float current = sqrtf(dx * dx + dy * dy);

                    if (current < min) {
                        if (isBullet) {
                            target->isDeleted = true;
                            continue;
                        }

                        auto collisionVector = position - target->position;
                        collisionVector = collisionVector.normalize();

                        auto relativeVelocity = getVelocity() - target->getVelocity();
                        float velocityAlongCollision =
                                relativeVelocity.x * collisionVector.x + relativeVelocity.y * collisionVector.y;

                        if (velocityAlongCollision > 0) {
                            return;
                        }

                        float impulseScalar =
                                (-(1 + restitution) * velocityAlongCollision) / (1 / radius + 1 / target->radius);

                        auto newVelocity1 = getVelocity() + (collisionVector * impulseScalar * (1 / radius));
                        auto newVelocity2 =
                                target->getVelocity() - (collisionVector * impulseScalar * (1 / target->radius));


                        setFromVelocity(newVelocity1);
                        target->setFromVelocity(newVelocity2);
                    }
                }
            }
        }

        {
            //backtrack
            if (
                    (position.x + radius >= screen.width || position.x - radius < 0) ||
                    (position.y + radius >= screen.height || position.y - radius < 0)
                    ) {
                direction = getRandomOppositeDirection();
            }
        }


        {
            //correction
            if (position.x + radius > screen.width) {
                position.x = screen.width - radius;
            }

            if (position.x - radius < 0) {
                position.x = radius;
            }

            if (position.y + radius > screen.height) {
                position.y = screen.height - radius;
            }

            if (position.y - radius < 0) {
                position.y = radius;
            }
        }

    }
};
