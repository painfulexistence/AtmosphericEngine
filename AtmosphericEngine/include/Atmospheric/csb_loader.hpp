#pragma once
#include "globals.hpp"
#include <functional>
#include <string>
#include <unordered_map>
#include <vector>

class Application;
class GameObject;

namespace flatbuffers {
struct CSParseBinary;
struct NodeTree;
struct WidgetOptions;
struct SpriteOptions;
struct ImageViewOptions;
struct SingleNodeOptions;
}// namespace flatbuffers

// Result of loading a CSB file
struct CSBLoadResult {
    GameObject* root = nullptr;
    std::vector<GameObject*> allNodes;
    std::unordered_map<int, GameObject*> nodesByActionTag;
    std::unordered_map<std::string, GameObject*> nodesByName;
    bool success = false;
    std::string error;
};

// Configuration for CSB loading
struct CSBLoadConfig {
    std::string basePath = "";// Base path for resolving texture paths
    bool loadTextures = true;
    bool applyTransforms = true;
    CanvasLayer defaultLayer = CanvasLayer::LAYER_WORLD;

    // Callback for custom node type handling
    // Return true if handled, false to use default handling
    std::function<bool(GameObject*, const std::string& classname, const flatbuffers::NodeTree*)> customNodeHandler =
      nullptr;
};

class CSBLoader {
public:
    explicit CSBLoader(Application* app);
    ~CSBLoader() = default;

    // Load a CSB file and create GameObjects
    CSBLoadResult Load(const std::string& path, const CSBLoadConfig& config = {});

    // Load from memory buffer
    CSBLoadResult LoadFromBuffer(const uint8_t* buffer, size_t size, const CSBLoadConfig& config = {});

    // Get supported node types
    static std::vector<std::string> GetSupportedNodeTypes();

private:
    Application* _app;

    // Parse the CSB binary and create node hierarchy
    GameObject* ParseNodeTree(
      const flatbuffers::NodeTree* nodeTree, const CSBLoadConfig& config, CSBLoadResult& result, GameObject* parent
    );

    // Node type handlers
    GameObject* CreateNode(const flatbuffers::WidgetOptions* options, const CSBLoadConfig& config);
    GameObject* CreateSprite(const flatbuffers::SpriteOptions* options, const CSBLoadConfig& config);
    GameObject* CreateImageView(const flatbuffers::ImageViewOptions* options, const CSBLoadConfig& config);
    GameObject* CreateSingleNode(const flatbuffers::SingleNodeOptions* options, const CSBLoadConfig& config);

    // Apply common widget options to a GameObject
    void ApplyWidgetOptions(GameObject* go, const flatbuffers::WidgetOptions* options, const CSBLoadConfig& config);

    // Resolve texture path
    int ResolveTexture(const std::string& path, const std::string& plistFile, int resourceType, const CSBLoadConfig& config);
};
