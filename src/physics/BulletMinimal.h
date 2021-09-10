#pragma once
#include <bullet/LinearMath/btQuaternion.h>
#include <bullet/LinearMath/btTransform.h>
#include <bullet/LinearMath/btVector3.h>
#include <bullet/LinearMath/btDefaultMotionState.h>
#include <bullet/LinearMath/btIDebugDraw.h>
#define FIXED_TIME_STEP 1.0 / 60.0
#define GRAVITY 9.8

// Forward declare classes which will be used for pointer rather than include all-in-one headers (eg. btBulletCollisionsCommon.h)
class btCollisionConfiguration;
class btCollisionDispatcher;
class btBroadphaseInterface;
class btConstraintSolver;
class btDynamicsWorld;
class btCollisionShape;
class btBoxShape;
class btCapsuleShape;
class btConvexHullShape;
class btCompoundShape;
class btSphereShape;
class btRigidBody;
class btCollisionObject;