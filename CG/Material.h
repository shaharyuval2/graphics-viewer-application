#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>
#include <cstdint>

class Material {
public:
    Material();
    Material(float ka, float kd, float ks, float n, uint32_t Color);

    glm::vec3 K; // K = (ka,kd,ks)
    glm::vec3 color; // in RGB
    float n; //"shininess"

    glm::vec3 ConvertUint32toVector(uint32_t uint_Color);
    uint32_t ConvertVectortoUint32(const glm::vec3& vec_Colo);
    uint32_t getUintColor();
};

#endif // MATERIAL_H
