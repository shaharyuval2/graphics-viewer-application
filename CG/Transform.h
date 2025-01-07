#ifndef TRANSFORM_H
#define TRANSFORM_H

#include <glm/glm.hpp>

class Transform {
public:
    Transform();

    // Transformation methods
    void objectRotate(float sx, float sy, float sz);
    void objectScale(float factor);
    void objectTranslate(float tx, float ty, float tz);

    void worldRotate(float sx, float sy, float sz);
    void worldScale(float factor);
    void worldTranslate(float tx, float ty, float tz);

    //get final transformation matrix
    glm::mat4 getObjectTransformationMatrix();
    glm::mat4 getWorldTransformationMatrix();

private:
    glm::mat4 objectR;
    glm::mat4 objectS;
    glm::mat4 objectT;

    glm::mat4 worldR;
    glm::mat4 worldS;
    glm::mat4 worldT;
};

#endif // TRANSFORM_H