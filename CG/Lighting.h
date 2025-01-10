#ifndef LIGHTING_H
#define LIGHTING_H

#include "Light.h"
#include <glm/glm.hpp>

class Lighting {
public:
    // Constructor
    Lighting();
    Lighting(const Light& light1,const Light& light2 ,const glm::vec3& intensity);

    void setAmbientIntensity(const glm::vec3& intensity);

    // Print function
    void printLightingParameters() const;

    Light light1;                   // First light source
    Light light2;                   // Second light source
    glm::vec3 ambientIntensity;    // Global ambient light intensity
};

#endif // LIGHTING_H

