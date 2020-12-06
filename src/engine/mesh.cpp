#include "mesh.hpp"

Mesh::Mesh()
{
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &ibo);
}

Mesh::~Mesh()
{
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ebo);
    glDeleteBuffers(1, &ibo);
}

void Mesh::AsCube(const GLfloat& size)
{
    GLfloat vertices[] = {
        //left
        .5f * size, .5f * size, .5f * size, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        .5f * size, -.5f * size, .5f * size, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -.5f * size, .5f * size, .5f * size, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -.5f * size, -.5f * size, .5f * size, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        //right
        .5f * size, .5f * size, -.5f * size, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        .5f * size, -.5f * size, -.5f * size, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -.5f * size, .5f * size, -.5f * size, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -.5f * size, -.5f * size, -.5f * size, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        //back
        -.5f * size, .5f * size, .5f * size, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -.5f * size, .5f * size, -.5f * size, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -.5f * size, -.5f * size, .5f * size, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -.5f * size, -.5f * size, -.5f * size, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //front
        .5f * size, .5f * size, .5f * size, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        .5f * size, .5f * size, -.5f * size, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        .5f * size, -.5f * size, .5f * size, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        .5f * size, -.5f * size, -.5f * size, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //top
        .5f * size, .5f * size, .5f * size, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        .5f * size, .5f * size, -.5f * size, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -.5f * size, .5f * size, .5f * size, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -.5f * size, .5f * size, -.5f * size, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        //bottom
        .5f * size, -.5f * size, .5f * size, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        .5f * size, -.5f * size, -.5f * size, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -.5f * size, -.5f * size, .5f * size, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -.5f * size, -.5f * size, -.5f * size, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };
    verts.assign(vertices, vertices + 192);

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
    tris.assign(triangles, triangles + 36);

    bounds[0] = glm::vec3(.5f * size, .5f * size, .5f * size);
    bounds[1] = glm::vec3(-.5f * size, .5f * size, .5f * size);
    bounds[2] = glm::vec3(-.5f * size, -.5f * size, .5f * size);
    bounds[3] = glm::vec3(.5f * size, -.5f * size, .5f * size);
    bounds[4] = glm::vec3(.5f * size, .5f * size, .5f * size);
    bounds[5] = glm::vec3(-.5f * size, .5f * size, .5f * size);
    bounds[6] = glm::vec3(-.5f * size, -.5f * size, .5f * size);
    bounds[7] = glm::vec3(.5f * size, -.5f * size, .5f * size);

    _initialized = true;
    BufferData();
}

void Mesh::AsSphere(const GLfloat& radius, const GLint& division)
{
    float delta = (float)PI / (float)division;

    verts.resize(8 * (division + 1) * (2 * division + 1));
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
            verts[8 * (v * (2 * division + 1) + h)] = pos.x;
            verts[8 * (v * (2 * division + 1) + h) + 1] = pos.y;
            verts[8 * (v * (2 * division + 1) + h) + 2] = pos.z;
            verts[8 * (v * (2 * division + 1) + h) + 3] = norm.x;
            verts[8 * (v * (2 * division + 1) + h) + 4] = norm.y;
            verts[8 * (v * (2 * division + 1) + h) + 5] = norm.z;
            verts[8 * (v * (2 * division + 1) + h) + 6] = (float)h / (float)(2 * division);
            verts[8 * (v * (2 * division + 1) + h) + 7] = 1.0f - (float)v / (float)division;
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

    bounds[0] = glm::vec3(radius, radius, radius);
    bounds[1] = glm::vec3(-radius, radius, radius);
    bounds[2] = glm::vec3(-radius, -radius, radius);
    bounds[3] = glm::vec3(radius, -radius, radius);
    bounds[4] = glm::vec3(radius, radius, radius);
    bounds[5] = glm::vec3(-radius, radius, radius);
    bounds[6] = glm::vec3(-radius, -radius, radius);
    bounds[7] = glm::vec3(radius, -radius, radius);

    _initialized = true;
    BufferData();
}

void Mesh::AsTerrain(const GLfloat& size, const GLint& vnum, const std::vector<GLfloat>& heightmap)
{
    verts.resize(vnum * vnum * 8);
    float c_size = size / float(vnum - 1);
    for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
            float x = -.5f * size + j * c_size;
            float y = heightmap[(i * vnum + j)];
            float z = -.5f * size + i * c_size;
            verts[(i * vnum + j) * 8 + 0] = x;
            verts[(i * vnum + j) * 8 + 1] = y;
            verts[(i * vnum + j) * 8 + 2] = z;
            verts[(i * vnum + j) * 8 + 3] = 0.f; //2 * float(i)/float(vnum);
            verts[(i * vnum + j) * 8 + 4] = 1.f; //2 * float(j)/float(vnum);
            verts[(i * vnum + j) * 8 + 5] = 0.f; //0.8f;
            verts[(i * vnum + j) * 8 + 6] = (float)(i%2);
            verts[(i * vnum + j) * 8 + 7] = (float)(j%2);
        }
    }

    tris.resize((vnum * 2 + 1) * (vnum - 1));
    primitiveType = GL_TRIANGLE_STRIP;
    for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
            tris[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        tris[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
    }

    bounds[0] = glm::vec3(.5f * size, .5f, .5f * size);
    bounds[1] = glm::vec3(-.5f * size, .5f, .5f * size);
    bounds[2] = glm::vec3(-.5f * size, -.5f, .5f * size);
    bounds[3] = glm::vec3(.5f * size, -.5f, .5f * size);
    bounds[4] = glm::vec3(.5f * size, .5f, .5f * size);
    bounds[5] = glm::vec3(-.5f * size, .5f, .5f * size);
    bounds[6] = glm::vec3(-.5f * size, -.5f, .5f * size);
    bounds[7] = glm::vec3(.5f * size, -.5f, .5f * size);
    
    _initialized = true;
    BufferData();
}

void Mesh::BufferData()
{
    // Bind/fill vertex buffer and element buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, verts.size() * sizeof(GLfloat), verts.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, tris.size() * sizeof(GLushort), tris.data(), GL_STATIC_DRAW);
}

void Mesh::Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices) const
{
    if (!_initialized)
        throw std::runtime_error("VAO not initialized");
    
    glEnable(GL_PRIMITIVE_RESTART);
    if (cullFaceEnabled) {
        glEnable(GL_CULL_FACE);
    } else {
        glDisable(GL_CULL_FACE);
    }
    glEnable(GL_DEPTH_TEST);
    //glEnable(GL_STENCIL_TEST);

    glDepthFunc(GL_LESS);
    //glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
    //glStencilFunc(GL_ALWAYS, 1, 0xFF);

    program.SetUniform(std::string("surf.ambient"), material.ambient);
    program.SetUniform(std::string("surf.diffuse"), material.diffuse);
    program.SetUniform(std::string("surf.specular"), material.specular);
    program.SetUniform(std::string("surf.shininess"), material.shininess);
    program.SetUniform(std::string("surf.albedo"), material.albedo);
    program.SetUniform(std::string("surf.metallic"), material.metallic);
    program.SetUniform(std::string("surf.roughness"), material.roughness);
    program.SetUniform(std::string("surf.ao"), material.ao);
    program.SetUniform(std::string("tex_unit"), (int)material.GetTexUnit());

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, worldMatrices.size() * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(4 * sizeof(GLfloat)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(8 * sizeof(GLfloat)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(12 * sizeof(GLfloat)));
    glEnableVertexAttribArray(3); 
    glEnableVertexAttribArray(4); 
    glEnableVertexAttribArray(5); 
    glEnableVertexAttribArray(6); 
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);
    
    glPolygonMode(GL_FRONT_AND_BACK, drawMode);
    glDrawElementsInstanced(primitiveType, tris.size(), GL_UNSIGNED_SHORT, 0, worldMatrices.size());
}

void Mesh::Render(ShaderProgram& program, const std::vector<glm::mat4>& worldMatrices, float outline) const
{
    if (!_initialized)
        throw std::runtime_error("VAO not initialized");

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
