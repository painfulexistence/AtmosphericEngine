#pragma once
#include "globals.hpp"
#include "component.hpp"

class Drawable2D : public Component {
public:
    Drawable2D(GameObject* gameObject);
    // virtual ~Drawable2D() = default;

    std::string GetName() const override;

    glm::vec2 GetSize() { return _size; }
    glm::vec2 GetPivot() { return _pivot; }
    glm::vec4 GetColor() { return _color; }
    uint8_t GetTextureID() { return _textureID; }
    float GetRotation() { return _rotation; }

    void SetSize(const glm::vec2& size) { _size = size; }
    void SetPivot(const glm::vec2& pivot) { _pivot = pivot; }
    void SetColor(const glm::vec4& color) { _color = color; }
    void SetTextureID(uint8_t textureID) { _textureID = textureID; }
    void SetRotation(float rotation) { _rotation = rotation; }

private:
    glm::vec2 _size = glm::vec2(100.0f, 100.0f); // Base size in pixels
    glm::vec2 _pivot = glm::vec2(0.5f, 0.5f); // Pivot point (0,0 = top-left, 1,1 = bottom-right)
    glm::vec4 _color = glm::vec4(1.0f);
    uint8_t _textureID = 0;
    float _rotation = 0.0f; // Rotation in radians
};