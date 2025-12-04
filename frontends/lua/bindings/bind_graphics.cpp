#include "../lua_application.hpp"

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
