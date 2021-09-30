#ifndef _SCENE_H
#define _SCENE_H

#include "entt/entt.hpp"

class Scene {
public:
    Scene();
    void update(float dt);
    void sendUpdates(int rendererFd);

protected:
    std::unique_ptr<entt::registry> registry;
    float counter = 0.0f;
};

#endif