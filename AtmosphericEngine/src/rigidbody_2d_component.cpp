#include "rigidbody_2d_component.hpp"
#include "game_object.hpp"
#include "transform_component.hpp"

Rigidbody2DComponent::Rigidbody2DComponent(GameObject* gameObject, const Rigidbody2DProps& props)
    : Component(gameObject), _props(props), _shapeDef(props.shape) {
}

Rigidbody2DComponent::~Rigidbody2DComponent() {
    DestroyBody();
}

void Rigidbody2DComponent::OnAttach() {
    CreateBody();
    Physics2DServer::Get()->RegisterRigidbody2D(this);
}

void Rigidbody2DComponent::OnDetach() {
    Physics2DServer::Get()->UnregisterRigidbody2D(this);
    DestroyBody();
}

void Rigidbody2DComponent::CreateBody() {
    auto* physics = Physics2DServer::Get();
    if (!physics || !physics->GetWorld()) return;

    // Get initial position from props or from transform
    glm::vec2 posPixels = _props.position;
    if (_gameObject) {
        auto* transform = _gameObject->GetTransformComponent();
        if (transform) {
            glm::vec3 pos = transform->GetPosition();
            posPixels = glm::vec2(pos.x, pos.y);
        }
    }

    // Create body definition
    b2BodyDef bodyDef;
    switch (_props.type) {
        case BodyType2D::Static:
            bodyDef.type = b2_staticBody;
            break;
        case BodyType2D::Kinematic:
            bodyDef.type = b2_kinematicBody;
            break;
        case BodyType2D::Dynamic:
        default:
            bodyDef.type = b2_dynamicBody;
            break;
    }

    glm::vec2 posMeters = Physics2DServer::PixelsToMeters(posPixels);
    bodyDef.position.Set(posMeters.x, posMeters.y);
    bodyDef.angle = _props.angle;
    bodyDef.linearDamping = _props.linearDamping;
    bodyDef.angularDamping = _props.angularDamping;
    bodyDef.gravityScale = _props.gravityScale;
    bodyDef.fixedRotation = _props.fixedRotation;
    bodyDef.bullet = _props.bullet;

    // Store pointer to this component for collision callbacks
    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(this);

    _body = physics->CreateBody(&bodyDef);

    if (_body) {
        CreateFixture();
    }
}

void Rigidbody2DComponent::CreateFixture() {
    if (!_body) return;

    b2FixtureDef fixtureDef;
    fixtureDef.density = _shapeDef.density;
    fixtureDef.friction = _shapeDef.friction;
    fixtureDef.restitution = _shapeDef.restitution;
    fixtureDef.isSensor = _shapeDef.isSensor;

    switch (_shapeDef.type) {
        case ShapeType2D::Box: {
            b2PolygonShape boxShape;
            glm::vec2 halfSize = Physics2DServer::PixelsToMeters(_shapeDef.boxSize * 0.5f);
            boxShape.SetAsBox(halfSize.x, halfSize.y);
            fixtureDef.shape = &boxShape;
            _body->CreateFixture(&fixtureDef);
            break;
        }
        case ShapeType2D::Circle: {
            b2CircleShape circleShape;
            circleShape.m_radius = Physics2DServer::PixelsToMeters(_shapeDef.circleRadius);
            fixtureDef.shape = &circleShape;
            _body->CreateFixture(&fixtureDef);
            break;
        }
        case ShapeType2D::Polygon: {
            if (_shapeDef.polygonVertices.size() >= 3 && _shapeDef.polygonVertices.size() <= b2_maxPolygonVertices) {
                b2PolygonShape polygonShape;
                std::vector<b2Vec2> vertices;
                vertices.reserve(_shapeDef.polygonVertices.size());

                for (const auto& v : _shapeDef.polygonVertices) {
                    glm::vec2 vMeters = Physics2DServer::PixelsToMeters(v);
                    vertices.push_back(b2Vec2(vMeters.x, vMeters.y));
                }

                polygonShape.Set(vertices.data(), static_cast<int32>(vertices.size()));
                fixtureDef.shape = &polygonShape;
                _body->CreateFixture(&fixtureDef);
            }
            break;
        }
    }
}

void Rigidbody2DComponent::DestroyBody() {
    if (_body) {
        auto* physics = Physics2DServer::Get();
        if (physics) {
            physics->DestroyBody(_body);
        }
        _body = nullptr;
    }
}

void Rigidbody2DComponent::Tick(float dt) {
    if (!_body || !_gameObject) return;

    // Sync Box2D body position/rotation to GameObject transform
    auto* transform = _gameObject->GetTransformComponent();
    if (transform) {
        b2Vec2 pos = _body->GetPosition();
        glm::vec2 posPixels = Physics2DServer::MetersToPixels(glm::vec2(pos.x, pos.y));

        glm::vec3 newPos(posPixels.x, posPixels.y, transform->GetPosition().z);
        transform->SetPosition(newPos);

        float angle = _body->GetAngle();
        glm::vec3 rotation = transform->GetRotation();
        rotation.z = glm::degrees(angle);
        transform->SetRotation(rotation);
    }
}

glm::vec2 Rigidbody2DComponent::GetPosition() const {
    if (_body) {
        b2Vec2 pos = _body->GetPosition();
        return Physics2DServer::MetersToPixels(glm::vec2(pos.x, pos.y));
    }
    return glm::vec2(0.0f);
}

void Rigidbody2DComponent::SetPosition(const glm::vec2& position) {
    if (_body) {
        glm::vec2 posM = Physics2DServer::PixelsToMeters(position);
        _body->SetTransform(b2Vec2(posM.x, posM.y), _body->GetAngle());
    }
}

float Rigidbody2DComponent::GetAngle() const {
    if (_body) {
        return _body->GetAngle();
    }
    return 0.0f;
}

void Rigidbody2DComponent::SetAngle(float angle) {
    if (_body) {
        _body->SetTransform(_body->GetPosition(), angle);
    }
}

glm::vec2 Rigidbody2DComponent::GetLinearVelocity() const {
    if (_body) {
        b2Vec2 vel = _body->GetLinearVelocity();
        return Physics2DServer::MetersToPixels(glm::vec2(vel.x, vel.y));
    }
    return glm::vec2(0.0f);
}

void Rigidbody2DComponent::SetLinearVelocity(const glm::vec2& velocity) {
    if (_body) {
        glm::vec2 velM = Physics2DServer::PixelsToMeters(velocity);
        _body->SetLinearVelocity(b2Vec2(velM.x, velM.y));
    }
}

float Rigidbody2DComponent::GetAngularVelocity() const {
    if (_body) {
        return _body->GetAngularVelocity();
    }
    return 0.0f;
}

void Rigidbody2DComponent::SetAngularVelocity(float omega) {
    if (_body) {
        _body->SetAngularVelocity(omega);
    }
}

void Rigidbody2DComponent::ApplyForce(const glm::vec2& force, const glm::vec2& point) {
    if (_body) {
        glm::vec2 forceM = Physics2DServer::PixelsToMeters(force);
        glm::vec2 pointM = Physics2DServer::PixelsToMeters(point);
        _body->ApplyForce(b2Vec2(forceM.x, forceM.y), b2Vec2(pointM.x, pointM.y), true);
    }
}

void Rigidbody2DComponent::ApplyForceToCenter(const glm::vec2& force) {
    if (_body) {
        glm::vec2 forceM = Physics2DServer::PixelsToMeters(force);
        _body->ApplyForceToCenter(b2Vec2(forceM.x, forceM.y), true);
    }
}

void Rigidbody2DComponent::ApplyTorque(float torque) {
    if (_body) {
        _body->ApplyTorque(torque, true);
    }
}

void Rigidbody2DComponent::ApplyLinearImpulse(const glm::vec2& impulse, const glm::vec2& point) {
    if (_body) {
        glm::vec2 impulseM = Physics2DServer::PixelsToMeters(impulse);
        glm::vec2 pointM = Physics2DServer::PixelsToMeters(point);
        _body->ApplyLinearImpulse(b2Vec2(impulseM.x, impulseM.y), b2Vec2(pointM.x, pointM.y), true);
    }
}

void Rigidbody2DComponent::ApplyLinearImpulseToCenter(const glm::vec2& impulse) {
    if (_body) {
        glm::vec2 impulseM = Physics2DServer::PixelsToMeters(impulse);
        _body->ApplyLinearImpulse(b2Vec2(impulseM.x, impulseM.y), _body->GetWorldCenter(), true);
    }
}

void Rigidbody2DComponent::ApplyAngularImpulse(float impulse) {
    if (_body) {
        _body->ApplyAngularImpulse(impulse, true);
    }
}

BodyType2D Rigidbody2DComponent::GetBodyType() const {
    if (_body) {
        switch (_body->GetType()) {
            case b2_staticBody:
                return BodyType2D::Static;
            case b2_kinematicBody:
                return BodyType2D::Kinematic;
            case b2_dynamicBody:
            default:
                return BodyType2D::Dynamic;
        }
    }
    return _props.type;
}

void Rigidbody2DComponent::SetBodyType(BodyType2D type) {
    if (_body) {
        switch (type) {
            case BodyType2D::Static:
                _body->SetType(b2_staticBody);
                break;
            case BodyType2D::Kinematic:
                _body->SetType(b2_kinematicBody);
                break;
            case BodyType2D::Dynamic:
                _body->SetType(b2_dynamicBody);
                break;
        }
    }
}

void Rigidbody2DComponent::SetGravityScale(float scale) {
    if (_body) {
        _body->SetGravityScale(scale);
    }
}

float Rigidbody2DComponent::GetGravityScale() const {
    if (_body) {
        return _body->GetGravityScale();
    }
    return _props.gravityScale;
}

void Rigidbody2DComponent::SetFixedRotation(bool fixed) {
    if (_body) {
        _body->SetFixedRotation(fixed);
    }
}

bool Rigidbody2DComponent::IsFixedRotation() const {
    if (_body) {
        return _body->IsFixedRotation();
    }
    return _props.fixedRotation;
}

float Rigidbody2DComponent::GetMass() const {
    if (_body) {
        return _body->GetMass();
    }
    return 0.0f;
}
