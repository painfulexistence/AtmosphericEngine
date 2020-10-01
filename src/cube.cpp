#include "cube.hpp"

Cube::Cube(int size, Shader* shader)
{
    _size = size;
    _localTransform = glm::scale(glm::mat4(1.0f), glm::vec3(size * 0.5f, size * 0.5f, size * 0.5f));
    _shader = shader;

    vertices = new float[192] {
        //left
        1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
        //right
        1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, -1.0f, 0.0f, 0.0f,
        //back
        -1.0f, 1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, -1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //front
        1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f,
        1.0f, -1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
        //top
        1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, 1.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, 1.0f, -1.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
        //bottom
        1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, -1.0f, 0.0f, -1.0f, 0.0f, 0.0f, 0.0f
    };

    triangles = new GLushort[36] {
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

void Cube::AddRigidBody(glm::vec3 position, btDiscreteDynamicsWorld* dynamicsWorld, float mass = 0.f)
{
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(position.x, position.y, position.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, _size / 2.f, _size / 2.f));

    _rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    dynamicsWorld->addRigidBody(_rigidbody);
}

void Cube::Render(glm::mat4 mMatrix, Light* light, Camera* camera) 
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
    glDrawElements(GL_TRIANGLES, numElements, GL_UNSIGNED_SHORT, 0); //glDrawArrays(GL_TRIANGLES, 0, 24);

    glBindVertexArray(0);
}