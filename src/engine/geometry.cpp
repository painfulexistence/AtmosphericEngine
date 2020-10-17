#include "geometry.hpp"

Geometry::Geometry()
{
    _id = 1;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glGenBuffers(1, &ibo);
}

Geometry::~Geometry()
{
    _id = 0;
    delete[] vertices;
    delete[] triangles;
}

void Geometry::Init()
{
    // Bind VAO
    glBindVertexArray(vao);

    // Fill vertex buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, numVert*sizeof(float), vertices, GL_STATIC_DRAW);

    // Fill element buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numTri*sizeof(GLushort), triangles, GL_STATIC_DRAW);

    // Specify and enable vertex attributes
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)0);
    glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(4 * sizeof(float)));
    glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(8 * sizeof(float)));
    glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(12 * sizeof(float)));
    glEnableVertexAttribArray(3); 
    glEnableVertexAttribArray(4); 
    glEnableVertexAttribArray(5); 
    glEnableVertexAttribArray(6); 
    glVertexAttribDivisor(3, 1);
    glVertexAttribDivisor(4, 1);
    glVertexAttribDivisor(5, 1);
    glVertexAttribDivisor(6, 1);

    // Unbind VAO
    glBindVertexArray(0);
    
    initialized = true;
}

void Geometry::Embody(glm::vec3 center, float mass = 0.0f, btDiscreteDynamicsWorld* world = nullptr)
{
    std::runtime_error("Virtual method not implemented!");
}

void Geometry::Update(float time)
{
    return;
}

void Geometry::Render(std::vector<glm::mat4> worldMatrices)
{
    std::runtime_error("Virtual method not implemented");
}

void Geometry::Render(glm::mat4* worldMatrices, int num)
{
    std::runtime_error("Virtual method not implemented");
}

void Geometry::RenderWithOutline(glm::mat4* worldsMatrices, int num)
{
    std::runtime_error("Virtual method not implemented");

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
