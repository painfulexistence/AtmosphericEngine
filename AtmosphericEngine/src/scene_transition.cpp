#include "scene_transition.hpp"
#include "asset_manager.hpp"
#include "file_system.hpp"
#include <nlohmann/json.hpp>
#include <spdlog/spdlog.h>

// ── GameManifest ─────────────────────────────────────────────────────────────

static GameManifest ParseJSON(const std::string& json)
{
    GameManifest manifest;
    try {
        auto j = nlohmann::json::parse(json);
        manifest.name = j.value("name", "");

        if (j.contains("textures"))
            for (auto& v : j["textures"])
                manifest.textures.push_back(v.get<std::string>());

        if (j.contains("shaders"))
            for (auto& v : j["shaders"])
                manifest.shaders.push_back(v.get<std::string>());

    } catch (const std::exception& e) {
        spdlog::error("GameManifest: JSON parse error: {}", e.what());
    }
    return manifest;
}

GameManifest GameManifest::FromJSON(const std::string& json)
{
    return ParseJSON(json);
}

// ── ClearScene ───────────────────────────────────────────────────────────────
// Clears all scene-specific GPU resources, preserving default textures.
// TODO: preserve default shaders once AssetManager tracks them separately.

static void ClearScene()
{
    AssetManager::Get().ClearSceneAssets();
}

// ── SceneTransition::Go ──────────────────────────────────────────────────────

void SceneTransition::Go(const std::string& sceneName, OnReadyFn onReady, OnErrorFn onError,
                         const std::string& currentSceneName)
{
    const std::string manifestPath = std::string(kManifestDir) + sceneName + ".json";

    bool shouldClear = !currentSceneName.empty();
    FileSystem::Get().Prefetch({ manifestPath }, [sceneName, manifestPath, shouldClear, onReady, onError]() {
        auto bytes = FileSystem::Get().ReadSync(manifestPath);
        if (bytes.empty()) {
            std::string reason = "failed to read manifest: " + manifestPath;
            spdlog::error("SceneTransition: {}", reason);
            if (onError) onError(reason);
            return;
        }

        auto manifest = GameManifest::FromJSON(std::string(bytes.begin(), bytes.end()));

        // Prefetch all scene assets, then load.
        std::vector<std::string> allPaths;
        allPaths.insert(allPaths.end(), manifest.textures.begin(), manifest.textures.end());

        spdlog::info("SceneTransition: prefetching {} asset(s) for '{}'",
                     allPaths.size(), sceneName);

        FileSystem::Get().Prefetch(allPaths, [manifest, sceneName, shouldClear, onReady, onError]() {
            spdlog::info("SceneTransition: loading '{}'", sceneName);

            if (shouldClear)
                ClearScene();

            try {
                if (!manifest.textures.empty())
                    AssetManager::Get().LoadTextures(manifest.textures);

                // TODO: load shaders from manifest.shaders

            } catch (const std::exception& e) {
                spdlog::error("SceneTransition: load failed: {}", e.what());
                if (onError) onError(e.what());
                return;
            }

            spdlog::info("SceneTransition: '{}' ready", sceneName);
            if (onReady) onReady();
        });
    });
}
