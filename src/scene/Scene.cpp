#include "Scene.h"
#include <unistd.h>
#include <string>

struct Vector3
{
    float x;
    float y;
    float z;
    static Vector3 zero()
    {
        return Vector3({0.0f, 0.0f, 0.0f});
    }
};

struct Quaternion
{
    float x;
    float y;
    float z;
    float w;
    static Quaternion identity()
    {
        return Quaternion({0.0f, 0.0f, 0.0f, 1.0f});
    }
};

enum class ComponentClassId : int32_t
{
    NONE = 0,
    TRANSFORM = 1,
    BOX_SHAPE = 16
};

uint32_t componentCounter = 0;
class Component
{
public:
    Component(ComponentClassId _classId) : classId(_classId), componentNumber(componentCounter++) {}

    std::string toJSON()
    {
        return "{\"classId\": " + std::to_string(static_cast<int32_t>(classId)) +
               ",\"data\":" + data() + "}";
    }

    virtual std::string data()
    {
        return "{}";
    }

    uint64_t getComponentNumber() { return componentNumber; }

protected:
    uint64_t componentNumber = 0;
    ComponentClassId classId = ComponentClassId::NONE;
};

class TransformComponent : public Component
{
public:
    TransformComponent() : Component(ComponentClassId::TRANSFORM)
    {
        position = Vector3::zero();
        rotation = Quaternion::identity();
        scale = Vector3({1.0f, 1.0f, 1.0f});
    }

    Vector3 position, scale;
    Quaternion rotation;

    std::string data()
    {
        return std::string() +
               "{\"rotation\":{" +
               "\"x\":" + std::to_string(rotation.x) +
               ",\"y\":" + std::to_string(rotation.y) +
               ",\"z\":" + std::to_string(rotation.z) +
               ",\"w\":" + std::to_string(rotation.w) +
               "},\"position\":{" +
               "\"x\":" + std::to_string(position.x) +
               ",\"y\":" + std::to_string(position.y) +
               ",\"z\":" + std::to_string(position.z) +
               "},\"scale\":{" +
               "\"x\":" + std::to_string(scale.x) +
               ",\"y\":" + std::to_string(scale.y) +
               ",\"z\":" + std::to_string(scale.z) +
               "}}";
    }
};

class BoxShape : public Component
{
public:
    BoxShape() : Component(ComponentClassId::BOX_SHAPE)
    {
    }

    std::string data()
    {
        return "{\"withCollisions\":true,\"isPointerBlocker\":true,\"visible\":true}";
    }
};

Scene::Scene()
{
    registry = std::make_unique<entt::registry>();


    // entity "0" rootEntity (reserved)
    const auto zeroEntity = registry->create();

    // unique entity
    const auto entity = registry->create();

    auto transform = registry->emplace<TransformComponent>(entity);
    transform.position = Vector3({8.0f, 1.0f, 8.0f});

    registry->emplace<BoxShape>(entity);
}

void Scene::update(float dt)
{
    counter += 5.0f * dt;

    auto view = registry->view<TransformComponent>();
    for (auto [entity, pos] : view.each())
    {
       // pos.position.x = 12.0f + cos(static_cast<double>(counter));
        pos.position.z = 8.0f + sin(static_cast<double>(counter));
    }
}

uint64_t timestamp = 0;

void sendPutComponentMessage(
    int rendererFd, int64_t entityId, int64_t componentNumber,
    void *data, uint32_t dataLength)
{
    char sceneMessage[1024];
    char *ptr = sceneMessage;
    int32_t messageType = 1;

    memcpy(ptr, (void *)&messageType, 4);
    ptr += 4;

    memcpy(ptr, (void *)&entityId, 8);
    ptr += 8;

    memcpy(ptr, (void *)&componentNumber, 8);
    ptr += 8;

    timestamp++;
    memcpy(ptr, (void *)&timestamp, 8);
    ptr += 8;

    memcpy(ptr, (void *)&dataLength, 4);
    ptr += 4;

    if (dataLength > 0)
    {
        memcpy(ptr, data, dataLength);
        ptr += dataLength;
    }
    write(rendererFd, sceneMessage, ptr - sceneMessage);
}

void Scene::sendUpdates(int rendererFd)
{

    auto view = registry->view<TransformComponent>();
    for (auto [entity, transform] : view.each())
    {
        std::string componentJson = transform.toJSON();
        // printf("%s\n", componentJson.c_str());
        sendPutComponentMessage(rendererFd,
                                (int64_t)entity, transform.getComponentNumber(),
                                (void *)componentJson.c_str(), componentJson.size());
    }

    static int bs = 0;
    if (bs == 0)
    {
        bs = 1;
        auto view2 = registry->view<BoxShape>();
        for (auto [entity, transform] : view2.each())
        {
            std::string componentJson = transform.toJSON();
            // printf("%s\n", componentJson.c_str());
            sendPutComponentMessage(rendererFd,
                                    (int64_t)entity, transform.getComponentNumber(),
                                    (void *)componentJson.c_str(), componentJson.size());
        }
    }
}