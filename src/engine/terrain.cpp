#include "terrain.hpp"

Terrain::Terrain(int size, int vnum, float heightmap[])
{
    _size = size;    
    verts.resize(vnum * vnum * 8);
    float c_size = float(size)/float(vnum-1);
    for (int i = 0; i < vnum; i++) {
        for (int j = 0; j < vnum; j++) {
            float x = - float(size/2) + j * c_size;
            float y = heightmap[(i * vnum + j)];
            float z = - float(size/2) + i * c_size;
            verts[(i * vnum + j) * 8 + 0] = x;
            verts[(i * vnum + j) * 8 + 1] = y - 10;
            verts[(i * vnum + j) * 8 + 2] = z;
            verts[(i * vnum + j) * 8 + 3] = 0.f; //2 * float(i)/float(vnum);
            verts[(i * vnum + j) * 8 + 4] = 1.f; //2 * float(j)/float(vnum);
            verts[(i * vnum + j) * 8 + 5] = 0.f; //0.8f;
            verts[(i * vnum + j) * 8 + 6] = (float)(i%2);
            verts[(i * vnum + j) * 8 + 7] = (float)(j%2);
        }
    }

    tris.resize((vnum * 2 + 1) * (vnum - 1));
    for (int i = 0; i < vnum - 1; i++) {
        for (int j = 0; j < 2 * vnum; j++) {
            tris[i * (2 * vnum + 1) + j] = (i + j % 2) * vnum + (j / 2);
        }
        tris[i * (2 * vnum + 1) + 2 * vnum] = 0xFFFF;
    }
}

void Terrain::Embody(glm::vec3 center, float mass = 0.0f, const std::shared_ptr<btDiscreteDynamicsWorld>& world = nullptr)
{
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(center.x ,center.y, center.z));
    btDefaultMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btBoxShape(btVector3(_size / 2.f, 5, _size / 2.f));

    rigidbody = std::make_shared<btRigidBody>(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    world->addRigidBody(rigidbody.get());
}

void Terrain::Render(std::vector<glm::mat4> worldMatrices, GLenum mode) 
{
    if (!initialized)
        throw std::runtime_error("VAO uninitialized!");
        
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, worldMatrices.size() * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLE_STRIP, tris.size(), GL_UNSIGNED_SHORT, 0, worldMatrices.size());
    glBindVertexArray(0);
}
