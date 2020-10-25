#include "sphere.hpp"

using namespace std;


Sphere::Sphere(float radius) : _radius(radius)
{
    const int division = 18; // Eg. 6 means 30deg, 12 means 15deg
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
                pos = glm::vec3(0.f, _radius, 0.f);
                norm = glm::vec3(0.f, 1.f, 0.f);
            }
            else if (v == division)
            {
                pos = glm::vec3(0.f, -_radius, 0.f);
                norm = glm::vec3(0.f, -1.f, 0.f);
            }
            else
            {
                pos = glm::vec3(
                    _radius * glm::sin(vAngle) * glm::cos(hAngle), 
                    _radius * glm::cos(vAngle), 
                    _radius * glm::sin(vAngle) * glm::sin(hAngle)
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
}

void Sphere::Embody(glm::vec3 center, float mass = 0.f, const shared_ptr<btDiscreteDynamicsWorld>& world = nullptr)
{
    if (world == 0)
    {
        throw runtime_error("Failed to create rigidbody");
    }
    btQuaternion qtn;
    btTransform trans;
    qtn.setEuler(0, 0, 0);
    trans.setIdentity();
    trans.setRotation(qtn);
    trans.setOrigin(btVector3(center.x, center.y, center.z));
    
    btMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btSphereShape(btScalar(_radius));
    rigidbody = make_shared<btRigidBody>(btScalar(mass), motionState, shape, btVector3(1, 1, 1));
    
    world->addRigidBody(rigidbody.get());
}

void Sphere::Render(vector<glm::mat4> worldMatrices, GLenum mode) 
{
    if (!initialized)
        throw runtime_error("VAO uninitialized!");
        
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, ibo);
    glBufferData(GL_ARRAY_BUFFER, worldMatrices.size() * sizeof(glm::mat4), worldMatrices.data(), GL_STATIC_DRAW);

    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDrawElementsInstanced(GL_TRIANGLES, tris.size(), GL_UNSIGNED_SHORT, 0, worldMatrices.size());
    glBindVertexArray(0);
}
