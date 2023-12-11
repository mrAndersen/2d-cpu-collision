#include "ServiceContainer.h"
#include <EntityManager.h>
#include <raylib.h>

ServiceContainer *ServiceContainer::build() {
    auto container = new ServiceContainer();
    container->pEntityManager = new EntityManager(container);

    SetTraceLogLevel(LOG_NONE);
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    SetConfigFlags(FLAG_MSAA_4X_HINT);

    InitWindow(container->width, container->height, "2d-collision");
    SetTargetFPS(120);

    return container;
}