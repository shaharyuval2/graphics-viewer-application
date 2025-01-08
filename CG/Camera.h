#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>

class Camera {
public:
    Camera();
    Camera(float mnear, float mfar, float top, float right);

    // Set camera position and orientation
    void movePosition(float dcx, float dcy, float dcz);
    void lookAt(const glm::vec4& target);
    void updateProjectionMatrix(float mnear, float mfar, float top, float right);

    // Get transformation matrices
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix() const;

    //helper function
    glm::vec3 nonhomogenous(glm::vec4 homoV);

private:
    glm::vec3 position;
    glm::vec3 front;

    float fov;
    float aspect;
    float nearPlane;
    float farPlane;

    // Matrices
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;

    // Update vectors based on pitch and yaw
    void updateViewMatrix();
};

#endif // CAMERA_H