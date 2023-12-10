#pragma once

class EntityManager;

class ServiceContainer {

public:
    EntityManager *pEntityManager;

    int width = 1920;

    int height = 1080;

    static ServiceContainer *build();
};
