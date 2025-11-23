#pragma once
#include "layer.hpp"
#include <glad/glad.h>

// Forward declarations
class Application;
class GameObject;
class RigidbodyComponent;
class LightComponent;
class CameraComponent;
class MeshComponent;
class SpriteComponent;

class EditorLayer : public Layer {
public:
    EditorLayer(Application* app);
    ~EditorLayer() = default;

    void OnAttach() override {
    }
    void OnDetach() override {
    }
    void OnUpdate(float dt) override {
    }
    void OnRender(float dt) override;

private:
    Application* _app;
    bool _showSystemInfo = false;
    bool _showAppView = true;
    bool _showEngineView = true;
    GameObject* _selectedEntity = nullptr;

    void DrawSystemInfo();
    void DrawAppView();
    void DrawEntityInspector(GameObject* entity);
    void DrawEngineView();
};
