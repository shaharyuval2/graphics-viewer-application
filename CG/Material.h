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

    // Setters
    void setKa(float ka) { K.x = ka; }
    void setKd(float kd) { K.y = kd; }
    void setKs(float ks) { K.z = ks; }
    void setShininess(float shininess) { n = shininess; }
    void setColor(const uint32_t Color) { color = ConvertUint32toVector(Color); }

    
};

#endif // MATERIAL_H
