#include "Atmospheric.hpp"

#ifdef __EMSCRIPTEN__
// WebAssetFetcher: async texture fetch with IndexedDB caching.
// Include it only for Emscripten builds.
#include "../AtmosphericEngine/src/web_asset_fetcher.hpp"
#endif

class HelloWorld : public Application {
    using Application::Application;

    GameObject* cube;

    // World space sprites (rendered by WorldCanvasPass with depth testing)
    std::vector<GameObject*> worldSprites;

    // Screen space sprites (rendered by CanvasPass, no depth testing)
    std::vector<GameObject*> screenSprites;
    FontID fontID;

    void OnLoad() override {
        // Load font
        fontID = GraphicsServer::Get()->LoadFont("assets/fonts/NotoSans-SemiBold.ttf", 32.0f);

        SceneDef scene = {
            .materials = { { .baseMap = 0,
                             .normalMap = 1,
                             .aoMap = 2,
                             .roughnessMap = 3,
                             .diffuse = { 1., 1., 1. },
                             .specular = { .296648, .296648, .296648 },
                             .ambient = { .25, .20725, .20725 },
                             .shininess = 0.088 } },
        };
        LoadScene(scene);

        mainCamera->gameObject->SetPosition(glm::vec3(-10.0, 5.0, 0.0));

        // Create rotating cube
        auto cubeMesh = AssetManager::Get().CreateCubeMesh("CubeMesh", 1.0f);
        cubeMesh->SetMaterial(AssetManager::Get().GetMaterials()[0]);

        cube = CreateGameObject();
        cube->AddComponent<MeshComponent>(cubeMesh);

        // === World Space Sprites (WorldCanvasPass) ===
        // These are rendered with depth testing - occluded by 3D geometry
        glm::vec4 worldColors[] = {
            { 1.0f, 0.3f, 0.3f, 0.8f },// Red
            { 0.3f, 1.0f, 0.3f, 0.8f },// Green
            { 0.3f, 0.3f, 1.0f, 0.8f },// Blue
            { 1.0f, 1.0f, 0.3f, 0.8f },// Yellow
        };

        for (int i = 0; i < 4; i++) {
            auto* spriteObj = CreateGameObject();
            spriteObj->SetPosition(glm::vec3(i * 2.0f - 3.0f, 2.0f, 3.0f));

            spriteObj->AddComponent<SpriteComponent>(SpriteProps{
              .size = glm::vec2(1.0f, 1.0f),
              .pivot = glm::vec2(0.5f, 0.5f),
              .color = worldColors[i],
              .textureID = -1,
              .layer = CanvasLayer::LAYER_WORLD,
            });

            worldSprites.push_back(spriteObj);
        }

        // === 2D Sprites (CanvasPass) ===
        // These use screen coordinates (pixels), rendered after 3D
        glm::vec4 sprite2DColors[] = {
            { 1.0f, 0.5f, 0.0f, 0.9f },// Orange
            { 0.5f, 0.0f, 1.0f, 0.9f },// Purple
            { 0.0f, 1.0f, 1.0f, 0.9f },// Cyan
        };

        for (int i = 0; i < 3; i++) {
            auto* spriteObj = CreateGameObject();
            // Screen coordinates: top-left origin, pixels
            spriteObj->SetPosition(glm::vec3(20.0f + i * 70.0f, 20.0f, 0.0f));

            spriteObj->AddComponent<SpriteComponent>(SpriteProps{
              .size = glm::vec2(50.0f, 50.0f),// Pixels
              .pivot = glm::vec2(0.0f, 0.0f),// Top-left pivot
              .color = sprite2DColors[i],
              .textureID = -1,
              // Default layer is LAYER_WORLD_2D (2D screen space)
            });

            screenSprites.push_back(spriteObj);
        }

        console.Info(fmt::format("Game fully loaded in {:.1f} seconds", GetWindowTime()));
        console.Info("Press R to reload shaders, ESC to quit");
        console.Info("3D sprites: 4 (depth tested), 2D sprites: 3 (screen space)");
    }

    void OnUpdate(float dt, float time) override {
        cube->SetPosition(glm::vec3(0.0f, 5.0f, std::cos(time) * 2.0f));
        cube->SetRotation(glm::vec3(0.0, time * 0.5, time * 1.0));

        // Animate world sprites (float up and down in 3D space)
        for (size_t i = 0; i < worldSprites.size(); i++) {
            float offset = std::sin(time * 2.0f + i * 0.5f) * 0.5f;
            worldSprites[i]->SetPosition(glm::vec3(i * 2.0f - 3.0f, 2.0f + offset, 3.0f));
        }

        // Animate screen sprites (pulse alpha)
        for (size_t i = 0; i < screenSprites.size(); i++) {
            float pulse = 0.7f + 0.3f * std::sin(time * 3.0f + i * 1.0f);
            auto* sprite = screenSprites[i]->GetComponent<SpriteComponent>();
            glm::vec4 color = sprite->GetColor();
            color.a = pulse;
            sprite->SetColor(color);
        }

        // Draw Hello World text
        GraphicsServer::Get()->DrawText(
          fontID, "Hello World from C++!", 50.0f, 100.0f, 1.0f, glm::vec4(1.0f, 1.0f, 1.0f, 1.0f)
        );

        // Draw 3D text above cube
        GraphicsServer::Get()->DrawText3D(
          fontID, "Cube", cube->GetPosition() + glm::vec3(0.0f, 1.2f, 0.0f), 0.5f, glm::vec4(1.0f, 1.0f, 0.0f, 1.0f)
        );

        if (input.IsKeyDown(Key::R)) {
            AssetManager::Get().ReloadShaders();
        }
        if (input.IsKeyDown(Key::ESCAPE)) {
            Quit();
        }
    }
};

#ifdef __EMSCRIPTEN__
// ─────────────────────────────────────────────────────────────────────────────
// Web / WebAssembly entry point
//
// Memory strategy
// ───────────────
// Textures are loaded dynamically via emscripten_fetch rather than bundled
// in the --preload-file .data archive.  This has two benefits:
//
//   1. Startup heap footprint is small — the .data bundle only contains
//      shaders (a few hundred KB total), not all textures.
//
//   2. IndexedDB caching (EMSCRIPTEN_FETCH_PERSIST_FILE):
//      After the first visit the browser serves textures from IndexedDB,
//      so subsequent page-loads are near-instant and work offline.
//
// Peak WASM heap per KTX2 texture during load
//   ktx2_file_bytes (moved from fetch buffer)
//   + ETC2 block buffer (~4× smaller than uncompressed RGBA)
//   → both freed immediately after glCompressedTexImage2D
//
// For comparison, the old stb_image + --preload-file approach kept ALL
// textures in WASM memory simultaneously as uncompressed RGBA.
// ─────────────────────────────────────────────────────────────────────────────

// Default texture list — must match the .ktx2 paths in
// AssetManager::LoadDefaultTextures() (see asset_manager.cpp).
// Convert source images once with:
//   basisu -ktx2 -mipmap <src.jpg>
//   toktx --t2 --encode etc1s --mipmap <out.ktx2> <src.jpg>
static const std::vector<std::string> kDefaultTextureURLs = {
    "assets/textures/default_diff.ktx2",
    "assets/textures/default_norm.ktx2",
    "assets/textures/default_ao.ktx2",
    "assets/textures/default_rough.ktx2",
    "assets/textures/default_metallic.ktx2",
};

// Forward declaration — defined as a static local below to avoid a
// global with indeterminate initialization order in WASM.
static void StartGame();

int main(int argc, char* argv[]) {
    // Kick off async fetch for all textures before creating the Application.
    // WebAssetFetcher stores the bytes in AssetManager::_webAssetCache.
    // When all fetches complete, StartGame() is called by the browser event loop.
    //
    // main() returns immediately here; Emscripten's runtime keeps the page alive.
    WebAssetFetcher::Preload(kDefaultTextureURLs, StartGame);
    return 0;
}

static void StartGame() {
    // At this point all KTX2 bytes are in AssetManager::_webAssetCache.
    // Application::Run() → OnLoad() → LoadDefaultTextures() will find them
    // there and call LoadKTX2Texture() without an extra fopen()/fread().
    static HelloWorld game({
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    game.Run(); // installs emscripten_set_main_loop; never returns
}

#else
// ─────────────────────────────────────────────────────────────────────────────
// Native entry point (Linux / macOS / Windows)
// ─────────────────────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    HelloWorld game({
        .useDefaultTextures = true,
        .useDefaultShaders  = true,
    });
    game.Run();
    return 0;
}
#endif