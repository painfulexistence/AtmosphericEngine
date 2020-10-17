#include "terrain.hpp"

Terrain::Terrain(int size, int vnum, float heightmap[])
{
    _size = size;    
    numVert = vnum * vnum * 8;
    vertices = new float[numVert];      
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

    numTri = (vnum * 2 + 1) * (vnum - 1);
    triangles = new GLushort[numTri];
    for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
            triangles[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        triangles[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
    }
}

void Terrain::Embody(glm::vec3 center, float mass = 0.0f, btDiscreteDynamicsWorld* dynamicsWorld = nullptr)
{
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(center.x ,center.y, center.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, 5, _size / 2.f));

    rigidbody = new btRigidBody(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    dynamicsWorld->addRigidBody(rigidbody);
}

void Terrain::Render(std::vector<glm::mat4> worldMatrices) 
{
    if (!initialized)
        std::runtime_error("VAO uninitialized!");
        
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, worldMatrices.size() * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLE_STRIP, numTri, GL_UNSIGNED_SHORT, 0, worldMatrices.size());
    glBindVertexArray(0);
}

void Terrain::Render(glm::mat4* worldMatrices, int num)
{
    if (!initialized)
        std::runtime_error("VAO uninitialized!");

    glBindVertexArray(vao);
    
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, num * sizeof(glm::mat4), worldMatrices, GL_STATIC_DRAW);
    
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElements(GL_TRIANGLE_STRIP, numTri, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);
}
