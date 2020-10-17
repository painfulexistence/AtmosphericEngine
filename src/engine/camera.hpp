#pragma once
#include "../common.hpp"
#include "../physics/BulletMain.h"


class Camera {
    public:
        Camera(glm::vec3 position, float hAngle, float vAngle, float fov = glm::radians(45.0f), float aspectRatio = 4.f / 3.f, btDiscreteDynamicsWorld* dynamicsWorld = NULL) {
            _position = position;
            _hAngle = hAngle;
            _vAngle = vAngle;
            _fov = fov;
            _aspectRatio = aspectRatio;

            if (dynamicsWorld != NULL) {
                btTransform trans = btTransform(btQuaternion(0, 0, 0), btVector3(position.x, position.y, position.z));
                btDefaultMotionState* motionState = new btDefaultMotionState(trans);
                btCollisionShape* shape = new btCapsuleShape(btScalar(0.5), btScalar(1.5));

                _rigidbody = new btRigidBody(btScalar(10.0), motionState, shape, btVector3(1, 1, 1));
                _rigidbody->setLinearFactor(btVector3(1, 1, 1));
                _rigidbody->setAngularFactor(btVector3(0, 0, 0));

                dynamicsWorld->addRigidBody(_rigidbody);
            }
        }

        void backAndForthMove(float speed) {
            glm::vec3 direction = glm::vec3(glm::cos(_hAngle) * glm::cos(_vAngle), glm::sin(_vAngle), glm::sin(_hAngle) * glm::cos(_vAngle));
            glm::vec3 direction_cos = glm::normalize(glm::vec3(direction.x, 0, direction.z));
            glm::vec3 velocity = speed * direction_cos;
            _rigidbody->activate();
            _rigidbody->setLinearVelocity(btVector3(velocity.x, _rigidbody->getLinearVelocity().y(), velocity.z));
        }
        
        void damp() {
            //glm::vec4 transformed_position_4d = getDynamicsTransform() * glm::vec4(_position, 1.0);
            //printf("(%f, %f, %f) ", transformed_position_4d.x, transformed_position_4d.y, transformed_position_4d.z);
            _rigidbody->setLinearVelocity(btVector3(0, _rigidbody->getLinearVelocity().y(), 0));
        }

        void horizontallyMove(float speed) {
            glm::vec3 direction = glm::vec3(glm::cos(_hAngle) * glm::cos(_vAngle), glm::sin(_vAngle), glm::sin(_hAngle) * glm::cos(_vAngle));
            glm::vec3 velocity = glm::normalize(glm::cross(direction, glm::vec3(0, 1, 0))) * speed;
            _rigidbody->activate();
            _rigidbody->setLinearVelocity(btVector3(velocity.x, _rigidbody->getLinearVelocity().y(), velocity.z));
        }

        void verticallyMove(float vSpeed) {
            glm::vec3 velocity = glm::vec3(0, 1, 0) * vSpeed;
            _rigidbody->activate();
            _rigidbody->setLinearVelocity(btVector3(_rigidbody->getLinearVelocity().x(), velocity.y, _rigidbody->getLinearVelocity().z()));
        }

        void yaw(float angleOffset) 
        {
            _hAngle += angleOffset;
        }

        void pitch(float angleOffset)
        {
            _vAngle = std::max(minVAngle, std::min(maxVAngle, _vAngle + angleOffset));
        }

        glm::mat4 getDynamicsTransform() {
            btTransform trans;
            _rigidbody->getMotionState()->getWorldTransform(trans);
            btScalar mat[16] = { 0.0f };
            trans.getOpenGLMatrix(mat);
            glm::mat4 transform = glm::mat4(
                mat[0], mat[1], mat[2], mat[3],
                mat[4], mat[5], mat[6], mat[7],
                mat[8], mat[9], mat[10], mat[11],
                mat[12], mat[13], mat[14], mat[15]
            );
            return transform;
        }

        glm::mat4 getProjectionViewMatrix() {
            glm::vec3 direction = glm::vec3(glm::cos(_hAngle) * glm::cos(_vAngle), glm::sin(_vAngle), glm::sin(_hAngle) * glm::cos(_vAngle));
            glm::vec3 position = getPosition();

            glm::mat4 _projection = glm::perspective(_fov, _aspectRatio, 0.1f, 5000.0f);
            glm::mat4 _view = glm::lookAt(position, position + direction, glm::vec3(0, 1, 0));
            
            return _projection * _view;
        }

        bool isFreezing() {
            bool isFreezing = abs(_rigidbody->getLinearVelocity().y()) <= 0.0001;
            return true;
        }

        glm::vec3 getPosition() {
            glm::vec4 transformed_position_4d = getDynamicsTransform() * glm::vec4(_position, 1.0);
            glm::vec3 transformed_position = glm::vec3(transformed_position_4d.x, transformed_position_4d.y, transformed_position_4d.z);
            return transformed_position;
        }

        void setPosition(glm::vec3 position) {
            //_position = position;
            btTransform trans = btTransform(btQuaternion(0, 0, 0), btVector3(position.x, position.y, position.z));
            _rigidbody->proceedToTransform(trans);
        }

    private:
        const float maxVAngle = 3.14f/2.0f;
        const float minVAngle = -3.14f/2.0f;

        glm::vec3 _position;
        float _hAngle;
        float _vAngle;
        float _fov;
        float _aspectRatio;

        btRigidBody* _rigidbody;
};
