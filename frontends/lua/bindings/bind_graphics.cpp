#include "../lua_application.hpp"
#include "Atmospheric/renderer.hpp"

// Keep track of current draw color
static glm::vec4 s_CurrentColor = glm::vec4(1.0f);

void BindGraphicsAPI(sol::state& lua, GraphicsServer* graphics)
{
    sol::table atmos = lua["atmos"];
    sol::table gfx = atmos.create("graphics");

    // Get main camera
    gfx["getMainCamera"] = [graphics]() -> CameraComponent* {
        return graphics->GetMainCamera();
    };

    // Get main light
    gfx["getMainLight"] = [graphics]() -> LightComponent* {
        return graphics->GetMainLight();
    };

    // Screen dimensions (via Window)
    gfx["getScreenSize"] = []() -> std::tuple<int, int> {
        auto size = Window::Get()->GetSize();
        return std::make_tuple(size.width, size.height);
    };

    gfx["getWidth"] = []() {
        return Window::Get()->GetSize().width;
    };

    gfx["getHeight"] = []() {
        return Window::Get()->GetSize().height;
    };

    // Shader management
    gfx["reloadShaders"] = []() {
        AssetManager::Get().ReloadShaders();
    };

    // ===== 2D Immediate Mode Drawing =====
    // Note: These use the BatchRenderer2D and should be called within proper scene bounds

    // Set current draw color
    gfx["setColor"] = sol::overload(
        [](float r, float g, float b) {
            s_CurrentColor = glm::vec4(r, g, b, 1.0f);
        },
        [](float r, float g, float b, float a) {
            s_CurrentColor = glm::vec4(r, g, b, a);
        },
        [](const glm::vec4& color) {
            s_CurrentColor = color;
        }
    );

    gfx["getColor"] = []() -> glm::vec4 {
        return s_CurrentColor;
    };

    // Draw colored rectangle
    gfx["drawRect"] = sol::overload(
        [graphics](float x, float y, float w, float h) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(w, h), s_CurrentColor);
        },
        [graphics](float x, float y, float w, float h, float rotation) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawRotatedQuad(glm::vec2(x, y), glm::vec2(w, h), rotation, s_CurrentColor);
        }
    );

    // Draw textured sprite
    gfx["drawSprite"] = sol::overload(
        // Draw with texture ID at position
        [graphics](uint32_t texID, float x, float y) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(64, 64), texID, s_CurrentColor);
        },
        // Draw with texture ID, position, and size
        [graphics](uint32_t texID, float x, float y, float w, float h) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawQuad(glm::vec2(x, y), glm::vec2(w, h), texID, s_CurrentColor);
        },
        // Draw with texture ID, position, size, and rotation
        [graphics](uint32_t texID, float x, float y, float w, float h, float rotation) {
            auto* batch = graphics->renderer->GetBatchRenderer();
            batch->DrawRotatedQuad(glm::vec2(x, y), glm::vec2(w, h), rotation, texID, s_CurrentColor);
        }
    );

    // Begin/End 2D scene (for manual control)
    gfx["begin2D"] = [graphics]() {
        auto* batch = graphics->renderer->GetBatchRenderer();
        // Use orthographic projection matching screen size
        auto size = Window::Get()->GetSize();
        glm::mat4 proj = glm::ortho(0.0f, (float)size.width, (float)size.height, 0.0f, -1.0f, 1.0f);
        batch->BeginScene(proj);
    };

    gfx["end2D"] = [graphics]() {
        auto* batch = graphics->renderer->GetBatchRenderer();
        batch->EndScene();
    };

    // ===== CameraComponent usertype =====
    lua.new_usertype<CameraComponent>("CameraComponent",
        sol::no_constructor,

        "gameObject", sol::readonly(&CameraComponent::gameObject)
    );

    // ===== LightComponent usertype =====
    lua.new_usertype<LightComponent>("LightComponent",
        sol::no_constructor,

        "gameObject", sol::readonly(&LightComponent::gameObject),

        // Direct access to public members
        "intensity", &LightComponent::intensity,
        "ambient", &LightComponent::ambient,
        "diffuse", &LightComponent::diffuse,
        "specular", &LightComponent::specular,
        "castShadow", &LightComponent::castShadow
    );

    // ===== Asset management =====
    sol::table assets = atmos.create("assets");

    assets["createCubeMesh"] = [](const std::string& name, float size) -> Mesh* {
        return AssetManager::Get().CreateCubeMesh(name, size);
    };

    assets["createSphereMesh"] = [](const std::string& name, float radius, int segments) -> Mesh* {
        return AssetManager::Get().CreateSphereMesh(name, radius, segments);
    };

    assets["createPlaneMesh"] = [](const std::string& name, float width, float height) -> Mesh* {
        return AssetManager::Get().CreatePlaneMesh(name, width, height);
    };

    assets["getMesh"] = [](const std::string& name) -> Mesh* {
        return AssetManager::Get().GetMesh(name);
    };

    assets["loadTexture"] = [](const std::string& path) {
        AssetManager::Get().LoadTextures({path});
    };

    // Get texture by name (returns GL texture ID for use with drawSprite)
    assets["getTexture"] = [](const std::string& name) -> uint32_t {
        return AssetManager::Get().GetTexture(name);
    };

    // ===== Mesh usertype =====
    lua.new_usertype<Mesh>("Mesh",
        sol::no_constructor,

        "setMaterial", [](Mesh* mesh, int index) {
            auto& materials = AssetManager::Get().GetMaterials();
            if (index >= 0 && index < static_cast<int>(materials.size())) {
                mesh->SetMaterial(materials[index]);
            }
        }
    );
}
