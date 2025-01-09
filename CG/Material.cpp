#include "Material.h"

Material::Material() {
	K = glm::vec3(0.1f, 0.5f, 0.3f);
	color = glm::vec3(0.5f, 0.5f, 0.5f); //grey
	n = 16;
}

Material::Material(float ka, float kd, float ks, float n, uint32_t Color) {
	K = glm::vec3(ka,kd,ks);
	color = ConvertUint32toVector(Color);
	Material::n = n;
}

glm::vec3 Material::ConvertUint32toVector(uint32_t uint_color) {
	// Extract the Red, Green, and Blue components from the 32-bit color value
    float r = ((uint_color >> 16) & 0xFF) / 255.0f; // Red component
    float g = ((uint_color >> 8) & 0xFF) / 255.0f;  // Green component
    float b = (uint_color & 0xFF) / 255.0f;         // Blue component

    return glm::vec3(r, g, b); // Return the normalized RGB vector
}

uint32_t Material::ConvertVectortoUint32(const glm::vec3& vec_Color) {
    // Clamp the values to ensure they are within the [0.0, 1.0] range
    float r = glm::clamp(vec_Color.r, 0.0f, 1.0f);
    float g = glm::clamp(vec_Color.g, 0.0f, 1.0f);
    float b = glm::clamp(vec_Color.b, 0.0f, 1.0f);

    // Scale the normalized values to [0, 255]
    uint32_t red = static_cast<uint32_t>(r * 255.0f);
    uint32_t green = static_cast<uint32_t>(g * 255.0f);
    uint32_t blue = static_cast<uint32_t>(b * 255.0f);

    // Combine into a single 32-bit value with alpha set to 255
    uint32_t color = (255 << 24) | (blue << 16) | (green << 8) | red;

    return color;
}

uint32_t Material::getUintColor() {
    return ConvertVectortoUint32(color);
}