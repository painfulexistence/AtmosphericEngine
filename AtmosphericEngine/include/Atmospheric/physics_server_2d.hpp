#pragma once

#include "server.hpp"
#include <box2d/box2d.h>
#include <functional>
#include <glm/vec2.hpp>
#include <memory>
#include <unordered_map>
#include <vector>

class Rigidbody2DComponent;
class GameObject;

// Collision callback types
using CollisionCallback = std::function<void(Rigidbody2DComponent*, Rigidbody2DComponent*)>;

// Contact listener for collision callbacks
class Physics2DContactListener : public b2ContactListener {
public:
    void BeginContact(b2Contact* contact) override;
    void EndContact(b2Contact* contact) override;

    CollisionCallback onBeginContact;
    CollisionCallback onEndContact;
};

// 2D Physics Server - manages Box2D world and all 2D physics bodies
class Physics2DServer : public Server {
private:
    static Physics2DServer* _instance;

public:
    static Physics2DServer* Get() {
        return _instance;
    }

    Physics2DServer();
    ~Physics2DServer();

    void Init(Application* app) override;
    void Process(float dt) override;
    void DrawImGui(float dt) override;

    // World management
    void SetGravity(const glm::vec2& gravity);
    glm::vec2 GetGravity() const;

    // Body management
    b2Body* CreateBody(const b2BodyDef* def);
    void DestroyBody(b2Body* body);

    // Registration
    Rigidbody2DComponent* RegisterRigidbody2D(Rigidbody2DComponent* rb);
    void UnregisterRigidbody2D(Rigidbody2DComponent* rb);

    // Callbacks
    void SetBeginContactCallback(CollisionCallback callback);
    void SetEndContactCallback(CollisionCallback callback);

    // Debug drawing
    void SetDebugDraw(bool enabled) {
        _debugDrawEnabled = enabled;
    }
    bool IsDebugDrawEnabled() const {
        return _debugDrawEnabled;
    }

    // Raycast
    struct RaycastResult {
        bool hit = false;
        glm::vec2 point;
        glm::vec2 normal;
        Rigidbody2DComponent* body = nullptr;
        float fraction = 1.0f;
    };
    RaycastResult Raycast(const glm::vec2& origin, const glm::vec2& direction, float maxDistance);

    // Query AABB
    std::vector<Rigidbody2DComponent*> QueryAABB(const glm::vec2& lowerBound, const glm::vec2& upperBound);

    // Pixel to meter conversion (Box2D uses meters)
    static constexpr float PIXELS_PER_METER = 100.0f;
    static float PixelsToMeters(float pixels) {
        return pixels / PIXELS_PER_METER;
    }
    static float MetersToPixels(float meters) {
        return meters * PIXELS_PER_METER;
    }
    static glm::vec2 PixelsToMeters(const glm::vec2& pixels) {
        return pixels / PIXELS_PER_METER;
    }
    static glm::vec2 MetersToPixels(const glm::vec2& meters) {
        return meters * PIXELS_PER_METER;
    }

    b2World* GetWorld() {
        return _world.get();
    }

private:
    std::unique_ptr<b2World> _world;
    Physics2DContactListener _contactListener;
    std::vector<Rigidbody2DComponent*> _rigidbodies;
    bool _debugDrawEnabled = false;

    // Simulation parameters
    int32 _velocityIterations = 8;
    int32 _positionIterations = 3;
};
