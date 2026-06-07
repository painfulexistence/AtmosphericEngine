#pragma once
#include "voxel_chunk.hpp"
#include "frustum.hpp"
#include "renderer.hpp"
#include <glm/vec3.hpp>
#include <memory>
#include <vector>

class GraphicsServer;
class PhysicsServer;
class ShaderProgram;

class VoxelWorld {
public:
    // World extents in chunks
    static constexpr int WORLD_X = 25;
    static constexpr int WORLD_Y = 3;
    static constexpr int WORLD_Z = 25;

    VoxelWorld() = default;
    ~VoxelWorld();

    void Init(GraphicsServer* gfx, PhysicsServer* physics, int seed = 42);
    void Update(float dt, const glm::vec3& cameraPos);

    // Submit visible chunk render commands to the renderer opaque queue.
    // Call this each frame before renderer->RenderFrame().
    void SubmitRenderCommands(Renderer* renderer, const glm::mat4& viewProj,
                              const glm::vec3& cameraPos);

    uint8_t GetVoxel(int wx, int wy, int wz) const;
    void    SetVoxel(int wx, int wy, int wz, uint8_t type);

private:
    std::vector<std::unique_ptr<VoxelChunk>> _chunks;
    GraphicsServer* _gfx     = nullptr;
    PhysicsServer*  _physics  = nullptr;
    int             _seed     = 42;

    VoxelChunk* GetChunk(int cx, int cy, int cz) const;
    int         ChunkIndex(int cx, int cy, int cz) const;

    void GenerateTerrain();
    void LinkNeighbors();
    void RebuildDirtyChunks();
};
