#pragma once
#include "component.hpp"
#include "globals.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct SpriteProps {
    glm::vec2 size = glm::vec2(100.0f, 100.0f);
    glm::vec2 pivot = glm::vec2(0.5f, 0.5f);
    glm::vec4 color = glm::vec4(1.0f);
    uint8_t textureID = 0;
};

class SpriteComponent : public Component {
public:
    SpriteComponent(GameObject* gameObject, const SpriteProps& props);
    // virtual ~SpriteComponent() = default;

    std::string GetName() const override;

    void OnAttach() override;
    void OnDetach() override;

    bool CanTick() const override {
        return false;
    }

    glm::vec2 GetSize() {
        return _size;
    }
    glm::vec2 GetPivot() {
        return _pivot;
    }
    glm::vec4 GetColor() {
        return _color;
    }
    uint8_t GetTextureID() {
        return _textureID;
    }

    void SetSize(const glm::vec2& size) {
        _size = size;
    }
    void SetPivot(const glm::vec2& pivot) {
        _pivot = pivot;
    }
    void SetColor(const glm::vec4& color) {
        _color = color;
    }
    void SetTextureID(uint8_t textureID) {
        _textureID = textureID;
    }

private:
    glm::vec2 _size;// Base size in pixels
    glm::vec2 _pivot;// Pivot point (0,0 = top-left, 1,1 = bottom-right)
    glm::vec4 _color;
    uint8_t _textureID;
};