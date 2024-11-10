#include "mesh.hpp"
#include "config.hpp"
#include "stb_image.h"

std::map<std::string, Mesh*> Mesh::MeshList;

void PrintVertex(const Vertex& v)
{
    fmt::print("P: ({},{},{}), UV: ({},{})\n, N: ({},{},{}), T: ({},{},{}), B: ({},{},{})\n", v.position.x, v.position.y, v.position.z, v.uv.x, v.uv.y, v.normal.x, v.normal.y, v.normal.z, v.tangent.x, v.tangent.y, v.tangent.z, v.bitangent.x, v.bitangent.y, v.bitangent.z);
}

void CalculateNormalsAndTangents(std::vector<Vertex>& verts, std::vector<uint16_t>& tris)
{
    for (int i = 0; i < tris.size(); i += 3)
    {
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

Mesh::Mesh()
{
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &ibo);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &ibo);
    glDeleteVertexArrays(1, &vao);
    //delete collisionShape; // FIXME: Should delete collisionShape somewhere else before the pointer is out of scope
}

// Terrain mesh initialization
void Mesh::Initialize(MeshType type, const std::vector<Vertex>& verts)
{
    this->type = type;

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(8 * sizeof(float)));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);

    glBindVertexArray(0);

    this->vertCount = verts.size();
    this->triCount = 0;
    this->initialized = true;
}

void Mesh::Initialize(MeshType type, const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris)
{
    this->type = type;

    // Buffer binding reference: https://stackoverflow.com/questions/17332657/does-a-vao-remember-both-a-ebo-ibo-elements-or-indices-and-a-vbo
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(Vertex), verts.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(3 * sizeof(float)));
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(5 * sizeof(float)));
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(8 * sizeof(float)));
    glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)(11 * sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);
    glEnableVertexAttribArray(4);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(4 * sizeof(float)));
    glVertexAttribPointer(7, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(8 * sizeof(float)));
    glVertexAttribPointer(8, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(5);
    glEnableVertexAttribArray(6);
    glEnableVertexAttribArray(7);
    glEnableVertexAttribArray(8);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    glVertexAttribDivisor(7, 1);
    glVertexAttribDivisor(8, 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(uint16_t), tris.data(), GL_STATIC_DRAW);

    glBindVertexArray(0);

    this->vertCount = verts.size();
    this->triCount = tris.size() / 3;
    this->initialized = true;
}

Mesh* Mesh::CreateCube(const float& size)
{
    Vertex vertices[] = {
        // front
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
    GLushort triangles[] = {
        0, 1, 2,
        2, 1, 3,
        4, 5, 6,
        6, 5, 7,
        8, 9, 10,
        10, 9, 11,
        12, 13, 14,
        14, 13, 15,
        16, 17, 18,
        18, 17, 19,
        20, 21, 22,
        22, 21, 23
    };

    std::vector<Vertex> verts(vertices, vertices + 24);
    std::vector<uint16_t> tris(triangles, triangles + 36);
    CalculateNormalsAndTangents(verts, tris);

    auto cube = new Mesh();
    cube->Initialize(MeshType::MESH_PRIM, verts, tris);

    cube->bounds[0] = glm::vec3(.5f * size, .5f * size, .5f * size);
    cube->bounds[1] = glm::vec3(-.5f * size, .5f * size, .5f * size);
    cube->bounds[2] = glm::vec3(-.5f * size, -.5f * size, .5f * size);
    cube->bounds[3] = glm::vec3(.5f * size, -.5f * size, .5f * size);
    cube->bounds[4] = glm::vec3(.5f * size, .5f * size, .5f * size);
    cube->bounds[5] = glm::vec3(-.5f * size, .5f * size, .5f * size);
    cube->bounds[6] = glm::vec3(-.5f * size, -.5f * size, .5f * size);
    cube->bounds[7] = glm::vec3(.5f * size, -.5f * size, .5f * size);

    return cube;
}

// TODO: make sure the uvs are correct
Mesh* Mesh::CreateSphere(const float& radius, const int& division)
{
    float delta = (float)PI / (float)division;

    std::vector<Vertex> verts;
    std::vector<uint16_t> tris;
    verts.resize((division + 1) * (2 * division + 1));
    for (int v = 0; v <= division; ++v)
    {
        float vAngle = v * delta;
        for (int h = 0; h <= 2 * division; ++h)
        {
            float hAngle = h * delta;

            glm::vec3 pos, norm;
            if (v == 0)
            {
                pos = glm::vec3(0.f, radius, 0.f);
                norm = glm::vec3(0.f, 1.f, 0.f);
            }
            else if (v == division)
            {
                pos = glm::vec3(0.f, -radius, 0.f);
                norm = glm::vec3(0.f, -1.f, 0.f);
            }
            else
            {
                pos = glm::vec3(
                    radius * glm::sin(vAngle) * glm::cos(hAngle),
                    radius * glm::cos(vAngle),
                    radius * glm::sin(vAngle) * glm::sin(hAngle)
                );
                norm = glm::normalize(pos);
            }
            verts[(v * (2 * division + 1) + h)].position = pos;
            verts[(v * (2 * division + 1) + h)].normal = norm;
            verts[(v * (2 * division + 1) + h)].uv = glm::vec2((float)h / (float)(2 * division), 1.0f - (float)v / (float)division);
        }
    }
    tris.resize((6 * division - 6) * (2 * division));
    for (int v = 0, i = 0; v <= division - 1; ++v)
    {
        for (int h = 0; h <= 2 * division - 1; ++h)
        {
            if (v != 0) //top-left triangles except for north pole
            {
                tris[i] = (2 * division + 1) * v + h;
                tris[i + 1] = (2 * division + 1) * v + h + 1;
                tris[i + 2] = (2 * division + 1) * (v + 1) + h;
                i += 3;
            }
            if (v != division - 1) //bottom-right triangles except for south pole
            {
                tris[i] = (2 * division + 1) * (v + 1) + h;
                tris[i + 1] = (2 * division + 1) * v + h + 1;
                tris[i + 2] = (2 * division + 1) * (v + 1) + h + 1;
                i += 3;
            }
        }
    }
    CalculateNormalsAndTangents(verts, tris);

    auto sphere = new Mesh();
    sphere->Initialize(MeshType::MESH_PRIM, verts, tris);

    sphere->bounds[0] = glm::vec3(radius, radius, radius);
    sphere->bounds[1] = glm::vec3(-radius, radius, radius);
    sphere->bounds[2] = glm::vec3(-radius, -radius, radius);
    sphere->bounds[3] = glm::vec3(radius, -radius, radius);
    sphere->bounds[4] = glm::vec3(radius, radius, radius);
    sphere->bounds[5] = glm::vec3(-radius, radius, radius);
    sphere->bounds[6] = glm::vec3(-radius, -radius, radius);
    sphere->bounds[7] = glm::vec3(radius, -radius, radius);

    return sphere;
}

// TODO: make sure the uvs are correct
// resolution: number of quads along each side of the terrain
Mesh* Mesh::CreateTerrain(const float& worldSize, const int& resolution)
{
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

            verts[patchIndex * 4 + 0].position = glm::vec3(-halfWorldSize + (col + 0) * patchSize, 0.f, -halfWorldSize + (row + 0) * patchSize);
            verts[patchIndex * 4 + 0].uv = glm::vec2((row + 0)/ (float)resolution, (col + 0) / (float)resolution);

            verts[patchIndex * 4 + 1].position = glm::vec3(-halfWorldSize + (col + 0) * patchSize, 0.f, -halfWorldSize + (row + 1) * patchSize);
            verts[patchIndex * 4 + 1].uv = glm::vec2((row + 1) / (float)resolution, (col + 0) / (float)resolution);

            verts[patchIndex * 4 + 2].position = glm::vec3(-halfWorldSize + (col + 1) * patchSize, 0.f, -halfWorldSize + (row + 1) * patchSize);
            verts[patchIndex * 4 + 2].uv = glm::vec2((row + 1) / (float)resolution, (col + 1) / (float)resolution);

            verts[patchIndex * 4 + 3].position = glm::vec3(-halfWorldSize + (col + 1) * patchSize, 0.f, -halfWorldSize + (row + 0) * patchSize);
            verts[patchIndex * 4 + 3].uv = glm::vec2((row + 0) / (float)resolution, (col + 1) / (float)resolution);
        }
    }

    auto terrain = new Mesh();
    terrain->Initialize(MeshType::MESH_TERRAIN,verts);

    terrain->bounds[0] = glm::vec3(.5f * worldSize, .5f, .5f * worldSize);
    terrain->bounds[1] = glm::vec3(-.5f * worldSize, .5f, .5f * worldSize);
    terrain->bounds[2] = glm::vec3(-.5f * worldSize, -.5f, .5f * worldSize);
    terrain->bounds[3] = glm::vec3(.5f * worldSize, -.5f, .5f * worldSize);
    terrain->bounds[4] = glm::vec3(.5f * worldSize, .5f, .5f * worldSize);
    terrain->bounds[5] = glm::vec3(-.5f * worldSize, .5f, .5f * worldSize);
    terrain->bounds[6] = glm::vec3(-.5f * worldSize, -.5f, .5f * worldSize);
    terrain->bounds[7] = glm::vec3(.5f * worldSize, -.5f, .5f * worldSize);

    return terrain;
}

Mesh* Mesh::CreateCubeWithPhysics(const float& size) {
    auto cube = CreateCube(size);
    cube->_shape = new btBoxShape(0.5 * btVector3(size, size, size));
    return cube;
}

Mesh* Mesh::CreateSphereWithPhysics(const float& radius, const int& division) {
    auto sphere = CreateSphere(radius, division);
    sphere->_shape = new btSphereShape(0.5 * radius);
    return sphere;
}

Mesh* Mesh::CreateTerrainWithPhysics(const float& size, const int& resolution, const std::string& heightmap) {
    auto terrain = CreateTerrain(size, resolution);
    // terrain->_shape = new btBoxShape(btVector3(size * 0.5f, 0.2f, size * 0.5f));

    int w, h, numChannels;
    unsigned char* data = stbi_load(heightmap.c_str(), &w, &h, &numChannels, 0);
    if (data != nullptr) {
        std::vector<float> terrainData; //FIXME: this needs to be stored somewhere, or the collision shape data will be deleted

        const int terrainDataSize = w * h;
        terrainData.resize(terrainDataSize);
        for (int i = 0; i < terrainDataSize; ++i) {
            terrainData[i] = (static_cast<float>(data[i]) / 255.0f) * 32.0f;
        }
        terrain->_shape = new btHeightfieldTerrainShape(w, h, terrainData.data(), 1.0f, -64.0f, 64.0f, 1, PHY_FLOAT, true);
        terrain->_shape->setLocalScaling(btVector3(10.0f, 1.0f, 10.0f));

        stbi_image_free(data);
    } else {
        throw std::runtime_error("Could not load heightmap");
    }

    return terrain;
}

void Mesh::SetMaterial(Material* material) {
    _material = material;
}

void Mesh::AddCapsuleShape(float radius, float height) {
    _shape = new btCapsuleShape(radius, height);
}