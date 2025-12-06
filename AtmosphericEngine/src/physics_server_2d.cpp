#include "physics_server_2d.hpp"
#include "batch_renderer_2d.hpp"
#include "console.hpp"
#include "game_object.hpp"
#include "renderer.hpp"
#include "rigidbody_2d_component.hpp"
#include <algorithm>
#include <imgui.h>

Physics2DServer* Physics2DServer::_instance = nullptr;

// Contact Listener Implementation
void Physics2DContactListener::BeginContact(b2Contact* contact) {
    if (onBeginContact) {
        void* userDataA = contact->GetFixtureA()->GetBody()->GetUserData().pointer
                            ? reinterpret_cast<void*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer)
                            : nullptr;
        void* userDataB = contact->GetFixtureB()->GetBody()->GetUserData().pointer
                            ? reinterpret_cast<void*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer)
                            : nullptr;

        if (userDataA && userDataB) {
            auto* rbA = static_cast<Rigidbody2DComponent*>(userDataA);
            auto* rbB = static_cast<Rigidbody2DComponent*>(userDataB);
            onBeginContact(rbA, rbB);
        }
    }
}

void Physics2DContactListener::EndContact(b2Contact* contact) {
    if (onEndContact) {
        void* userDataA = contact->GetFixtureA()->GetBody()->GetUserData().pointer
                            ? reinterpret_cast<void*>(contact->GetFixtureA()->GetBody()->GetUserData().pointer)
                            : nullptr;
        void* userDataB = contact->GetFixtureB()->GetBody()->GetUserData().pointer
                            ? reinterpret_cast<void*>(contact->GetFixtureB()->GetBody()->GetUserData().pointer)
                            : nullptr;

        if (userDataA && userDataB) {
            auto* rbA = static_cast<Rigidbody2DComponent*>(userDataA);
            auto* rbB = static_cast<Rigidbody2DComponent*>(userDataB);
            onEndContact(rbA, rbB);
        }
    }
}

// Physics2DServer Implementation
Physics2DServer::Physics2DServer() {
    _instance = this;
}

Physics2DServer::~Physics2DServer() {
    if (_instance == this) {
        _instance = nullptr;
    }
}

void Physics2DServer::Init(Application* app) {
    Server::Init(app);

    // Create Box2D world with default gravity (0, 9.8 m/s^2 downward in screen space)
    b2Vec2 gravity(0.0f, 9.8f);
    _world = std::make_unique<b2World>(gravity);

    // Set contact listener
    _world->SetContactListener(&_contactListener);

    Console::Get()->Info("Physics2DServer initialized with Box2D");
}

void Physics2DServer::Process(float dt) {
    if (!_world) return;

    // Step the physics simulation
    _world->Step(dt, _velocityIterations, _positionIterations);

    // Sync transforms
    for (auto* rb : _rigidbodies) {
        rb->SyncToTransform(dt);
    }
}

void Physics2DServer::DrawImGui(float dt) {
    if (ImGui::CollapsingHeader("Physics 2D")) {
        ImGui::Text("Bodies: %d", (int)_rigidbodies.size());
        ImGui::Checkbox("Debug Draw", &_debugDrawEnabled);

        glm::vec2 gravity = GetGravity();
        if (ImGui::DragFloat2("Gravity", &gravity.x, 0.1f)) {
            SetGravity(gravity);
        }

        ImGui::SliderInt("Velocity Iterations", &_velocityIterations, 1, 20);
        ImGui::SliderInt("Position Iterations", &_positionIterations, 1, 20);
    }
}

void Physics2DServer::SetGravity(const glm::vec2& gravity) {
    if (_world) {
        _world->SetGravity(b2Vec2(gravity.x, gravity.y));
    }
}

glm::vec2 Physics2DServer::GetGravity() const {
    if (_world) {
        b2Vec2 g = _world->GetGravity();
        return glm::vec2(g.x, g.y);
    }
    return glm::vec2(0.0f);
}

b2Body* Physics2DServer::CreateBody(const b2BodyDef* def) {
    if (_world) {
        return _world->CreateBody(def);
    }
    return nullptr;
}

void Physics2DServer::DestroyBody(b2Body* body) {
    if (_world && body) {
        _world->DestroyBody(body);
    }
}

Rigidbody2DComponent* Physics2DServer::RegisterRigidbody2D(Rigidbody2DComponent* rb) {
    if (rb) {
        _rigidbodies.push_back(rb);
    }
    return rb;
}

void Physics2DServer::UnregisterRigidbody2D(Rigidbody2DComponent* rb) {
    auto it = std::find(_rigidbodies.begin(), _rigidbodies.end(), rb);
    if (it != _rigidbodies.end()) {
        _rigidbodies.erase(it);
    }
}

void Physics2DServer::SetBeginContactCallback(CollisionCallback callback) {
    _contactListener.onBeginContact = callback;
}

void Physics2DServer::SetEndContactCallback(CollisionCallback callback) {
    _contactListener.onEndContact = callback;
}

// Raycast callback
class RaycastCallback : public b2RayCastCallback {
public:
    Physics2DServer::RaycastResult result;

    float ReportFixture(b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, float fraction) override {
        result.hit = true;
        result.point = Physics2DServer::MetersToPixels(glm::vec2(point.x, point.y));
        result.normal = glm::vec2(normal.x, normal.y);
        result.fraction = fraction;

        void* userData = fixture->GetBody()->GetUserData().pointer
                           ? reinterpret_cast<void*>(fixture->GetBody()->GetUserData().pointer)
                           : nullptr;
        if (userData) {
            result.body = static_cast<Rigidbody2DComponent*>(userData);
        }

        return fraction;// Continue to find closest
    }
};

Physics2DServer::RaycastResult
  Physics2DServer::Raycast(const glm::vec2& origin, const glm::vec2& direction, float maxDistance) {
    RaycastResult result;
    if (!_world) return result;

    glm::vec2 originM = PixelsToMeters(origin);
    glm::vec2 endM = PixelsToMeters(origin + direction * maxDistance);

    RaycastCallback callback;
    _world->RayCast(&callback, b2Vec2(originM.x, originM.y), b2Vec2(endM.x, endM.y));

    return callback.result;
}

// AABB Query callback
class AABBQueryCallback : public b2QueryCallback {
public:
    std::vector<Rigidbody2DComponent*> results;

    bool ReportFixture(b2Fixture* fixture) override {
        void* userData = fixture->GetBody()->GetUserData().pointer
                           ? reinterpret_cast<void*>(fixture->GetBody()->GetUserData().pointer)
                           : nullptr;
        if (userData) {
            results.push_back(static_cast<Rigidbody2DComponent*>(userData));
        }
        return true;// Continue query
    }
};

std::vector<Rigidbody2DComponent*>
  Physics2DServer::QueryAABB(const glm::vec2& lowerBound, const glm::vec2& upperBound) {
    std::vector<Rigidbody2DComponent*> results;
    if (!_world) return results;

    glm::vec2 lowerM = PixelsToMeters(lowerBound);
    glm::vec2 upperM = PixelsToMeters(upperBound);

    b2AABB aabb;
    aabb.lowerBound = b2Vec2(lowerM.x, lowerM.y);
    aabb.upperBound = b2Vec2(upperM.x, upperM.y);

    AABBQueryCallback callback;
    _world->QueryAABB(&callback, aabb);

    return callback.results;
}
