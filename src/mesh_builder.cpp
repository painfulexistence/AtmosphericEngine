#include "mesh_builder.hpp"
#include "asset_manager.hpp"
#include "mesh.hpp"

void CalculateNormalsAndTangents(std::vector<Vertex>& verts, std::vector<uint16_t>& tris) {
    for (int i = 0; i < tris.size(); i += 3) {
        glm::vec3 edge1 = verts[tris[i]].position - verts[tris[i + 1]].position;
        glm::vec3 edge2 = verts[tris[i + 2]].position - verts[tris[i + 1]].position;
        glm::vec2 dUV1 = verts[tris[i]].uv - verts[tris[i + 1]].uv;
        glm::vec2 dUV2 = verts[tris[i + 2]].uv - verts[tris[i + 1]].uv;
        // Calculate normals
        glm::vec3 normal = glm::normalize(glm::cross(edge2, edge1));
        verts[tris[i]].normal = normal;
        verts[tris[i + 1]].normal = normal;
        verts[tris[i + 2]].normal = normal;
        // Calculate tangents & bitangents
        float f = 1.0f / (dUV1.x * dUV2.y - dUV1.y * dUV2.x);
        glm::vec3 tangent = f * (dUV2.y * edge1 - dUV1.y * edge2);
        verts[tris[i]].tangent = tangent;
        verts[tris[i + 1]].tangent = tangent;
        verts[tris[i + 2]].tangent = tangent;
        glm::vec3 bitangent = f * (dUV1.x * edge2 - dUV2.x * edge1);
        verts[tris[i]].bitangent = bitangent;
        verts[tris[i + 1]].bitangent = bitangent;
        verts[tris[i + 2]].bitangent = bitangent;
        // For TBN debugging
        // if (glm::dot(glm::cross(tangent, bitangent), normal) < 0.0f) {
        //     PrintVertex(verts[tris[i]]);
        //     PrintVertex(verts[tris[i + 1]]);
        //     PrintVertex(verts[tris[i + 2]]);
        //     throw std::runtime_error("Triangle is degenerate");
        // }
    }
    // (u0 - u1) * T + (v0 - v1) * B = p0 - p1
    // (u2 - u1) * T + (v2 - v1) * B = p2 - p1
    // det = (u0 - u1) * (v2 - v1) - (v0 - v1) * (u2 - u1)
    // T = ((v2 - v1) * (p0 - p1)  - (v0 - v1) * (p2 - p1)) / det
    // B = ((u0 - u1) * (p2 - p1) - (u2 - u1) * (p0 - p1)) / det
}

Mesh* MeshBuilder::CreateCube(const float& size) {
    Vertex vertices[] = { // front
                          { { .5f * size, .5f * size, .5f * size }, { 1.f, 1.f }, { 0.0f, 0.0f, 1.0f } },
                          { { -.5f * size, .5f * size, .5f * size }, { 0.f, 1.f }, { 0.0f, 0.0f, 1.0f } },
                          { { .5f * size, -.5f * size, .5f * size }, { 1.f, 0.f }, { 0.0f, 0.0f, 1.0f } },
                          { { -.5f * size, -.5f * size, .5f * size }, { 0.f, 0.f }, { 0.0f, 0.0f, 1.0f } },
                          // back
                          { { -.5f * size, .5f * size, -.5f * size }, { 1.f, 1.f }, { 0.0f, 0.0f, -1.0f } },
                          { { .5f * size, .5f * size, -.5f * size }, { 0.f, 1.f }, { 0.0f, 0.0f, -1.0f } },
                          { { -.5f * size, -.5f * size, -.5f * size }, { 1.f, 0.f }, { 0.0f, 0.0f, -1.0f } },
                          { { .5f * size, -.5f * size, -.5f * size }, { 0.f, 0.f }, { 0.0f, 0.0f, -1.0f } },
                          // right
                          { { .5f * size, .5f * size, -.5f * size }, { 1.f, 1.f }, { 1.0f, 0.0f, 0.0f } },
                          { { .5f * size, .5f * size, .5f * size }, { 0.f, 1.f }, { 1.0f, 0.0f, 0.0f } },
                          { { .5f * size, -.5f * size, -.5f * size }, { 1.f, 0.f }, { 1.0f, 0.0f, 0.0f } },
                          { { .5f * size, -.5f * size, .5f * size }, { 0.f, 0.f }, { 1.0f, 0.0f, 0.0f } },
                          // left
                          { { -.5f * size, .5f * size, .5f * size }, { 1.f, 1.f }, { -1.0f, 0.0f, 0.0f } },
                          { { -.5f * size, .5f * size, -.5f * size }, { 0.f, 1.f }, { -1.0f, 0.0f, 0.0f } },
                          { { -.5f * size, -.5f * size, .5f * size }, { 1.f, 0.f }, { -1.0f, 0.0f, 0.0f } },
                          { { -.5f * size, -.5f * size, -.5f * size }, { 0.f, 0.f }, { -1.0f, 0.0f, 0.0f } },
                          // top
                          { { .5f * size, .5f * size, -.5f * size }, { 1.f, 1.f }, { 0.0f, 1.0f, 0.0f } },
                          { { -.5f * size, .5f * size, -.5f * size }, { 0.f, 1.f }, { 0.0f, 1.0f, 0.0f } },
                          { { .5f * size, .5f * size, .5f * size }, { 1.f, 0.f }, { 0.0f, 1.0f, 0.0f } },
                          { { -.5f * size, .5f * size, .5f * size }, { 0.f, 0.f }, { 0.0f, 1.0f, 0.0f } },
                          // bottom
                          { { .5f * size, -.5f * size, .5f * size }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
                          { { -.5f * size, -.5f * size, .5f * size }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
                          { { .5f * size, -.5f * size, -.5f * size }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
                          { { -.5f * size, -.5f * size, -.5f * size }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } }
    };
    GLushort triangles[] = { 0,  1,  2,  2,  1,  3,  4,  5,  6,  6,  5,  7,  8,  9,  10, 10, 9,  11,
                             12, 13, 14, 14, 13, 15, 16, 17, 18, 18, 17, 19, 20, 21, 22, 22, 21, 23 };

    std::vector<Vertex> verts(vertices, vertices + 24);
    std::vector<uint16_t> tris(triangles, triangles + 36);
    CalculateNormalsAndTangents(verts, tris);

    auto cube = new Mesh(MeshType::PRIM);
    cube->Initialize(verts, tris);
    cube->SetBoundingBox({ { glm::vec3(.5f * size, .5f * size, .5f * size),
                             glm::vec3(-.5f * size, .5f * size, .5f * size),
                             glm::vec3(-.5f * size, -.5f * size, .5f * size),
                             glm::vec3(.5f * size, -.5f * size, .5f * size),
                             glm::vec3(.5f * size, .5f * size, .5f * size),
                             glm::vec3(-.5f * size, .5f * size, .5f * size),
                             glm::vec3(-.5f * size, -.5f * size, .5f * size),
                             glm::vec3(.5f * size, -.5f * size, .5f * size) } });
    return cube;
}

Mesh* MeshBuilder::CreatePlane(float width, float height) {
    float hw = width * 0.5f;
    float hh = height * 0.5f;

    // Normal points up (Y)
    Vertex vertices[] = {
        { { -hw, 0.0f, hh }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },// Bottom-Left (Front-Left)
        { { -hw, 0.0f, -hh }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },// Top-Left (Back-Left)
        { { hw, 0.0f, -hh }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },// Top-Right (Back-Right)
        { { hw, 0.0f, hh }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } }// Bottom-Right (Front-Right)
    };

    // CCW Winding
    uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

    std::vector<Vertex> verts(vertices, vertices + 4);
    std::vector<uint16_t> tris(indices, indices + 6);
    CalculateNormalsAndTangents(verts, tris);

    auto plane = new Mesh(MeshType::PRIM);
    plane->Initialize(verts, tris);
    plane->SetBoundingBox({ { glm::vec3(hw, 0.0f, hh),
                              glm::vec3(-hw, 0.0f, hh),
                              glm::vec3(-hw, 0.0f, -hh),
                              glm::vec3(hw, 0.0f, -hh),
                              glm::vec3(hw, 0.0f, hh),
                              glm::vec3(-hw, 0.0f, hh),
                              glm::vec3(-hw, 0.0f, -hh),
                              glm::vec3(hw, 0.0f, -hh) } });
    return plane;
}

// TODO: make sure the uvs are correct
Mesh* MeshBuilder::CreateSphere(const float& radius, const int& division) {
    float delta = (float)PI / (float)division;

    std::vector<Vertex> verts;
    std::vector<uint16_t> tris;
    verts.resize((division + 1) * (2 * division + 1));
    for (int v = 0; v <= division; ++v) {
        float vAngle = v * delta;
        for (int h = 0; h <= 2 * division; ++h) {
            float hAngle = h * delta;

            glm::vec3 pos, norm;
            if (v == 0) {
                pos = glm::vec3(0.f, radius, 0.f);
                norm = glm::vec3(0.f, 1.f, 0.f);
            } else if (v == division) {
                pos = glm::vec3(0.f, -radius, 0.f);
                norm = glm::vec3(0.f, -1.f, 0.f);
            } else {
                pos = glm::vec3(
                  radius * glm::sin(vAngle) * glm::cos(hAngle),
                  radius * glm::cos(vAngle),
                  radius * glm::sin(vAngle) * glm::sin(hAngle)
                );
                norm = glm::normalize(pos);
            }
            verts[(v * (2 * division + 1) + h)].position = pos;
            verts[(v * (2 * division + 1) + h)].normal = norm;
            verts[(v * (2 * division + 1) + h)].uv =
              glm::vec2((float)h / (float)(2 * division), 1.0f - (float)v / (float)division);
        }
    }
    tris.resize((6 * division - 6) * (2 * division));
    for (int v = 0, i = 0; v <= division - 1; ++v) {
        for (int h = 0; h <= 2 * division - 1; ++h) {
            if (v != 0)// top-left triangles except for north pole
            {
                tris[i] = (2 * division + 1) * v + h;
                tris[i + 1] = (2 * division + 1) * v + h + 1;
                tris[i + 2] = (2 * division + 1) * (v + 1) + h;
                i += 3;
            }
            if (v != division - 1)// bottom-right triangles except for south pole
            {
                tris[i] = (2 * division + 1) * (v + 1) + h;
                tris[i + 1] = (2 * division + 1) * v + h + 1;
                tris[i + 2] = (2 * division + 1) * (v + 1) + h + 1;
                i += 3;
            }
        }
    }
    CalculateNormalsAndTangents(verts, tris);

    auto sphere = new Mesh(MeshType::PRIM);
    sphere->Initialize(verts, tris);
    sphere->SetBoundingBox({ { glm::vec3(radius, radius, radius),
                               glm::vec3(-radius, radius, radius),
                               glm::vec3(-radius, -radius, radius),
                               glm::vec3(radius, -radius, radius),
                               glm::vec3(radius, radius, radius),
                               glm::vec3(-radius, radius, radius),
                               glm::vec3(-radius, -radius, radius),
                               glm::vec3(radius, -radius, radius) } });
    return sphere;
}

// TODO: make sure the uvs are correct
// resolution: number of quads along each side of the terrain
Mesh* MeshBuilder::CreateTerrain(const float& worldSize, const int& resolution) {
    std::vector<Vertex> verts;
    verts.resize(resolution * resolution * 4);

    // resolution = 3
    // |-------|-------|-------|   X
    // |       |       |       |
    // |   p0  |   p1  |   p2  |
    // |-------|-------|-------|
    // |       |       |       |
    // |   p3  |       |       |
    // |-------|-------|-------|
    // |       |       |       |
    // |       |       |       |
    // |-------|-------|-------|
    //
    // Z

    float halfWorldSize = worldSize / 2.f;
    float patchSize = worldSize / float(resolution);
    for (int row = 0; row < resolution; row++) {
        for (int col = 0; col < resolution; col++) {
            int patchIndex = row * resolution + col;

            verts[patchIndex * 4 + 0].position =
              glm::vec3(-halfWorldSize + (col + 0) * patchSize, 0.f, -halfWorldSize + (row + 0) * patchSize);
            verts[patchIndex * 4 + 0].uv = glm::vec2((row + 0) / (float)resolution, (col + 0) / (float)resolution);

            verts[patchIndex * 4 + 1].position =
              glm::vec3(-halfWorldSize + (col + 0) * patchSize, 0.f, -halfWorldSize + (row + 1) * patchSize);
            verts[patchIndex * 4 + 1].uv = glm::vec2((row + 1) / (float)resolution, (col + 0) / (float)resolution);

            verts[patchIndex * 4 + 2].position =
              glm::vec3(-halfWorldSize + (col + 1) * patchSize, 0.f, -halfWorldSize + (row + 1) * patchSize);
            verts[patchIndex * 4 + 2].uv = glm::vec2((row + 1) / (float)resolution, (col + 1) / (float)resolution);

            verts[patchIndex * 4 + 3].position =
              glm::vec3(-halfWorldSize + (col + 1) * patchSize, 0.f, -halfWorldSize + (row + 0) * patchSize);
            verts[patchIndex * 4 + 3].uv = glm::vec2((row + 0) / (float)resolution, (col + 1) / (float)resolution);
        }
    }

    auto terrain = new Mesh(MeshType::TERRAIN);
    terrain->Initialize(verts);
    terrain->SetBoundingBox({ { glm::vec3(.5f * worldSize, .5f, .5f * worldSize),
                                glm::vec3(-.5f * worldSize, .5f, .5f * worldSize),
                                glm::vec3(-.5f * worldSize, -.5f, .5f * worldSize),
                                glm::vec3(.5f * worldSize, -.5f, .5f * worldSize),
                                glm::vec3(.5f * worldSize, .5f, .5f * worldSize),
                                glm::vec3(-.5f * worldSize, .5f, .5f * worldSize),
                                glm::vec3(-.5f * worldSize, -.5f, .5f * worldSize),
                                glm::vec3(.5f * worldSize, -.5f, .5f * worldSize) } });
    return terrain;
}

Mesh* MeshBuilder::CreateCubeWithPhysics(const float& size) {
    auto cube = CreateCube(size);
    cube->SetShape(new btBoxShape(0.5 * btVector3(size, size, size)));
    return cube;
}

Mesh* MeshBuilder::CreateSphereWithPhysics(const float& radius, const int& division) {
    auto sphere = CreateSphere(radius, division);
    sphere->SetShape(new btSphereShape(radius));
    return sphere;
}

Mesh* MeshBuilder::CreateTerrainWithPhysics(const float& size, const int& resolution, const std::string& heightmap) {
    auto terrain = CreateTerrain(size, resolution);
    // terrain->SetShape(new btBoxShape(btVector3(size * 0.5f, 0.2f, size * 0.5f)));

    auto img = AssetManager::Get().LoadImage(heightmap);
    if (img != nullptr) {
        std::vector<float>
          terrainData;// FIXME: this needs to be stored somewhere, or the collision shape data will be deleted

        const int terrainDataSize = img->width * img->height;
        terrainData.resize(terrainDataSize);
        for (int i = 0; i < terrainDataSize; ++i) {
            terrainData[i] = (static_cast<float>(img->byteArray[i]) / 255.0f) * 32.0f;
        }
        terrain->SetShape(new btHeightfieldTerrainShape(
          img->width, img->height, terrainData.data(), 1.0f, -64.0f, 64.0f, 1, PHY_FLOAT, true
        ));
        terrain->SetShapeLocalScaling(glm::vec3(10.0f, 1.0f, 10.0f));
    }

    return terrain;
}

void MeshBuilder::PushQuad(
  glm::vec3 position, glm::vec2 size, glm::vec3 normal, glm::quat rotation, glm::vec2 uvMin, glm::vec2 uvMax
) {
    uint32_t baseIndex = static_cast<uint32_t>(vertices.size());

    glm::vec3 tangent = glm::normalize(glm::cross(normal, glm::vec3(0, 1, 0)));
    if (glm::length(tangent) < 0.01f) {
        tangent = glm::normalize(glm::cross(normal, glm::vec3(1, 0, 0)));
    }
    glm::vec3 bitangent = glm::cross(normal, tangent);

    vertices.push_back(
      { { position.x - size.x / 2, position.y - size.y / 2, position.z }, { 0, 0 }, normal, tangent, bitangent }
    );
    vertices.push_back(
      { { position.x - size.x / 2, position.y + size.y / 2, position.z }, { 0, 1 }, normal, tangent, bitangent }
    );
    vertices.push_back(
      { { position.x + size.x / 2, position.y + size.y / 2, position.z }, { 1, 1 }, normal, tangent, bitangent }
    );
    vertices.push_back(
      { { position.x + size.x / 2, position.y - size.y / 2, position.z }, { 1, 0 }, normal, tangent, bitangent }
    );

    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 1);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 0);
    indices.push_back(baseIndex + 2);
    indices.push_back(baseIndex + 3);
}

void MeshBuilder::PushCube(glm::vec3 position, glm::vec3 size, glm::quat rotation) {
    // Front face
    PushQuad(position + glm::vec3(0, 0, size.z / 2), glm::vec2(size.x, size.y), glm::vec3(0, 0, 1));

    // Back face
    PushQuad(position + glm::vec3(0, 0, -size.z / 2), glm::vec2(size.x, size.y), glm::vec3(0, 0, -1));

    // Right face
    PushQuad(position + glm::vec3(size.x / 2, 0, 0), glm::vec2(size.z, size.y), glm::vec3(1, 0, 0));

    // Left face
    PushQuad(position + glm::vec3(-size.x / 2, 0, 0), glm::vec2(size.z, size.y), glm::vec3(-1, 0, 0));

    // Top face
    PushQuad(position + glm::vec3(0, size.y / 2, 0), glm::vec2(size.x, size.z), glm::vec3(0, 1, 0));

    // Bottom face
    PushQuad(position + glm::vec3(0, -size.y / 2, 0), glm::vec2(size.x, size.z), glm::vec3(0, -1, 0));
}

void MeshBuilder::PushCSG(const std::vector<CSG::AABB>& boxes) {
    for (const auto& box : boxes) {
        PushCube(box.GetCenter(), box.GetSize());
    }
}

void MeshBuilder::Clear() {
    vertices.clear();
    indices.clear();
}

std::shared_ptr<Mesh> MeshBuilder::Build() {
    // CalculateNormalsAndTangents(vertices, indices);

    auto mesh = std::make_shared<Mesh>(MeshType::PRIM);
    mesh->Initialize(vertices, indices);
    // Calculate bounds
    if (!vertices.empty()) {
        glm::vec3 min = vertices[0].position;
        glm::vec3 max = vertices[0].position;
        for (const auto& v : vertices) {
            min = glm::min(min, v.position);
            max = glm::max(max, v.position);
        }
        std::array<glm::vec3, 8> bounds = { glm::vec3(min.x, min.y, min.z), glm::vec3(max.x, min.y, min.z),
                                            glm::vec3(min.x, max.y, min.z), glm::vec3(max.x, max.y, min.z),
                                            glm::vec3(min.x, min.y, max.z), glm::vec3(max.x, min.y, max.z),
                                            glm::vec3(min.x, max.y, max.z), glm::vec3(max.x, max.y, max.z) };
        mesh->SetBoundingBox(bounds);
    }
    return mesh;
}
// VoxelMeshBuilder implementation

void VoxelMeshBuilder::PushFace(glm::ivec3 pos, FaceDir dir, uint8_t voxelId) {
    uint8_t x = static_cast<uint8_t>(pos.x);
    uint8_t y = static_cast<uint8_t>(pos.y);
    uint8_t z = static_cast<uint8_t>(pos.z);
    uint8_t faceId = static_cast<uint8_t>(dir);

    // Each face has 4 corners, we generate 6 vertices (2 triangles)
    // Vertex order: v0, v1, v2, v2, v3, v0 (two triangles sharing edge v0-v2)
    VoxelVertex v0, v1, v2, v3;

    switch (dir) {
    case FaceDir::TOP:// +Y
        v0 = { x, static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        v1 = { x, static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        v2 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        v3 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        break;

    case FaceDir::BOTTOM:// -Y
        v0 = { static_cast<uint8_t>(x + 1), y, z, voxelId, faceId };
        v1 = { static_cast<uint8_t>(x + 1), y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v2 = { x, y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v3 = { x, y, z, voxelId, faceId };
        break;

    case FaceDir::RIGHT:// +X
        v0 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        v1 = { static_cast<uint8_t>(x + 1), y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v2 = { static_cast<uint8_t>(x + 1), y, z, voxelId, faceId };
        v3 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        break;

    case FaceDir::LEFT:// -X
        v0 = { x, static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        v1 = { x, y, z, voxelId, faceId };
        v2 = { x, y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v3 = { x, static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        break;

    case FaceDir::FRONT:// +Z
        v0 = { x, static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        v1 = { x, y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v2 = { static_cast<uint8_t>(x + 1), y, static_cast<uint8_t>(z + 1), voxelId, faceId };
        v3 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), static_cast<uint8_t>(z + 1), voxelId, faceId };
        break;

    case FaceDir::BACK:// -Z
        v0 = { static_cast<uint8_t>(x + 1), static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        v1 = { static_cast<uint8_t>(x + 1), y, z, voxelId, faceId };
        v2 = { x, y, z, voxelId, faceId };
        v3 = { x, static_cast<uint8_t>(y + 1), z, voxelId, faceId };
        break;
    }

    // Two triangles: (v0, v1, v2) and (v2, v3, v0)
    _vertices.push_back(v0);
    _vertices.push_back(v1);
    _vertices.push_back(v2);
    _vertices.push_back(v2);
    _vertices.push_back(v3);
    _vertices.push_back(v0);
}

void VoxelMeshBuilder::PushCube(glm::ivec3 pos, uint8_t voxelId) {
    PushFace(pos, FaceDir::TOP, voxelId);
    PushFace(pos, FaceDir::BOTTOM, voxelId);
    PushFace(pos, FaceDir::RIGHT, voxelId);
    PushFace(pos, FaceDir::LEFT, voxelId);
    PushFace(pos, FaceDir::FRONT, voxelId);
    PushFace(pos, FaceDir::BACK, voxelId);
}

void VoxelMeshBuilder::Clear() {
    _vertices.clear();
}