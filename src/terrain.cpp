#include "terrain.hpp"

Terrain::Terrain(int size, int vnum, float heightmap[], Shader* shader)
{
    _size = size;
    _localTransform = glm::mat4(1.0f);
    _shader = shader;
    
    _drawMode = GL_FILL;
    numVertices = vnum * vnum * 8;
    vertices = new float[numVertices];      
    float c_size = float(size)/float(vnum-1);
    for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
            float x = - float(size/2) + j * c_size;
            float y = heightmap[(i * vnum + j)];
            float z = - float(size/2) + i * c_size;
            vertices[(i * vnum + j) * 8 + 0] = x;
            vertices[(i * vnum + j) * 8 + 1] = y - 10;
            vertices[(i * vnum + j) * 8 + 2] = z;
            vertices[(i * vnum + j) * 8 + 3] = 0.f; //2 * float(i)/float(vnum);
            vertices[(i * vnum + j) * 8 + 4] = 1.f; //2 * float(j)/float(vnum);
            vertices[(i * vnum + j) * 8 + 5] = 0.f; //0.8f;
            vertices[(i * vnum + j) * 8 + 6] = (float)(i%2);
            vertices[(i * vnum + j) * 8 + 7] = (float)(j%2);
        }
    }

    numElements = (vnum * 2 + 1) * (vnum - 1);
    triangles = new GLushort[numElements];
    for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
            triangles[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        triangles[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ARRAY_BUFFER, numVertices*sizeof(float), vertices, GL_STATIC_DRAW);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, numElements*sizeof(GLushort), triangles, GL_STATIC_DRAW);

    glVertexAttribPointer(shader->GetAttrib("position"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)0);
    glVertexAttribPointer(shader->GetAttrib("normal"), 3, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(3*sizeof(float)));
    glVertexAttribPointer(shader->GetAttrib("uv"), 2, GL_FLOAT, GL_FALSE, 8*sizeof(float), (void*)(6*sizeof(float)));
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);
}

Terrain::~Terrain()
{
    delete[] vertices;
    delete[] triangles;
}

void Terrain::AddRigidBody(btDiscreteDynamicsWorld* dynamicsWorld)
{
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(0, -15, 0));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, 5, _size / 2.f));

    _rigidbody = new btRigidBody(btScalar(0.0), motionState, shape, btVector3(1, 1, 1));
    dynamicsWorld->addRigidBody(_rigidbody);
}

void Terrain::Render(glm::mat4 mMatrix, Light* light, Camera* camera) 
{
    glBindVertexArray(vao);

    glm::mat4 PV = camera->getProjectionViewMatrix();
    glm::mat4 M = mMatrix * _localTransform;
    glm::vec3 lightColor = light->getColor();
    glm::vec3 lightPosition = light->getPosition();  
    glm::vec3 lightDirection = light->getDirection();
    glm::vec3 cameraPosition = camera->getPosition();

    glUniformMatrix4fv(_shader->GetUniform("PV"), 1, GL_FALSE, &PV[0][0]);
    glUniformMatrix4fv(_shader->GetUniform("M"), 1, GL_FALSE, &M[0][0]);
    glUniform3fv(_shader->GetUniform("light_pos"), 1, &lightPosition[0]);
    glUniform3fv(_shader->GetUniform("light_color"), 1, &lightColor[0]);      
    glUniform3fv(_shader->GetUniform("light_dir"), 1, &lightDirection[0]);
    glUniform3fv(_shader->GetUniform("view_pos"), 1, &cameraPosition[0]);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, numElements, GL_UNSIGNED_SHORT, 0);

    glBindVertexArray(0);
}