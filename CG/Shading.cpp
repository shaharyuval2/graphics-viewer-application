#include "Shading.h"



Shading::Shading() {
	currentZBufferMode = ZBUFFER_MODE_1;
	currentShadingMode = WIREFRAME;
}

Shading::Shading(ZBufferMode ZMode, ShadingMode ShadingMode) {
	currentZBufferMode = ZMode;
	currentShadingMode = ShadingMode;
}

void Shading::rasterize(
    const Camera& camera,
    const glm::mat4& viewportMatrix,
    const std::vector<glm::vec4>& worldVertices,
    const std::vector<glm::vec4>& worldNormals,
    const std::vector<glm::vec4>& clipVertices,
    const std::vector<glm::vec4>& clipNormals,
    const std::vector<Wavefront_obj::Face>& faces,
    const Lighting& lighting,
    Material material,
    int screenWidth,
    int screenHeight) {

    

    Renderer renderer;
    std::vector<Pixel> pixels; // Container for visible pixels

    // Light source properties
    glm::vec3 light1Dir = glm::normalize(-lighting.light1.direction);
    glm::vec3 light1Pos = lighting.light1.position;

    glm::vec3 light2Dir = glm::normalize(-lighting.light2.direction);
    glm::vec3 light2Pos = lighting.light2.position;

    float light2enabled = lighting.light2.enabled;
    float dampingFactor = 1.0f / (1 + light2enabled);

    // Allocate and initialize the Z-buffer with maximum depth
    double* zBuffer = new double[screenWidth * screenHeight];
    std::fill(zBuffer, zBuffer + screenWidth * screenHeight, std::numeric_limits<double>::max());

    // Loop through each face
    for (const auto& face : faces) {
        // Extract vertex positions and normals in world space
        glm::vec3 worldP0 = glm::vec3(worldVertices[face.v[0]]);
        glm::vec3 worldP1 = glm::vec3(worldVertices[face.v[1]]);
        glm::vec3 worldP2 = glm::vec3(worldVertices[face.v[2]]);

        glm::vec3 n0 = glm::normalize(glm::vec3(worldNormals[face.v[0]]));
        glm::vec3 n1 = glm::normalize(glm::vec3(worldNormals[face.v[1]]));
        glm::vec3 n2 = glm::normalize(glm::vec3(worldNormals[face.v[2]]));

        // Perspective divide to transform clip space vertices to screen space
        glm::vec4 v0 = clipVertices[face.v[0]] / clipVertices[face.v[0]].w;
        glm::vec4 v1 = clipVertices[face.v[1]] / clipVertices[face.v[1]].w;
        glm::vec4 v2 = clipVertices[face.v[2]] / clipVertices[face.v[2]].w;

        // Map to screen coordinates
        auto toScreen = [screenWidth, screenHeight](const glm::mat4& viewportMatrix, const glm::vec4& v) -> glm::ivec2 {
            glm::vec4 viewportV4 = viewportMatrix * v;
            return glm::ivec2(
                viewportV4.x,
                viewportV4.y
            );
        };
        glm::ivec2 s0 = toScreen(viewportMatrix, v0);
        glm::ivec2 s1 = toScreen(viewportMatrix, v1);
        glm::ivec2 s2 = toScreen(viewportMatrix, v2);

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
                    // Interpolate depth and compare based on current mode
                    int bufferIndex = y * screenWidth + x;
                    double depth;

                    double farPlane = camera.getFarPlane();
                    double nearPlane = camera.getNearPlane();

                    if (currentZBufferMode == ZBUFFER_MODE_1) {
                        // Mode 1: Interpolate and compare f(z)
                        depth = bary0 * v0.z + bary1 * v1.z + bary2 * v2.z; // f(z) interpolation
                    }
                    else if (currentZBufferMode == ZBUFFER_MODE_2) {
                        // Mode 2: Interpolate f(z), restore z, and compare
                        double interpolatedFZ = bary0 * v0.z + bary1 * v1.z + bary2 * v2.z; // f(z)
                        depth = restoreZ(interpolatedFZ, nearPlane, farPlane); // f^-1(f(z))
                    }
                    else if (currentZBufferMode == ZBUFFER_MODE_3) {
                        // Mode 3: Restore z on vertices, interpolate z, and compare
                        double z0 = restoreZ(v0.z, nearPlane, farPlane); // Restore z for vertex 0
                        double z1 = restoreZ(v1.z, nearPlane, farPlane); // Restore z for vertex 1
                        double z2 = restoreZ(v2.z, nearPlane, farPlane); // Restore z for vertex 2

                        depth = bary0 * z0 + bary1 * z1 + bary2 * z2; // Interpolate restored z
                    }

                    // Z-buffer test
                    if (depth < zBuffer[bufferIndex]) {
                        zBuffer[bufferIndex] = depth;

                        // Interpolate world-space fragment position and normal
                        glm::vec3 fragmentPos = bary0 * worldP0 + bary1 * worldP1 + bary2 * worldP2;
                        glm::vec3 interpolatedNormal = glm::normalize(bary0 * n0 + bary1 * n1 + bary2 * n2);

                        // Light 1 calculations
                        float diffuseIntensity1 = 0.0f;
                        float specularIntensity1 = 0.0f;
                        if (lighting.light1.type == DIRECTIONAL) {
                            diffuseIntensity1 = std::max(0.0f, glm::dot(interpolatedNormal, light1Dir));
                            glm::vec3 reflectDir1 = glm::reflect(-light1Dir, interpolatedNormal);
                            specularIntensity1 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir1), 0.0f), material.n);
                        }
                        else if (lighting.light1.type == POINTY) {
                            glm::vec3 light1ToFrag = glm::normalize(light1Pos - fragmentPos);
                            diffuseIntensity1 = std::max(0.0f, glm::dot(interpolatedNormal, light1ToFrag));
                            glm::vec3 reflectDir1 = glm::reflect(-light1ToFrag, interpolatedNormal);
                            specularIntensity1 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir1), 0.0f), material.n);
                        }

                        // Light 2 calculations
                        float diffuseIntensity2 = 0.0f;
                        float specularIntensity2 = 0.0f;
                        if (lighting.light2.type == DIRECTIONAL) {
                            diffuseIntensity2 = std::max(0.0f, glm::dot(interpolatedNormal, light2Dir));
                            glm::vec3 reflectDir2 = glm::reflect(-light2Dir, interpolatedNormal);
                            specularIntensity2 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir2), 0.0f), material.n);
                        }
                        else if (lighting.light2.type == POINTY) {
                            glm::vec3 light2ToFrag = glm::normalize(light2Pos - fragmentPos);
                            diffuseIntensity2 = std::max(0.0f, glm::dot(interpolatedNormal, light2ToFrag));
                            glm::vec3 reflectDir2 = glm::reflect(-light2ToFrag, interpolatedNormal);
                            specularIntensity2 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir2), 0.0f), material.n);
                        }

                        // Final color computation
                        glm::vec3 ambientLightColor = material.K.x * lighting.ambientIntensity * material.color;
                        glm::vec3 diffuseLightColor1 = material.K.y * diffuseIntensity1 * lighting.light1.intensity * material.color;
                        glm::vec3 diffuseLightColor2 = material.K.y * diffuseIntensity2 * lighting.light2.intensity * material.color;
                        glm::vec3 specularLightColor1 = material.K.z * specularIntensity1 * glm::vec3(1.0f);
                        glm::vec3 specularLightColor2 = material.K.z * specularIntensity2 * glm::vec3(1.0f);

                        glm::vec3 color;
                        

                        if (currentShadingMode == FLAT) {
                            color = ambientLightColor;
                        }
                        else if (currentShadingMode == GOURAUD) {
                            color = ambientLightColor + dampingFactor * (diffuseLightColor1
                                + light2enabled * (diffuseLightColor2));
                        }
                        else {
                            color = ambientLightColor + dampingFactor * (diffuseLightColor1 + specularLightColor1
                                + light2enabled * (diffuseLightColor2 + specularLightColor2));
                        }

                            
                        
                        // Convert color to RGBA format
                        unsigned int rgbaColor = material.ConvertVectortoUint32(color);

                        // Store the pixel
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

double Shading::restoreZ(double fz, float nearPlane, float farPlane) {
    double A = farPlane - nearPlane;
    double B = farPlane + nearPlane;
    double C = 2.0 * farPlane * nearPlane;

    return - C / (fz * A - B);
}

/*
void Shading::rasterize(
    const std::vector<glm::vec4>& worldVertices,
    const std::vector<glm::vec4>& worldNormals,
    const std::vector<glm::vec4>& vertices,
    const std::vector<glm::vec4>& normals,
    const std::vector<Wavefront_obj::Face>& faces,
    const Lighting& lighting,
    Material material,
    int screenWidth,
    int screenHeight) {

    Renderer renderer;
    std::vector<Pixel> pixels; // Container for visible pixels

    //for now light source is static
    // Light source properties
    glm::vec3 light1Dir = -lighting.light1.direction;
    light1Dir.z = -light1Dir.z;
    glm::vec3 light1Pos = lighting.light1.position;

    glm::vec3 light2Dir = -lighting.light2.direction;
    light2Dir.z = -light2Dir.z;
    glm::vec3 light2Pos = lighting.light2.position;

    float light2enabled = lighting.light2.enabled;
    float dampingFactor = 1.0f / (1 + light2enabled);

    // allocate and Initialize the Z-buffer with maximum depth
    double* zBuffer = new double[screenWidth * screenHeight];
    std::fill(zBuffer, zBuffer + screenWidth * screenHeight, std::numeric_limits<double>::max());

    // Loop through each face
    for (const auto& face : faces) {
        // Extract vertex positions
        glm::vec4 p0 = vertices[face.v[0]];
        glm::vec4 p1 = vertices[face.v[1]];
        glm::vec4 p2 = vertices[face.v[2]];

        //std::cout << "p0: (" << p0.x << ", " << p0.y << ", " << p0.z << ", " << p0.w << ")" << std::endl;

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


                        glm::vec3 interpolatedNormal = glm::normalize(bary0 * n0 + bary1 * n1 + bary2 * n2);
                        glm::vec3 fragmentPos = bary0 * v0 + bary1 * v1 + bary2 * v2;

                        // Light 1 calculations
                        float diffuseIntensity1 = 0.0f;
                        float specularIntensity1 = 0.0f;
                        if (lighting.light1.type == DIRECTIONAL) {
                            diffuseIntensity1 = std::max(0.0f, glm::dot(interpolatedNormal, light1Dir));
                            glm::vec3 reflectDir1 = glm::reflect(-light1Dir, interpolatedNormal);
                            specularIntensity1 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir1), 0.0f), material.n);
                        }
                        else if (lighting.light1.type == POINTY) {
                            glm::vec3 light1ToFrag = glm::normalize(light1Pos - fragmentPos);
                            diffuseIntensity1 = std::max(0.0f, glm::dot(interpolatedNormal, light1ToFrag));
                            glm::vec3 reflectDir1 = glm::reflect(-light1ToFrag, interpolatedNormal);
                            specularIntensity1 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir1), 0.0f), material.n);
                        }

                        // Light 2 calculations
                        float diffuseIntensity2 = 0.0f;
                        float specularIntensity2 = 0.0f;
                        if (lighting.light2.type == DIRECTIONAL) {
                            diffuseIntensity2 = std::max(0.0f, glm::dot(interpolatedNormal, light2Dir));
                            glm::vec3 reflectDir2 = glm::reflect(-light2Dir, interpolatedNormal);
                            specularIntensity2 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir2), 0.0f), material.n);
                        }
                        else if (lighting.light2.type == POINTY) {
                            glm::vec3 light2ToFrag = glm::normalize(light2Pos - fragmentPos);
                            diffuseIntensity2 = std::max(0.0f, glm::dot(interpolatedNormal, light2ToFrag));
                            glm::vec3 reflectDir2 = glm::reflect(-light2ToFrag, interpolatedNormal);
                            specularIntensity2 = std::pow(std::max(glm::dot(glm::normalize(-fragmentPos), reflectDir2), 0.0f), material.n);
                        }
                        

                        glm::vec3 color = material.K.x * lighting.ambientIntensity * material.color
                            + dampingFactor * (material.K.y * diffuseIntensity1 * material.color
                            + material.K.z * specularIntensity1 * glm::vec3(1.0f)
                            + light2enabled * (material.K.y * diffuseIntensity2 * material.color
                            + material.K.z * specularIntensity2 * glm::vec3(1.0f)));

                        // Convert color to RGBA format
                        unsigned int rgbaColor = material.ConvertVectortoUint32(color);


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
*/

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

void Shading::setZBufferMode(ZBufferMode mode) {
    this->currentZBufferMode = mode;
}
void Shading::setShadingMode(ShadingMode mode) {
    this->currentShadingMode = mode;
}