#ifndef MATERIAL_H
#define MATERIAL_H

#include <glm/glm.hpp>

class Material {
public:
    Material();
    Material(float ka, float kd, float ks, float colorR, float colorG, float colorB, float n);

    glm::vec3 K; // K = (ka,kd,ks)
    glm::vec3 color; // in RGB
    float n; //"shininess"


private:
    
};

#endif // MATERIAL_H
