#include "voxel_world.hpp"
#include "graphics_server.hpp"
#include "frustum.hpp"

// FastNoiseLite – single-header noise library (pulled in by CMakeLists)
#include "FastNoiseLite.h"

#include <algorithm>
#include <cmath>

VoxelWorld::~VoxelWorld() {
    _chunks.clear();
}

void VoxelWorld::Init(GraphicsServer* gfx, PhysicsServer* physics, int seed) {
    _gfx     = gfx;
    _physics = physics;
    _seed    = seed;

    const int total = WORLD_X * WORLD_Y * WORLD_Z;
    _chunks.reserve(total);

    for (int cx = 0; cx < WORLD_X; ++cx)
        for (int cy = 0; cy < WORLD_Y; ++cy)
            for (int cz = 0; cz < WORLD_Z; ++cz)
                _chunks.push_back(std::make_unique<VoxelChunk>(glm::ivec3(cx, cy, cz)));

    GenerateTerrain();
    LinkNeighbors();
    RebuildDirtyChunks();
}

void VoxelWorld::Update(float /*dt*/, const glm::vec3& /*cameraPos*/) {
    RebuildDirtyChunks();
}

void VoxelWorld::SubmitRenderCommands(Renderer* renderer,
                                       const glm::mat4& viewProj,
                                       const glm::vec3& cameraPos)
{
    Frustum frustum(viewProj);

    for (auto& chunk : _chunks) {
        Mesh* mesh = chunk->GetMesh();
        if (!mesh || !mesh->UsesRenderMesh()) continue;

        // Bounding-sphere frustum cull
        glm::vec3 center = chunk->GetBoundingSphereCenter();
        float radius     = chunk->GetBoundingSphereRadius();
        if (!frustum.Intersects(center)) continue; // quick point check
        // (a full sphere test would need Frustum::IntersectsSphere, use bbox fallback)
        auto bbox = mesh->GetBoundingBox();
        if (!frustum.Intersects(bbox)) continue;

        glm::vec3 wp = chunk->GetWorldPos();
        glm::mat4 model = glm::translate(glm::mat4(1.0f), wp);

        RenderCommand cmd{ .mesh = mesh, .transform = model };
        renderer->SubmitCommand(cmd);
    }
}

uint8_t VoxelWorld::GetVoxel(int wx, int wy, int wz) const {
    int cx = wx / VoxelChunk::SIZE, lx = wx % VoxelChunk::SIZE;
    int cy = wy / VoxelChunk::SIZE, ly = wy % VoxelChunk::SIZE;
    int cz = wz / VoxelChunk::SIZE, lz = wz % VoxelChunk::SIZE;
    VoxelChunk* c = GetChunk(cx, cy, cz);
    return c ? c->GetVoxel(lx, ly, lz) : 0;
}

void VoxelWorld::SetVoxel(int wx, int wy, int wz, uint8_t type) {
    int cx = wx / VoxelChunk::SIZE, lx = wx % VoxelChunk::SIZE;
    int cy = wy / VoxelChunk::SIZE, ly = wy % VoxelChunk::SIZE;
    int cz = wz / VoxelChunk::SIZE, lz = wz % VoxelChunk::SIZE;
    VoxelChunk* c = GetChunk(cx, cy, cz);
    if (c) c->SetVoxel(lx, ly, lz, type);
}

VoxelChunk* VoxelWorld::GetChunk(int cx, int cy, int cz) const {
    int idx = ChunkIndex(cx, cy, cz);
    if (idx < 0) return nullptr;
    return _chunks[idx].get();
}

int VoxelWorld::ChunkIndex(int cx, int cy, int cz) const {
    if (cx < 0 || cx >= WORLD_X) return -1;
    if (cy < 0 || cy >= WORLD_Y) return -1;
    if (cz < 0 || cz >= WORLD_Z) return -1;
    return cx * WORLD_Y * WORLD_Z + cy * WORLD_Z + cz;
}

void VoxelWorld::GenerateTerrain() {
    // ---- Terrain height via FBm Simplex noise --------------------------------
    FastNoiseLite heightNoise;
    heightNoise.SetSeed(_seed);
    heightNoise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
    heightNoise.SetFrequency(0.0035f);
    heightNoise.SetFractalType(FastNoiseLite::FractalType_FBm);
    heightNoise.SetFractalOctaves(8);
    heightNoise.SetFractalLacunarity(2.0f);
    heightNoise.SetFractalGain(0.5f);

    // ---- Cave noise (3-D) ---------------------------------------------------
    FastNoiseLite caveNoise;
    caveNoise.SetSeed(_seed + 1);
    caveNoise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
    caveNoise.SetFrequency(0.04f);

    const int worldYVoxels = WORLD_Y * VoxelChunk::SIZE;
    const int SEA_LEVEL    = worldYVoxels / 3;  // roughly y=32

    for (int cx = 0; cx < WORLD_X; ++cx) {
        for (int cz = 0; cz < WORLD_Z; ++cz) {
            for (int lx = 0; lx < VoxelChunk::SIZE; ++lx) {
                for (int lz = 0; lz < VoxelChunk::SIZE; ++lz) {
                    int wx = cx * VoxelChunk::SIZE + lx;
                    int wz = cz * VoxelChunk::SIZE + lz;

                    // Height in world-voxel coords
                    float h = heightNoise.GetNoise((float)wx, (float)wz);
                    int   height = (int)(( h * 0.5f + 0.5f ) * worldYVoxels * 0.55f) + 10;
                    height = std::clamp(height, 0, worldYVoxels - 1);

                    for (int wy = 0; wy < height; ++wy) {
                        // Cave carving
                        float cv = caveNoise.GetNoise((float)wx, (float)wy, (float)wz);
                        if (cv > 0.55f && wy > 4) continue; // carve out cave

                        int cy = wy / VoxelChunk::SIZE;
                        int ly = wy % VoxelChunk::SIZE;
                        VoxelChunk* chunk = GetChunk(cx, cy, cz);
                        if (!chunk) continue;

                        uint8_t id;
                        if (wy == height - 1 && wy >= SEA_LEVEL) {
                            id = 2; // grass on surface above sea level
                        } else if (wy >= height - 4) {
                            id = (wy < SEA_LEVEL) ? 5 : 1; // sand below sea, dirt above
                        } else {
                            id = 3; // stone
                        }
                        chunk->SetVoxel(lx, ly, lz, id);
                    }
                }
            }
        }
    }
}

void VoxelWorld::LinkNeighbors() {
    for (int cx = 0; cx < WORLD_X; ++cx) {
        for (int cy = 0; cy < WORLD_Y; ++cy) {
            for (int cz = 0; cz < WORLD_Z; ++cz) {
                VoxelChunk* chunk = GetChunk(cx, cy, cz);
                if (!chunk) continue;

                for (int dx = -1; dx <= 1; ++dx) {
                    for (int dz = -1; dz <= 1; ++dz) {
                        if (dx == 0 && dz == 0) continue;
                        chunk->SetNeighbor(dx, dz, GetChunk(cx + dx, cy, cz + dz));
                    }
                }
            }
        }
    }
}

void VoxelWorld::RebuildDirtyChunks() {
    for (auto& chunk : _chunks) {
        if (chunk->IsDirty()) {
            chunk->RebuildMesh(_gfx);
        }
    }
}
