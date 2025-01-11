#ifndef LIGHT_H
#define LIGHT_H

#include <glm/glm.hpp>
#include <iostream>

enum LightType { DIRECTIONAL, POINTY };

class Light {
public:

    // Constructors
    Light() :
        type(DIRECTIONAL),
        enabled(true),
        position(0.0f),
        direction(0.0f, 0.0f, -1.0f),
        intensity(0.3f) {}

    Light(LightType t, bool enabled, glm::vec3 position, glm::vec3 direction, glm::vec3 intensity) {
        this->type = t;
        this->enabled = enabled;
        this->position = position;
        this->direction = direction;
        this->intensity = intensity;
    }

    void printParameters() const {
        std::cout << "Light Parameters:" << std::endl;
        std::cout << "  Type: " << (type == DIRECTIONAL ? "Directional" : "Point") << std::endl;
        std::cout << "  Enabled: " << (enabled ? "Yes" : "No") << std::endl;
        std::cout << "  Position: (" << position.x << ", " << position.y << ", " << position.z << ")" << std::endl;
        std::cout << "  Direction: (" << direction.x << ", " << direction.y << ", " << direction.z << ")" << std::endl;
        std::cout << "  Intensity: (" << intensity.r << ", " << intensity.g << ", " << intensity.b << ")" << std::endl;
    }

    void transformLightSource(const glm::mat4& worldTransformationMatrix) {
        if (type == POINTY) {
            // Transform position for point light
            glm::vec4 transformedPos = worldTransformationMatrix * glm::vec4(position, 1.0f);
            position = glm::vec3(transformedPos); // Drop the w component
        }
        else if (type == DIRECTIONAL) {
            // Transform direction for directional light (ignore translation)
            glm::mat3 rotationMatrix = glm::mat3(worldTransformationMatrix); // Extract upper-left 3x3 for rotation
            glm::vec3 transformedDir = glm::normalize(rotationMatrix * direction);
            direction = transformedDir; // Update the direction
        }
    }
        

    // Light properties
    LightType type;
    bool enabled;            // Enable or disable the light
    glm::vec3 position;      // Position (for point lights)
    glm::vec3 direction;     // Direction (for directional lights)
    glm::vec3 intensity;     // RGB intensity

    // Member functions
    void setType(LightType t) { type = t; }
    void setEnabled(bool e) { enabled = e; }
    void setPosition(const glm::vec3& pos) { position = pos; }
    void setDirection(const glm::vec3& dir) { direction = dir; }
    void setIntensity(const glm::vec3& inten) { intensity = inten; }
};

#endif // LIGHT_H