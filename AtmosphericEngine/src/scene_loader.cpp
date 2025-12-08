#include "scene_loader.hpp"
#include "application.hpp"
#include "asset_manager.hpp"
#include "game_object.hpp"
#include "sprite_component.hpp"

#include "Scene_generated.h"

#include <fstream>
#include <spdlog/spdlog.h>

SceneLoader::SceneLoader(Application* app) : _app(app) {
}

SceneLoadResult SceneLoader::Load(const std::string& path, const SceneLoadConfig& config) {
    SceneLoadResult result;

    // Read file
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        result.error = "Failed to open file: " + path;
        spdlog::error("SceneLoader: {}", result.error);
        return result;
    }

    std::streamsize size = file.tellg();
    file.seekg(0, std::ios::beg);

    std::vector<uint8_t> buffer(size);
    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        result.error = "Failed to read file: " + path;
        spdlog::error("SceneLoader: {}", result.error);
        return result;
    }

    // Derive base path from file path if not specified
    SceneLoadConfig actualConfig = config;
    if (actualConfig.basePath.empty()) {
        size_t lastSlash = path.find_last_of("/\\");
        if (lastSlash != std::string::npos) {
            actualConfig.basePath = path.substr(0, lastSlash + 1);
        }
    }

    return LoadFromBuffer(buffer.data(), buffer.size(), actualConfig);
}

SceneLoadResult SceneLoader::LoadFromBuffer(const uint8_t* buffer, size_t size, const SceneLoadConfig& config) {
    SceneLoadResult result;

    // Verify the buffer
    flatbuffers::Verifier verifier(buffer, size);
    if (!flatbuffers::VerifyCSParseBinaryBuffer(verifier)) {
        result.error = "Invalid CSB buffer - verification failed";
        spdlog::error("SceneLoader: {}", result.error);
        return result;
    }

    // Get the root
    auto csb = flatbuffers::GetCSParseBinary(buffer);
    if (!csb) {
        result.error = "Failed to parse CSB buffer";
        spdlog::error("SceneLoader: {}", result.error);
        return result;
    }

    // Log version
    if (csb->version()) {
        spdlog::info("SceneLoader: Loading CSB version {}", csb->version()->c_str());
    }

    // Load textures if requested
    if (config.loadTextures && csb->textures()) {
        for (auto tex : *csb->textures()) {
            if (tex) {
                std::string texPath = config.basePath + tex->c_str();
                AssetManager::Get().CreateTexture(texPath);
                spdlog::debug("SceneLoader: Loaded texture {}", texPath);
            }
        }
    }

    // Also load PNG textures
    if (config.loadTextures && csb->texturePngs()) {
        for (auto tex : *csb->texturePngs()) {
            if (tex) {
                std::string texPath = config.basePath + tex->c_str();
                AssetManager::Get().CreateTexture(texPath);
                spdlog::debug("SceneLoader: Loaded PNG texture {}", texPath);
            }
        }
    }

    // Parse node tree
    if (csb->nodeTree()) {
        result.root = ParseNodeTree(csb->nodeTree(), config, result, nullptr);
        result.success = (result.root != nullptr);
    } else {
        result.error = "CSB has no node tree";
        spdlog::warn("SceneLoader: {}", result.error);
    }

    // TODO: Parse animations (csb->action()) in future iteration

    return result;
}

GameObject* SceneLoader::ParseNodeTree(
  const flatbuffers::NodeTree* nodeTree, const SceneLoadConfig& config, SceneLoadResult& result, GameObject* parent
) {
    if (!nodeTree)
        return nullptr;

    std::string classname = nodeTree->classname() ? nodeTree->classname()->c_str() : "Node";
    GameObject* go = nullptr;

    // Check for custom handler first
    if (config.customNodeHandler && config.customNodeHandler(nullptr, classname, nodeTree)) {
        // Custom handler will create and return the GameObject
        // For now, we don't support this pattern - custom handler should return via result
        spdlog::debug("SceneLoader: Custom handler used for {}", classname);
    }

    // Get the options
    const flatbuffers::WidgetOptions* widgetOptions = nullptr;
    if (nodeTree->options() && nodeTree->options()->data()) {
        widgetOptions = nodeTree->options()->data();
    }

    // Create node based on classname
    if (classname == "Sprite") {
        // For Sprite, we need to get SpriteOptions from the options
        // CSB stores type-specific options in the Options table
        // The actual SpriteOptions is accessed via the data field with correct type
        go = CreateNode(widgetOptions, config);

        // Try to get sprite-specific data
        // In CSB format, sprite options include fileNameData
        if (widgetOptions) {
            // For POC: Create a sprite with the widget options
            // Full implementation would need to access SpriteOptions
            SpriteProps props;
            if (widgetOptions->size()) {
                props.size = glm::vec2(widgetOptions->size()->width(), widgetOptions->size()->height());
            }
            if (widgetOptions->anchorPoint()) {
                props.pivot = glm::vec2(widgetOptions->anchorPoint()->scaleX(), widgetOptions->anchorPoint()->scaleY());
            }
            if (widgetOptions->color()) {
                props.color = glm::vec4(
                  widgetOptions->color()->r() / 255.0f,
                  widgetOptions->color()->g() / 255.0f,
                  widgetOptions->color()->b() / 255.0f,
                  widgetOptions->alpha() / 255.0f
                );
            }
            props.layer = config.defaultLayer;
            props.flipX = widgetOptions->flipX();
            props.flipY = widgetOptions->flipY();
            props.zOrder = widgetOptions->zOrder();

            // Apply sprite component
            if (go) {
                go->AddComponent<SpriteComponent>(props);
            }
        }
    } else if (classname == "ImageView") {
        go = CreateNode(widgetOptions, config);
        if (widgetOptions && go) {
            SpriteProps props;
            if (widgetOptions->size()) {
                props.size = glm::vec2(widgetOptions->size()->width(), widgetOptions->size()->height());
            }
            if (widgetOptions->anchorPoint()) {
                props.pivot = glm::vec2(widgetOptions->anchorPoint()->scaleX(), widgetOptions->anchorPoint()->scaleY());
            }
            if (widgetOptions->color()) {
                props.color = glm::vec4(
                  widgetOptions->color()->r() / 255.0f,
                  widgetOptions->color()->g() / 255.0f,
                  widgetOptions->color()->b() / 255.0f,
                  widgetOptions->alpha() / 255.0f
                );
            }
            props.layer = config.defaultLayer;
            props.flipX = widgetOptions->flipX();
            props.flipY = widgetOptions->flipY();
            props.zOrder = widgetOptions->zOrder();
            go->AddComponent<SpriteComponent>(props);
        }
    } else if (classname == "Node" || classname == "SingleNode") {
        go = CreateNode(widgetOptions, config);
    } else {
        // Unknown type - create as basic node and log warning
        spdlog::warn("SceneLoader: Unsupported node type '{}', creating as basic node", classname);
        go = CreateNode(widgetOptions, config);
    }

    if (!go) {
        spdlog::error("SceneLoader: Failed to create GameObject for {}", classname);
        return nullptr;
    }

    // Apply widget options
    if (widgetOptions) {
        ApplyWidgetOptions(go, widgetOptions, config);
    }

    // Set parent relationship
    if (parent) {
        go->parent = parent;
    }

    // Track in result
    result.allNodes.push_back(go);
    if (widgetOptions) {
        if (widgetOptions->actionTag() != 0) {
            result.nodesByActionTag[widgetOptions->actionTag()] = go;
        }
        if (widgetOptions->name()) {
            result.nodesByName[widgetOptions->name()->c_str()] = go;
        }
    }

    // Process children
    if (nodeTree->children()) {
        for (auto child : *nodeTree->children()) {
            ParseNodeTree(child, config, result, go);
        }
    }

    return go;
}

GameObject* SceneLoader::CreateNode(const flatbuffers::WidgetOptions* options, const SceneLoadConfig& config) {
    glm::vec3 position(0.0f);
    glm::vec3 rotation(0.0f);
    glm::vec3 scale(1.0f);

    if (options && config.applyTransforms) {
        if (options->position()) {
            position = glm::vec3(options->position()->x(), options->position()->y(), 0.0f);
        }
        if (options->rotationSkew()) {
            // CSB uses rotationSkewX for Z rotation in 2D
            rotation = glm::vec3(0.0f, 0.0f, options->rotationSkew()->rotationSkewX());
        }
        if (options->scale()) {
            scale = glm::vec3(options->scale()->scaleX(), options->scale()->scaleY(), 1.0f);
        }
    }

    return _app->CreateGameObject(position, rotation, scale);
}

GameObject* SceneLoader::CreateSprite(const flatbuffers::SpriteOptions* options, const SceneLoadConfig& config) {
    auto go = _app->CreateGameObject();
    if (!go || !options)
        return go;

    SpriteProps props;

    // Get base widget options
    auto* widgetOpts = options->nodeOptions();
    if (widgetOpts) {
        if (widgetOpts->size()) {
            props.size = glm::vec2(widgetOpts->size()->width(), widgetOpts->size()->height());
        }
        if (widgetOpts->anchorPoint()) {
            props.pivot = glm::vec2(widgetOpts->anchorPoint()->scaleX(), widgetOpts->anchorPoint()->scaleY());
        }
        if (widgetOpts->color()) {
            props.color = glm::vec4(
              widgetOpts->color()->r() / 255.0f,
              widgetOpts->color()->g() / 255.0f,
              widgetOpts->color()->b() / 255.0f,
              widgetOpts->alpha() / 255.0f
            );
        }
    }

    // Load texture from fileNameData
    if (options->fileNameData() && options->fileNameData()->path()) {
        props.textureID = ResolveTexture(
          options->fileNameData()->path()->c_str(),
          options->fileNameData()->plistFile() ? options->fileNameData()->plistFile()->c_str() : "",
          options->fileNameData()->resourceType(),
          config
        );
    }

    props.layer = config.defaultLayer;
    go->AddComponent<SpriteComponent>(props);

    return go;
}

GameObject* SceneLoader::CreateImageView(const flatbuffers::ImageViewOptions* options, const SceneLoadConfig& config) {
    auto go = _app->CreateGameObject();
    if (!go || !options)
        return go;

    SpriteProps props;

    auto* widgetOpts = options->widgetOptions();
    if (widgetOpts) {
        if (widgetOpts->size()) {
            props.size = glm::vec2(widgetOpts->size()->width(), widgetOpts->size()->height());
        }
        if (widgetOpts->anchorPoint()) {
            props.pivot = glm::vec2(widgetOpts->anchorPoint()->scaleX(), widgetOpts->anchorPoint()->scaleY());
        }
        if (widgetOpts->color()) {
            props.color = glm::vec4(
              widgetOpts->color()->r() / 255.0f,
              widgetOpts->color()->g() / 255.0f,
              widgetOpts->color()->b() / 255.0f,
              widgetOpts->alpha() / 255.0f
            );
        }
    }

    // Load texture
    if (options->fileNameData() && options->fileNameData()->path()) {
        props.textureID = ResolveTexture(
          options->fileNameData()->path()->c_str(),
          options->fileNameData()->plistFile() ? options->fileNameData()->plistFile()->c_str() : "",
          options->fileNameData()->resourceType(),
          config
        );
    }

    props.layer = config.defaultLayer;
    go->AddComponent<SpriteComponent>(props);

    return go;
}

GameObject* SceneLoader::CreateSingleNode(const flatbuffers::SingleNodeOptions* options, const SceneLoadConfig& config) {
    return CreateNode(options ? options->nodeOptions() : nullptr, config);
}

void SceneLoader::ApplyWidgetOptions(
  GameObject* go, const flatbuffers::WidgetOptions* options, const SceneLoadConfig& config
) {
    if (!go || !options)
        return;

    // Set name
    if (options->name()) {
        go->SetName(options->name()->c_str());
    }

    // Set visibility
    go->SetActive(options->visible());

    // Transform is already applied in CreateNode, but we can update here if needed
    if (config.applyTransforms) {
        if (options->position()) {
            go->SetPosition(glm::vec3(options->position()->x(), options->position()->y(), 0.0f));
        }
        if (options->rotationSkew()) {
            go->SetRotation(glm::vec3(0.0f, 0.0f, options->rotationSkew()->rotationSkewX()));
        }
        if (options->scale()) {
            go->SetScale(glm::vec3(options->scale()->scaleX(), options->scale()->scaleY(), 1.0f));
        }
    }
}

int SceneLoader::ResolveTexture(
  const std::string& path, const std::string& plistFile, int resourceType, const SceneLoadConfig& config
) {
    // Resource types in CSB:
    // 0 = Normal file
    // 1 = Plist (sprite sheet)
    // 2 = Default (built-in)

    if (path.empty()) {
        return -1;
    }

    std::string fullPath = config.basePath + path;

    // For now, only handle normal files (type 0)
    // TODO: Add plist/sprite sheet support
    if (resourceType == 1 && !plistFile.empty()) {
        spdlog::warn("SceneLoader: Plist sprite sheets not yet supported, loading as regular texture: {}", path);
    }

    // Try to get existing texture or create new one
    GLuint texID = AssetManager::Get().GetTexture(fullPath);
    if (texID == 0) {
        texID = AssetManager::Get().CreateTexture(fullPath);
    }

    return static_cast<int>(texID);
}

std::vector<std::string> SceneLoader::GetSupportedNodeTypes() {
    return {"Node", "SingleNode", "Sprite", "ImageView"};
}
