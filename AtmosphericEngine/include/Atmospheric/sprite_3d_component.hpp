#pragma once
#include "canvas_drawable.hpp"
#include "component.hpp"
#include "globals.hpp"
#include <glm/vec2.hpp>
#include <glm/vec4.hpp>

struct Sprite3DProps {
    glm::vec2 size = glm::vec2(1.0f, 1.0f);// Size in world units
    glm::vec2 pivot = glm::vec2(0.5f, 0.5f);// (0,0) = bottom-left, (1,1) = top-right
    glm::vec4 color = glm::vec4(1.0f);
    int textureID = -1;
    BillboardMode billboardMode = BillboardMode::ViewPlane;// Default: face camera (Y-locked)
};

// Sprite3DComponent - A sprite rendered in 3D world space
// Used for health bars, name labels, damage numbers in 3D games
// Supports billboard modes to face the camera
class Sprite3DComponent : public CanvasDrawable {
public:
    Sprite3DComponent(GameObject* gameObject, const Sprite3DProps& props);

    std::string GetName() const override;

    void OnAttach() override;
    void OnDetach() override;

    void Draw(BatchRenderer2D* renderer) override;

    bool CanTick() const override {
        return false;
    }

    // Getters
    glm::vec2 GetSize() const { return _size; }
    glm::vec2 GetPivot() const { return _pivot; }
    glm::vec4 GetColor() const { return _color; }
    int GetTextureID() const { return _textureID; }
    BillboardMode GetBillboardMode() const { return _billboardMode; }

    // Setters (fluent style)
    Sprite3DComponent& SetSize(const glm::vec2& size) { _size = size; return *this; }
    Sprite3DComponent& SetSize(float w, float h) { _size = glm::vec2(w, h); return *this; }
    Sprite3DComponent& SetPivot(const glm::vec2& pivot) { _pivot = pivot; return *this; }
    Sprite3DComponent& SetColor(const glm::vec4& color) { _color = color; return *this; }
    Sprite3DComponent& SetTextureID(int textureID) { _textureID = textureID; return *this; }
    Sprite3DComponent& SetBillboardMode(BillboardMode mode) { _billboardMode = mode; return *this; }

    // UV for spritesheets
    glm::vec2 GetUVMin() const { return _uvMin; }
    glm::vec2 GetUVMax() const { return _uvMax; }
    Sprite3DComponent& SetUVs(const glm::vec2& min, const glm::vec2& max) {
        _uvMin = min;
        _uvMax = max;
        return *this;
    }

    CanvasLayer GetLayer() const override {
        return CanvasLayer::LAYER_WORLD_3D;
    }

private:
    glm::vec2 _size;
    glm::vec2 _pivot;
    glm::vec4 _color;
    int _textureID;
    BillboardMode _billboardMode;
    glm::vec2 _uvMin = glm::vec2(0.0f, 0.0f);
    glm::vec2 _uvMax = glm::vec2(1.0f, 1.0f);

    glm::mat4 CalculateBillboardMatrix(const glm::vec3& position, const glm::vec3& cameraPosition) const;
};
