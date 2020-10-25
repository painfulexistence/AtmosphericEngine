#include "camera.hpp"


static const float maxVAngle = PI / 2.0f - 0.01f;
static const float minVAngle = -PI / 2.0f + 0.01f;

Camera::Camera(glm::vec3 position, glm::vec2 vhAngle, CameraProperties props) {
    _position = position;
    _vhAngle = vhAngle;
    _projection = glm::perspective(props.fov, props.aspectRatio, props.nearClipPlane, props.farClipPlane);
}

void Camera::Embody(const std::shared_ptr<btDiscreteDynamicsWorld>& world)
{
    if (world == 0)
        return;

    btTransform trans = btTransform(btQuaternion(0, 0, 0), btVector3(_position.x, _position.y, _position.z));
    btMotionState* motionState = new btDefaultMotionState(trans);
    btCollisionShape* shape = new btCapsuleShape(btScalar(0.5), btScalar(1.5));

    _rigidbody = new btRigidBody(btScalar(10.0), motionState, shape, btVector3(1, 1, 1));
    _rigidbody->setLinearFactor(btVector3(1, 1, 1));
    _rigidbody->setAngularFactor(btVector3(0, 0, 0));
    world->addRigidBody(_rigidbody);
}

static glm::vec3 CalculateDirection(float vAngle, float hAngle)
{
    return glm::vec3(glm::cos(vAngle) * glm::cos(hAngle), glm::sin(vAngle), glm::sin(hAngle) * glm::cos(vAngle));
}

void Camera::backAndForthMove(float speed) {
    glm::vec3 dir = CalculateDirection(_vhAngle.x, _vhAngle.y);
    glm::vec3 velocity = speed * glm::normalize(glm::vec3(dir.x, 0, dir.z));
    _rigidbody->activate();
    _rigidbody->setLinearVelocity(btVector3(velocity.x, _rigidbody->getLinearVelocity().y(), velocity.z));
}

void Camera::damp() {
    //glm::vec4 transformed_position_4d = getDynamicsTransform() * glm::vec4(_position, 1.0);
    //printf("(%f, %f, %f) ", transformed_position_4d.x, transformed_position_4d.y, transformed_position_4d.z);
    _rigidbody->setLinearVelocity(btVector3(0, _rigidbody->getLinearVelocity().y(), 0));
}

void Camera::horizontallyMove(float speed) {
    glm::vec3 dir = CalculateDirection(_vhAngle.x, _vhAngle.y);
    glm::vec3 velocity = glm::normalize(glm::cross(dir, glm::vec3(0, 1, 0))) * speed;
    _rigidbody->activate();
    _rigidbody->setLinearVelocity(btVector3(velocity.x, _rigidbody->getLinearVelocity().y(), velocity.z));
}

void Camera::verticallyMove(float vSpeed) {
    glm::vec3 velocity = glm::vec3(0, 1, 0) * vSpeed;
    _rigidbody->activate();
    _rigidbody->setLinearVelocity(btVector3(_rigidbody->getLinearVelocity().x(), velocity.y, _rigidbody->getLinearVelocity().z()));
}

void Camera::yaw(float angleOffset) 
{
    _vhAngle.y += angleOffset;
}

void Camera::pitch(float angleOffset)
{
    _vhAngle.x = std::max(minVAngle, std::min(maxVAngle, _vhAngle.x + angleOffset));
}

glm::mat4 Camera::getDynamicsTransform() {
    btTransform t;
    _rigidbody->getMotionState()->getWorldTransform(t);
    btScalar mat[16] = { 0.0f };
    t.getOpenGLMatrix(mat);
    glm::mat4 transform = glm::mat4(
        mat[0], mat[1], mat[2], mat[3],
        mat[4], mat[5], mat[6], mat[7],
        mat[8], mat[9], mat[10], mat[11],
        mat[12], mat[13], mat[14], mat[15]
    );
    return transform;
}

glm::mat4 Camera::getProjectionViewMatrix() {
    glm::vec3 pos = getPosition();
    glm::mat4 view = glm::lookAt(pos, pos + CalculateDirection(_vhAngle.x, _vhAngle.y), glm::vec3(0, 1, 0));
    return _projection * view;
}

bool Camera::isFreezing() {
    bool isFreezing = abs(_rigidbody->getLinearVelocity().y()) <= 0.0001;
    return true;
}

