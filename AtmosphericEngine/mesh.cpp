#include "mesh.hpp"

std::map<std::string, Mesh*> Mesh::MeshList;

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

    program.SetUniform(std::string("surf.ambient"), material->ambient);
    program.SetUniform(std::string("surf.diffuse"), material->diffuse);
    program.SetUniform(std::string("surf.specular"), material->specular);
    program.SetUniform(std::string("surf.shininess"), material->shininess);
    program.SetUniform(std::string("surf.albedo"), material->albedo);
    program.SetUniform(std::string("surf.metallic"), material->metallic);
    program.SetUniform(std::string("surf.roughness"), material->roughness);
    program.SetUniform(std::string("surf.ao"), material->ao);
    program.SetUniform(std::string("tex_unit"), (int)material->GetTexUnit());

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
        //left
        { { .5f * size, .5f * size, .5f * size }, { 1.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { .5f * size, -.5f * size, .5f * size }, { 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -.5f * size, .5f * size, .5f * size }, { 0.0f, 1.0f }, { 0.0f, 0.0f, 1.0f } },
        { { -.5f * size, -.5f * size, .5f * size }, { 0.0f, 0.0f }, { 0.0f, 0.0f, 1.0f } },
        //right
        { { .5f * size, .5f * size, -.5f * size }, { 1.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { { .5f * size, -.5f * size, -.5f * size }, { 1.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -.5f * size, .5f * size, -.5f * size }, { 0.0f, 1.0f }, { 0.0f, 0.0f, -1.0f } },
        { { -.5f * size, -.5f * size, -.5f * size }, { 0.0f, 0.0f }, { 0.0f, 0.0f, -1.0f } },
        //back
        { { -.5f * size, .5f * size, .5f * size }, { 1.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -.5f * size, .5f * size, -.5f * size }, { 1.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -.5f * size, -.5f * size, .5f * size }, { 0.0f, 1.0f }, { -1.0f, 0.0f, 0.0f } },
        { { -.5f * size, -.5f * size, -.5f * size }, { 0.0f, 0.0f }, { -1.0f, 0.0f, 0.0f } },
        //front
        { { .5f * size, .5f * size, .5f * size }, { 1.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
        { { .5f * size, .5f * size, -.5f * size }, { 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f } },
        { { .5f * size, -.5f * size, .5f * size }, { 1.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        { { .5f * size, -.5f * size, -.5f * size }, { 0.0f, 0.0f }, { 1.0f, 0.0f, 0.0f } },
        //top
        { { .5f * size, .5f * size, .5f * size }, { 1.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { { .5f * size, .5f * size, -.5f * size }, { 1.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -.5f * size, .5f * size, .5f * size }, { 0.0f, 1.0f }, { 0.0f, 1.0f, 0.0f } },
        { { -.5f * size, .5f * size, -.5f * size }, { 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f } },
        //bottom
        { { .5f * size, -.5f * size, .5f * size }, { 1.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
        { { .5f * size, -.5f * size, -.5f * size }, { 1.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } },
        { { -.5f * size, -.5f * size, .5f * size }, { 0.0f, 1.0f }, { 0.0f, -1.0f, 0.0f } },
        { { -.5f * size, -.5f * size, -.5f * size }, { 0.0f, 0.0f }, { 0.0f, -1.0f, 0.0f } }
    };
    GLushort triangles[] = {
        0, 2, 1,
        1, 2, 3,
        4, 5, 6,
        6, 5, 7,
        8, 9, 10,
        10, 9, 11,
        12, 14, 13,
        13, 14, 15,
        16, 17, 18,
        18, 17, 19,
        20, 22, 21,
        21, 22, 23
    };

    std::vector<Vertex> verts(vertices, vertices + 24);
    std::vector<uint16_t> tris(triangles, triangles + 36);

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