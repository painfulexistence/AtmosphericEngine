#include "mesh.hpp"
#include "graphics_config.hpp"

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

void Mesh::Initialize(const std::vector<Vertex>& verts, const std::vector<uint16_t>& tris)
{
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
    this->_initialized = true;
}

void Mesh::Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const
{
    if (!_initialized)
        throw std::runtime_error("Buffer object contains no data");

    glEnable(GL_PRIMITIVE_RESTART);
    if (cullFaceEnabled)
        glEnable(GL_CULL_FACE);
    else
        glDisable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_STENCIL_TEST);
    glDepthFunc(GL_LESS);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //glStencilFunc(GL_ALWAYS, 1, 0xFF);
    glPolygonMode(GL_FRONT_AND_BACK, drawMode);

    // Surface parameters
    program.SetUniform(std::string("surf_params.diffuse"), material->diffuse);
    program.SetUniform(std::string("surf_params.specular"), material->specular);
    program.SetUniform(std::string("surf_params.ambient"), material->ambient);
    program.SetUniform(std::string("surf_params.shininess"), material->shininess);

    // Material textures
    if (material->baseMap >= 0) {
        program.SetUniform(std::string("base_map_unit"), NUM_MAP_UNITS + material->baseMap);
    } else {
        program.SetUniform(std::string("base_map_unit"), NUM_MAP_UNITS + 0);
    }
    if (material->normalMap >= 0) {
        program.SetUniform(std::string("normal_map_unit"), NUM_MAP_UNITS + material->normalMap);
    } else {
        program.SetUniform(std::string("normal_map_unit"), NUM_MAP_UNITS + 1);
    }
    if (material->aoMap >= 0) {
        program.SetUniform(std::string("ao_map_unit"), NUM_MAP_UNITS + material->aoMap);
    } else {
        program.SetUniform(std::string("ao_map_unit"), NUM_MAP_UNITS + 2);
    }
    if (material->roughnessMap >= 0) {
        program.SetUniform(std::string("roughness_map_unit"), NUM_MAP_UNITS + material->roughnessMap);
    } else {
        program.SetUniform(std::string("roughness_map_unit"), NUM_MAP_UNITS + 3);
    }
    if (material->metallicMap >= 0) {
        program.SetUniform(std::string("metallic_map_unit"), NUM_MAP_UNITS + material->metallicMap);
    } else {
        program.SetUniform(std::string("metallic_map_unit"), NUM_MAP_UNITS + 4);
    }

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, worldMatrices.size() * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);
    glDrawElementsInstanced(primitiveType, triCount * 3, GL_UNSIGNED_SHORT, 0, worldMatrices.size());
    glBindVertexArray(0);
}

void Mesh::Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const
{
    if (!_initialized)
        throw std::runtime_error("Buffer object contains no data");

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
    /*
    pass 1
    ...
     */
    glStencilMask(0x00);
    glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
    glDepthFunc(GL_ALWAYS);
    /*
    pass 2 (scaled)
    ...
     */
    glDepthFunc(GL_LESS);

    glStencilMask(0xFF);
    glStencilFunc(GL_ALWAYS, 1, 0xFF);
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
    cube->Initialize(verts, tris);

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
    sphere->Initialize(verts, tris);

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
Mesh* Mesh::CreateTerrain(const float& size, const int& vnum, const std::vector<float>& heightmap)
{
    std::vector<Vertex> verts;
    std::vector<uint16_t> tris;

    verts.resize(vnum * vnum);
    float c_size = size / float(vnum - 1);
    for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
            float x = -.5f * size + j * c_size;
            float y = heightmap[(i * vnum + j)];
            float z = -.5f * size + i * c_size;
            verts[(i * vnum + j)].position = glm::vec3(x, y, z);
            verts[(i * vnum + j)].normal = glm::vec3(0.0f, 1.0f, 0.0f); // glm::vec3(2 * float(i)/float(vnum), 2 * float(j)/float(vnum), 0.8)
            verts[(i * vnum + j)].uv = glm::vec2((float)(i % 2), (float)(j % 2));
        }
    }
    tris.resize((vnum * 2 + 1) * (vnum - 1));
    for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
            tris[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        tris[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
    }
    CalculateNormalsAndTangents(verts, tris);

    auto terrain = new Mesh();
    terrain->Initialize(verts, tris);
    terrain->primitiveType = GL_TRIANGLE_STRIP;

    terrain->bounds[0] = glm::vec3(.5f * size, .5f, .5f * size);
    terrain->bounds[1] = glm::vec3(-.5f * size, .5f, .5f * size);
    terrain->bounds[2] = glm::vec3(-.5f * size, -.5f, .5f * size);
    terrain->bounds[3] = glm::vec3(.5f * size, -.5f, .5f * size);
    terrain->bounds[4] = glm::vec3(.5f * size, .5f, .5f * size);
    terrain->bounds[5] = glm::vec3(-.5f * size, .5f, .5f * size);
    terrain->bounds[6] = glm::vec3(-.5f * size, -.5f, .5f * size);
    terrain->bounds[7] = glm::vec3(.5f * size, -.5f, .5f * size);

    return terrain;
}