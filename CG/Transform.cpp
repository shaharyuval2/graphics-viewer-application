#include "Transform.h"

const float PI = 3.141592645358;


Transform::Transform() {
    objectR = glm::mat4(1.0f);
    objectS = glm::mat4(1.0f);
    objectT = glm::mat4(1.0f);

    worldR = glm::mat4(1.0f);
    worldS = glm::mat4(1.0f);
    worldT = glm::mat4(1.0f);
}

// Transformation methods
// object methods
void Transform::objectRotate(float sx, float sy, float sz){
    // X rotation matrix
    float cosx = std::cos(PI * sx / 180);
    float sinx = std::sin(PI * sx / 180);

    glm::mat4 Rx = glm::mat4(
        1, 0, 0, 0,
        0, cosx, -sinx, 0,
        0, sinx, cosx, 0,
        0, 0, 0, 1
    );

    // Y rotation matrix
    float cosy = std::cos(PI * sy / 180);
    float siny = std::sin(PI * sy / 180);

    glm::mat4 Ry = glm::mat4(
        cosy, 0, siny, 0,
        0, 1, 0, 0,
        -siny, 0, cosy, 0,
        0, 0, 0, 1
    );

    // Z rotation matrix
    float cosz = std::cos(PI * sz / 180);
    float sinz = std::sin(PI * sz / 180);

    glm::mat4 Rz = glm::mat4(
        cosz, sinz, 0, 0,
        -sinz, cosz, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    objectR = Rz * Ry * Rx * objectR;
}

void Transform::objectScale(float factor) {
    glm::mat4 S = glm::mat4(
        factor, 0, 0, 0,
        0, factor, 0, 0,
        0, 0, factor, 0,
        0, 0, 0, 1
    );

    objectS =  S * objectS;

}

void Transform::objectTranslate(float tx, float ty, float tz) {
    glm::mat4 M = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        tx, ty, tz, 1
    );

    objectT =  M * objectT;
}

//world methods
void Transform::worldRotate(float sx, float sy, float sz) {
    // X rotation matrix
    float cosx = std::cos(PI * sx / 180);
    float sinx = std::sin(PI * sx / 180);

    glm::mat4 Rx = glm::mat4(
        1, 0, 0, 0,
        0, cosx, -sinx, 0,
        0, sinx, cosx, 0,
        0, 0, 0, 1
    );

    // Y rotation matrix
    float cosy = std::cos(PI * sy / 180);
    float siny = std::sin(PI * sy / 180);

    glm::mat4 Ry = glm::mat4(
        cosy, 0, siny, 0,
        0, 1, 0, 0,
        -siny, 0, cosy, 0,
        0, 0, 0, 1
    );

    // Z rotation matrix
    float cosz = std::cos(PI * sz / 180);
    float sinz = std::sin(PI * sz / 180);

    glm::mat4 Rz = glm::mat4(
        cosz, sinz, 0, 0,
        -sinz, cosz, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    );

    worldR = Rz * Ry * Rx * worldR;
}

void Transform::worldScale(float factor) {
    glm::mat4 S = glm::mat4(
        factor, 0, 0, 0,
        0, factor, 0, 0,
        0, 0, factor, 0,
        0, 0, 0, 1
    );

    worldS = S * worldS;

}

void Transform::worldTranslate(float tx, float ty, float tz) {
    glm::mat4 M = glm::mat4(
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        tx, ty, tz, 1
    );

    worldT = M * worldT;
}

glm::mat4 Transform::getObjectTransformationMatrix() {
    return objectT * (objectS * objectR);
}

glm::mat4 Transform::getWorldTransformationMatrix() {
    return worldR * (worldS * worldT);
}