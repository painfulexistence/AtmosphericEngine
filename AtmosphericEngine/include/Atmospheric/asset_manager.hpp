#pragma once
#include "globals.hpp"
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

class Mesh;
class Material;
class ShaderProgram;
struct ShaderProgramProps;
struct MaterialProps;

class Image {
public:
    Image(int width, int height, int numChannels, unsigned char* data)
      : width(width), height(height), channelCount(numChannels),
        byteArray(std::vector<unsigned char>(data, data + width * height * numChannels)) {
    }

    int width;
    int height;
    int channelCount;
    std::vector<unsigned char> byteArray;
};


using TextureID = uint32_t;
using ShaderID = uint32_t;
using MaterialID = uint32_t;

class AssetManager {
public:
    static AssetManager& Get();

    void Init();
    void Shutdown();

    // ========== CPU Resource Management ==========
    std::shared_ptr<Image> LoadImage(const std::string& path);

    Material* CreateMaterial(const std::string& name, const MaterialProps& props);
    Material* CreateMaterial(const MaterialProps& props);
    Material* GetMaterial(const std::string& name) const;
    Material* GetMaterialByID(uint32_t id) const;
    void LoadMaterials(const std::vector<MaterialProps>& materialDefs);

    ShaderProgram* CreateShader(const std::string& name, const ShaderProgramProps& props);
    ShaderProgram* GetShader(const std::string& name) const;
    ShaderProgram* GetShaderByID(uint32_t id) const;
    void LoadDefaultShaders();
    void LoadShaders(const std::unordered_map<std::string, ShaderProgramProps>& shaderDefs);
    void ReloadShaders();

    // ========== GPU Resource Management ==========
    GLuint CreateTexture(const std::string& path);
    GLuint CreateTextureFromImage(const std::shared_ptr<Image>& image);
    GLuint GetTexture(const std::string& name) const;
    GLuint GetTextureByID(uint32_t id) const;
    void LoadDefaultTextures();
    void LoadTextures(const std::vector<std::string>& paths);
    Mesh* CreateMesh(Mesh* mesh = nullptr);
    Mesh* CreateMesh(const std::string& name, Mesh* mesh = nullptr);
    Mesh* CreateCubeMesh(const std::string& name, float size = 1.0f);
    Mesh* CreatePlaneMesh(const std::string& name, float width, float height);
    Mesh* CreateSphereMesh(const std::string& name, float radius = 0.5f, int division = 18);
    Mesh* CreateCapsuleMesh(const std::string& name, float radius = 0.5f, float height = 3.0f);
    Mesh* CreateTerrainMesh(const std::string& name, float worldSize = 1024.f, int resolution = 10);
    Mesh* GetMesh(const std::string& name) const;
    std::shared_ptr<Mesh> LoadOBJ(const std::string& path);
    std::shared_ptr<Mesh> LoadGLTF(const std::string& path);

    // ========== Resource Access ==========
    const std::vector<GLuint>& GetTextures() const {
        return textures;
    }
    const std::vector<GLuint>& GetDefaultTextures() const {
        return defaultTextures;
    }
    const std::vector<Material*>& GetMaterials() const {
        return materials;
    }

    // ========== Cleanup ==========
    void Clear();

private:
    AssetManager() = default;
    ~AssetManager();

    static AssetManager* instance;

    // Images
    std::unordered_map<std::string, std::shared_ptr<Image>> _imageCache;

    // Shaders
    std::vector<ShaderProgram*> shaders;
    std::unordered_map<std::string, uint32_t> _shaderCache;
    uint32_t _nextShaderID = 0;

    // Materials
    std::vector<Material*> materials;
    std::unordered_map<std::string, uint32_t> _materialCache;
    uint32_t _nextMaterialID = 0;

    // Textures
    std::vector<GLuint> defaultTextures;
    std::vector<GLuint> textures;
    std::unordered_map<std::string, uint32_t> _textureCache;
    uint32_t _nextTextureID = 0;

    // Meshes
    std::vector<Mesh*> meshes;
    std::unordered_map<std::string, Mesh*> _meshCache;
    uint32_t _nextMeshID = 0;
};