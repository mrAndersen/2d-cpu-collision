#include <ServiceContainer.h>
#include <EntityManager.h>
#include <raylib.h>

int main() {
    auto container = ServiceContainer::build();

    for (int i = 0; i < 8096; ++i) {
        container->pEntityManager->addRandom();
    }


    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            container->width = GetScreenWidth();
            container->height = GetScreenHeight();
        }

        container->pEntityManager->update();

        BeginDrawing();
        ClearBackground(BLACK);
        container->pEntityManager->render();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
