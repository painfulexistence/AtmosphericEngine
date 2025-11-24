#include "asset_manager.hpp"
#include "console.hpp"
#include "job_system.hpp"
#include "material.hpp"
#include "mesh.hpp"
#include "shader.hpp"

#include "fmt/core.h"

// TODO: enable SIMD again after fixing build issues on Windows and Linux (ref:
// https://facebook.github.io/facebook360_dep/source/html/stb__image_8h_source.html)
#define STBI_NO_SIMD
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

// #define TINYOBJLOADER_IMPLEMENTATION
// #include "tiny_obj_loader.h"


AssetManager* AssetManager::instance = nullptr;

AssetManager& AssetManager::Get() {
    if (!instance) {
        instance = new AssetManager();
    }
    return *instance;
}

AssetManager::~AssetManager() {
    Clear();
}

void AssetManager::Init() {
    ENGINE_LOG("AssetManager initialized");
}

void AssetManager::Shutdown() {
    Clear();
    ENGINE_LOG("AssetManager shutdown");
}

void AssetManager::Clear() {
    // Clean up textures
    if (!textures.empty()) {
        glDeleteTextures(textures.size(), textures.data());
        textures.clear();
    }
    if (!defaultTextures.empty()) {
        glDeleteTextures(defaultTextures.size(), defaultTextures.data());
        defaultTextures.clear();
    }
    _textureCache.clear();
    _nextTextureID = 0;

    // Clean up shaders
    for (auto* shader : shaders) {
        delete shader;
    }
    shaders.clear();
    _shaderCache.clear();
    _nextShaderID = 0;

    // Clean up meshes
    for (auto* mesh : meshes) {
        delete mesh;
    }
    meshes.clear();
    _meshCache.clear();

    // Clean up materials
    for (auto* material : materials) {
        delete material;
    }
    materials.clear();
    _materialCache.clear();
    _nextMaterialID = 0;

    // Clear CPU cache
    _imageCache.clear();
}

// ============================================================================
// Image Management
// ============================================================================

std::shared_ptr<Image> AssetManager::LoadImage(const std::string& path) {
    // Check cache first
    auto it = _imageCache.find(path);
    if (it != _imageCache.end()) {
        return it->second;
    }

    int width, height, numChannels;
    if (!stbi_info(path.c_str(), &width, &height, &numChannels)) {
        throw std::runtime_error(fmt::format("Failed to load image at {}!\n", path));
    }

    int desiredChannels = 0;
    switch (numChannels) {
    case 1:
        desiredChannels = 1;
        break;
    case 3:
        desiredChannels = 4;
        break;
    case 4:
        desiredChannels = 4;
        break;
    default:
        throw std::runtime_error(fmt::format("Unknown texture format at {}\n", path));
        break;
    }

    uint8_t* data = stbi_load(path.c_str(), &width, &height, &numChannels, desiredChannels);
    if (data) {
        auto image = std::make_shared<Image>(width, height, desiredChannels, data);
        stbi_image_free(data);
        _imageCache[path] = image;
        return image;
    } else {
        stbi_image_free(data);
        return nullptr;
    }
}

// ============================================================================
// Shader Management
// ============================================================================

void AssetManager::LoadDefaultShaders() {
    LoadShaders({ {
                    "color",
                    { .vert = "assets/shaders/tbn.vert", .frag = "assets/shaders/pbr.frag" },
                  },
                  { "debug_line",
                    {
                      .vert = "assets/shaders/debug.vert",
                      .frag = "assets/shaders/flat.frag",
                    } },
                  {
                    "depth",
                    { .vert = "assets/shaders/depth_simple.vert", .frag = "assets/shaders/depth_simple.frag" },
                  },
                  {
                    "depth_cubemap",
                    { .vert = "assets/shaders/depth_cubemap.vert", .frag = "assets/shaders/depth_cubemap.frag" },
                  },
                  {
                    "hdr",
                    { .vert = "assets/shaders/hdr.vert", .frag = "assets/shaders/hdr_ca.frag" },
                  },
                  {
                    "terrain",
                    { .vert = "assets/shaders/terrain.vert",
                      .frag = "assets/shaders/terrain.frag",
                      .tesc = "assets/shaders/terrain.tesc",
                      .tese = "assets/shaders/terrain.tese" },
                  },
                  { "canvas", { .vert = "assets/shaders/canvas.vert", .frag = "assets/shaders/canvas.frag" } },
                  { "geometry", { .vert = "assets/shaders/geometry.vert", .frag = "assets/shaders/geometry.frag" } },
                  { "lighting", { .vert = "assets/shaders/lighting.vert", .frag = "assets/shaders/lighting.frag" } } });
}

void AssetManager::LoadShaders(const std::unordered_map<std::string, ShaderProgramProps>& shaderDefs) {
    for (const auto& [name, props] : shaderDefs) {
        CreateShader(name, props);
    }
}

ShaderProgram* AssetManager::CreateShader(const std::string& name, const ShaderProgramProps& props) {
    // Check if already exists
    auto it = _shaderCache.find(name);
    if (it != _shaderCache.end()) {
        ENGINE_LOG("Shader '{}' already exists, returning existing shader", name);
        return shaders[it->second];
    }

    auto* shader = new ShaderProgram(props);
    shaders.push_back(shader);
    _shaderCache[name] = _nextShaderID++;

    ENGINE_LOG("Shader '{}' loaded", name);
    return shader;
}

ShaderProgram* AssetManager::GetShader(const std::string& name) const {
    auto it = _shaderCache.find(name);
    if (it != _shaderCache.end()) {
        return shaders[it->second];
    }
    throw std::runtime_error(fmt::format("Shader '{}' not found", name));
}

ShaderProgram* AssetManager::GetShaderByID(uint32_t id) const {
    if (id < shaders.size()) {
        return shaders[id];
    }
    throw std::runtime_error(fmt::format("Shader ID {} out of range", id));
}

void AssetManager::ReloadShaders() {
    // TODO: Implement shader hot reloading
    ENGINE_LOG("Shader reloading not yet implemented");
}

// ============================================================================
// Material Management
// ============================================================================

void AssetManager::LoadMaterials(const std::vector<MaterialProps>& materialDefs) {
    for (const auto& props : materialDefs) {
        materials.push_back(new Material(props));
    }
}

Material* AssetManager::CreateMaterial(const std::string& name, const MaterialProps& props) {
    // Check if already exists
    auto it = _materialCache.find(name);
    if (it != _materialCache.end()) {
        ENGINE_LOG("Material '{}' already exists, returning existing material", name);
        return GetMaterialByID(it->second);
    }

    auto* material = new Material(props);
    materials.push_back(material);
    _materialCache[name] = _nextMaterialID++;
    return material;
}

Material* AssetManager::CreateMaterial(const MaterialProps& props) {
    auto* material = new Material(props);
    materials.push_back(material);
    _materialCache["unnamed_" + std::to_string(_nextMaterialID++)] = _nextMaterialID;
    return material;
}

Material* AssetManager::GetMaterial(const std::string& name) const {
    auto it = _materialCache.find(name);
    if (it != _materialCache.end()) {
        return GetMaterialByID(it->second);
    }
    throw std::runtime_error(fmt::format("Material '{}' not found", name));
}

Material* AssetManager::GetMaterialByID(uint32_t id) const {
    if (id < materials.size()) {
        return materials[id];
    }
    throw std::runtime_error(fmt::format("Material ID {} out of range", id));
}

// ============================================================================
// GPU Texture Management
// ============================================================================

void AssetManager::LoadDefaultTextures() {
    LoadTextures({ "assets/textures/default_diff.jpg",
                   "assets/textures/default_norm.jpg",
                   "assets/textures/default_ao.jpg",
                   "assets/textures/default_rough.jpg",
                   "assets/textures/default_metallic.jpg" });

    // Store as default textures
    defaultTextures = textures;
    textures.clear();
}

void AssetManager::LoadTextures(const std::vector<std::string>& paths) {
    int oldCount = textures.size(), newCount = paths.size();
    textures.resize(oldCount + newCount);

    // Load images in parallel using JobSystem
    std::vector<std::shared_ptr<Image>> images(newCount);
    for (int i = 0; i < newCount; i++) {
        auto path = paths[i];
        auto image = &images[i];
        JobSystem::Get()->Execute([this, path, image](int threadID) { *image = LoadImage(path); });
    }
    JobSystem::Get()->Wait();

    // Upload to GPU
    glGenTextures(newCount, &textures[oldCount]);
    for (int i = 0; i < newCount; i++) {
        auto img = images[i];
        if (!img) {
            throw std::runtime_error(fmt::format("Failed to load texture at {}\n", paths[i]));
        }

        GLuint texID = textures[oldCount + i];
        glBindTexture(GL_TEXTURE_2D, texID);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        switch (img->channelCount) {
        case 1:
            glTexImage2D(
              GL_TEXTURE_2D, 0, GL_R8, img->width, img->height, 0, GL_RED, GL_UNSIGNED_BYTE, img->byteArray.data()
            );
            break;
        case 3:
            glTexImage2D(
              GL_TEXTURE_2D, 0, GL_RGB, img->width, img->height, 0, GL_RGB, GL_UNSIGNED_BYTE, img->byteArray.data()
            );
            break;
        case 4:
            glTexImage2D(
              GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img->byteArray.data()
            );
            break;
        default:
            throw std::runtime_error(fmt::format("Unknown texture format at {}\n", paths[i]));
        }
        glGenerateMipmap(GL_TEXTURE_2D);

        // Store in map
        _textureCache[paths[i]] = texID;
    }
}

GLuint AssetManager::CreateTexture(const std::string& path) {
    // Check if already loaded
    auto it = _textureCache.find(path);
    if (it != _textureCache.end()) {
        return GetTextureByID(it->second);
    }

    // Load and create texture
    auto image = LoadImage(path);
    return CreateTextureFromImage(image);
}

GLuint AssetManager::CreateTextureFromImage(const std::shared_ptr<Image>& image) {
    if (!image) {
        throw std::runtime_error("Cannot create texture from null image");
    }

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_2D, texID);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    switch (image->channelCount) {
    case 1:
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_R8, image->width, image->height, 0, GL_RED, GL_UNSIGNED_BYTE, image->byteArray.data()
        );
        break;
    case 3:
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGB, image->width, image->height, 0, GL_RGB, GL_UNSIGNED_BYTE, image->byteArray.data()
        );
        break;
    case 4:
        glTexImage2D(
          GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image->byteArray.data()
        );
        break;
    }
    glGenerateMipmap(GL_TEXTURE_2D);

    _textureCache["unnamed_" + std::to_string(_nextTextureID++)] = _nextTextureID;
    textures.push_back(texID);
    return texID;
}

GLuint AssetManager::GetTexture(const std::string& name) const {
    auto it = _textureCache.find(name);
    if (it != _textureCache.end()) {
        return GetTextureByID(it->second);
    }
    throw std::runtime_error(fmt::format("Texture '{}' not found", name));
}

GLuint AssetManager::GetTextureByID(uint32_t id) const {
    if (id < textures.size()) {
        return textures[id];
    }
    throw std::runtime_error(fmt::format("Texture ID {} out of range", id));
}

// ============================================================================
// GPU Mesh Management
// ============================================================================

Mesh* AssetManager::CreateMesh(Mesh* mesh) {
    return CreateMesh("unnamed_" + std::to_string(_nextMeshID++), mesh);
}

Mesh* AssetManager::CreateMesh(const std::string& name, Mesh* mesh) {
    if (!mesh) {
        mesh = new Mesh();
    }
    meshes.push_back(mesh);
    _meshCache[name] = mesh;
    return mesh;
}

Mesh* AssetManager::CreateCubeMesh(const std::string& name, float size) {
    auto mesh = MeshBuilder::CreateCube(size);
    _meshCache[name] = mesh;
    return mesh;
}

Mesh* AssetManager::CreateSphereMesh(const std::string& name, float radius, int division) {
    auto mesh = MeshBuilder::CreateSphere(radius, division);
    _meshCache[name] = mesh;
    return mesh;
}

Mesh* AssetManager::CreateCapsuleMesh(const std::string& name, float radius, float height) {
    // TODO: Implement capsule mesh generation
    ENGINE_LOG("Capsule mesh '{}' created (generation not yet implemented)", name);
    return new Mesh();
}

Mesh* AssetManager::CreateTerrainMesh(const std::string& name, float worldSize, int resolution) {
    auto mesh = MeshBuilder::CreateTerrain(worldSize, resolution);
    _meshCache[name] = mesh;
    return mesh;
}

Mesh* AssetManager::GetMesh(const std::string& name) const {
    auto it = _meshCache.find(name);
    if (it != _meshCache.end()) {
        return it->second;
    }
    throw std::runtime_error(fmt::format("Mesh '{}' not found", name));
}

std::shared_ptr<Mesh> AssetManager::LoadOBJ(const std::string& path) {
    std::vector<Vertex> vertices;
    std::vector<uint16_t> indices;

    // tinyobj::attrib_t attrib;
    // std::vector<tinyobj::shape_t> shapes;
    // std::vector<tinyobj::material_t> materials;
    // std::string err;

    // if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &err, path.c_str())) {
    //     throw std::runtime_error(fmt::format("Failed to load model: {}", err));
    // }

    // for (const auto& shape : shapes) {
    //     for (const auto& index : shape.mesh.indices) {
    //         Vertex vert = {};
    //         vert.position = { attrib.vertices[3 * index.vertex_index + 0],
    //                           attrib.vertices[3 * index.vertex_index + 1],
    //                           attrib.vertices[3 * index.vertex_index + 2] };
    //         vert.normal = { attrib.normals[3 * index.normal_index + 0],
    //                         attrib.normals[3 * index.normal_index + 1],
    //                         attrib.normals[3 * index.normal_index + 2] };
    //         vert.uv = { attrib.texcoords[2 * index.texcoord_index + 0],
    //                     1.0 - attrib.texcoords[2 * index.texcoord_index + 1] };
    //         vertices.push_back(vert);
    //         indices.push_back(indices.size());
    //     }
    // }

    auto mesh = std::make_shared<Mesh>(MeshType::PRIM);
    mesh->Initialize(vertices, indices);

    return mesh;
}