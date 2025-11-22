#pragma once
#include "globals.hpp"
#include "component.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Drawable2DProps {
    glm::vec2 size = glm::vec2(100.0f, 100.0f);
    glm::vec2 pivot = glm::vec2(0.5f, 0.5f);
    glm::vec4 color = glm::vec4(1.0f);
    uint8_t textureID = 0;
};

class Drawable2D : public Component {
public:
    Drawable2D(GameObject* gameObject, const Drawable2DProps& props);
    // virtual ~Drawable2D() = default;

    std::string GetName() const override;

    glm::vec2 GetSize() { return _size; }
    glm::vec2 GetPivot() { return _pivot; }
    glm::vec4 GetColor() { return _color; }
    uint8_t GetTextureID() { return _textureID; }

    void SetSize(const glm::vec2& size) { _size = size; }
    void SetPivot(const glm::vec2& pivot) { _pivot = pivot; }
    void SetColor(const glm::vec4& color) { _color = color; }
    void SetTextureID(uint8_t textureID) { _textureID = textureID; }

private:
    glm::vec2 _size; // Base size in pixels
    glm::vec2 _pivot; // Pivot point (0,0 = top-left, 1,1 = bottom-right)
    glm::vec4 _color;
    uint8_t _textureID;
};