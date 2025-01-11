#include "Lighting.h"
#include <iostream>


// Constructor
Lighting::Lighting()
    : ambientIntensity(1.0f, 1.0f, 1.0f) { // Default ambient intensity to white
}

Lighting::Lighting(const Light& light1, const Light& light2, const glm::vec3& intensity) {
    this->light1 = light1;
    this->light2 = light2;
    this->ambientIntensity = intensity;
}

// Set ambient light intensity
void Lighting::setAmbientIntensity(const glm::vec3& intensity) {
    ambientIntensity = intensity;
}

// Print all lighting parameters
void Lighting::printLightingParameters() const {
    std::cout << "Lighting Parameters:\n";

    std::cout << "  Light 1:\n";
    light1.printParameters(); // Print Light 1 details

    std::cout << "  Light 2:\n";
    light2.printParameters(); // Print Light 2 details

    std::cout << "  Ambient Intensity: ("
        << ambientIntensity.r << ", "
        << ambientIntensity.g << ", "
        << ambientIntensity.b << ")\n";
}

void Lighting::applyWorldTransformationToLights(const glm::mat4& worldTransformationMatrix) {
    light1.transformLightSource(worldTransformationMatrix);
    light2.transformLightSource(worldTransformationMatrix);
}