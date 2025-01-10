#include "Shading.h"



Shading::Shading() {
	currentZBufferMode = ZBUFFER_MODE_1;
	currentShadingMode = WIREFRAME;
}

Shading::Shading(ZBufferMode ZMode, ShadingMode ShadingMode) {
	currentZBufferMode = ZMode;
	currentShadingMode = ShadingMode;
}

void Shading::rasterize(const std::vector<glm::vec4>& vertices, const std::vector<glm::vec4>& normals,
    const std::vector<Wavefront_obj::Face>& faces, const glm::vec3& ambientColor, const glm::vec3& objectColor
    , float specularCoefficient, float shininess, int screenWidth, int screenHeight) {

    Renderer renderer;
    std::vector<Pixel> pixels; // Container for visible pixels

    //for now light source is static
    glm::vec3 lightDir = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f)); // Directional light

    // allocate and Initialize the Z-buffer with maximum depth
    double* zBuffer = new double[screenWidth * screenHeight];
    std::fill(zBuffer, zBuffer + screenWidth * screenHeight, std::numeric_limits<double>::max());

    // Loop through each face
    for (const auto& face : faces) {
        // Extract vertex positions
        // Extract vertex positions
        glm::vec4 p0 = vertices[face.v[0]];
        glm::vec4 p1 = vertices[face.v[1]];
        glm::vec4 p2 = vertices[face.v[2]];

        glm::vec3 n0 = glm::vec3(normals[face.v[0]]);
        glm::vec3 n1 = glm::vec3(normals[face.v[1]]);
        glm::vec3 n2 = glm::vec3(normals[face.v[2]]);

        // Perspective divide to transform to screen space
        glm::vec3 v0 = glm::vec3(p0) / p0.w;
        glm::vec3 v1 = glm::vec3(p1) / p1.w;
        glm::vec3 v2 = glm::vec3(p2) / p2.w;

        // Map to screen coordinates
        auto toScreen = [screenWidth, screenHeight](const glm::vec3& v) -> glm::ivec2 {
            return glm::ivec2(
                (v.x * 0.5f + 0.5f) * screenWidth,
                (v.y * 0.5f + 0.5f) * screenHeight
            );
        };
        glm::ivec2 s0 = toScreen(v0);
        glm::ivec2 s1 = toScreen(v1);
        glm::ivec2 s2 = toScreen(v2);

        // Bounding box for the triangle
        int minX = std::max(0, std::min({ s0.x, s1.x, s2.x }));
        int maxX = std::min(screenWidth - 1, std::max({ s0.x, s1.x, s2.x }));
        int minY = std::max(0, std::min({ s0.y, s1.y, s2.y }));
        int maxY = std::min(screenHeight - 1, std::max({ s0.y, s1.y, s2.y }));

        // Rasterize the triangle
        for (int y = minY; y <= maxY; ++y) {
            for (int x = minX; x <= maxX; ++x) {
                // Compute edge functions
                float area = (s1.x - s2.x) * (s0.y - s2.y) - (s1.y - s2.y) * (s0.x - s2.x);
                if (area == 0.0f) {
                    continue; // Degenerate triangle
                }

                float w0 = (x - s1.x) * (s2.y - s1.y) - (y - s1.y) * (s2.x - s1.x);
                float w1 = (x - s2.x) * (s0.y - s2.y) - (y - s2.y) * (s0.x - s2.x);

                // Normalize barycentric coordinates
                float bary0 = w0 / area;
                float bary1 = w1 / area;
                float bary2 = 1 - bary0 - bary1;

                // If inside the triangle
                if (bary0 >= 0 && bary1 >= 0 && bary2 >= 0) {
                    //here you can change the zmode !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
                    // Interpolate depth using f(z)
                    double depth = bary0 * v0.z + bary1 * v1.z + bary2 * v2.z;

                    // Z-buffer test
                    int bufferIndex = y * screenWidth + x;
                    if (depth < zBuffer[bufferIndex]) {
                        zBuffer[bufferIndex] = depth;

                        //diffuse
                        glm::vec3 interpolatedNormal = glm::normalize(bary0 * n0 + bary1 * n1 + bary2 * n2);
                        float diffuseIntensity = std::max(0.0f, glm::dot(interpolatedNormal, lightDir));

                        //specular
                        glm::vec3 viewDir = glm::normalize(- glm::vec3(bary0 * v0 + bary1 * v1 + bary2 * v2));
                        glm::vec3 reflectDir = glm::reflect(-lightDir, interpolatedNormal);
                        float specularIntensity = std::pow(std::max(glm::dot(viewDir, reflectDir), 0.0f), shininess);

                        glm::vec3 color = ambientColor * objectColor
                            + 0.5f * diffuseIntensity * objectColor
                            + specularCoefficient * specularIntensity * glm::vec3(1.0f);

                        color = glm::clamp(color, 0.0f, 1.0f);



                        // Convert color to RGBA format
                        unsigned int rgbaColor =
                            (static_cast<unsigned int>(color.b * 255) & 0xFF) << 16 |  // Blue in bits 16-23
                            (static_cast<unsigned int>(color.g * 255) & 0xFF) << 8 |  // Green in bits 8-15
                            (static_cast<unsigned int>(color.r * 255) & 0xFF) |        // Red in bits 0-7
                            0xFF << 24;                                              // Alpha in bits 24-31


                        // store the pixel
                        pixels.push_back({ x, y, rgbaColor });

                        
                    }
                }
            }
        }
    }
    // Render the pixels
    renderer.drawPixels(pixels);
    // Deallocate Z-buffer
    delete[] zBuffer;
}


void Shading::printSettings() const {
    std::cout << "Shading Settings:" << std::endl;

    // Print Z-Buffer Mode
    std::cout << "  Z-Buffer Mode: ";
    switch (currentZBufferMode) {
    case ZBUFFER_MODE_1:
        std::cout << "Nonlinear Interpolation (f(z))" << std::endl;
        break;
    case ZBUFFER_MODE_2:
        std::cout << "Invert f(z) Per Pixel" << std::endl;
        break;
    case ZBUFFER_MODE_3:
        std::cout << "Invert f(z) Per Vertex, Linear Interpolation" << std::endl;
        break;
    }

    // Print Shading Mode
    std::cout << "  Shading Mode: ";
    switch (currentShadingMode) {
    case WIREFRAME:
        std::cout << "Wireframe" << std::endl;
        break;
    case FLAT:
        std::cout << "Flat Shading" << std::endl;
        break;
    case GOURAUD:
        std::cout << "Gouraud Shading" << std::endl;
        break;
    case PHONG:
        std::cout << "Phong Shading" << std::endl;
        break;
    }
}