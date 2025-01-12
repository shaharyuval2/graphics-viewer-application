#include "Shading.h"



Shading::Shading() {
	currentZBufferMode = ZBUFFER_MODE_1;
	currentShadingMode = WIREFRAME;
}

Shading::Shading(ZBufferMode ZMode, ShadingMode ShadingMode,SidedMode SideMode) {
	currentZBufferMode = ZMode;
	currentShadingMode = ShadingMode;
    currentSidedMode = SideMode;
}

/*
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

        // Compute lighting for Flat and Gouraud shading
        glm::vec3 flatColor, gouraudColor0, gouraudColor1, gouraudColor2;
        if (currentShadingMode == FLAT) {
            // Compute flat shading color using the triangle's first vertex


            glm::vec3 triangleNormal = glm::normalize(glm::cross(worldP1 - worldP0, worldP2 - worldP0));

            // Compute lighting at centroid
            flatColor = computeLighting(worldP0, triangleNormal, lighting, material, light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
        }
        else if (currentShadingMode == GOURAUD) {
            // Compute Gouraud shading colors for each vertex
            gouraudColor0 = computeLighting(worldP0, n0, lighting, material, light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
            gouraudColor1 = computeLighting(worldP1, n1, lighting, material, light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
            gouraudColor2 = computeLighting(worldP2, n2, lighting, material, light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
        }

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


                        glm::vec3 color;
                        if (currentShadingMode == FLAT) {
                            color = flatColor;
                        }
                        else if (currentShadingMode == GOURAUD) {
                            color = bary0 * gouraudColor0 + bary1 * gouraudColor1 + bary2 * gouraudColor2;
                        }
                        else if (currentShadingMode == PHONG) {

                            // Phong shading already implemented
                            glm::vec3 fragmentPos = bary0 * worldP0 + bary1 * worldP1 + bary2 * worldP2;
                            glm::vec3 interpolatedNormal = glm::normalize(bary0 * n0 + bary1 * n1 + bary2 * n2);
                            color = computeLighting(fragmentPos, interpolatedNormal, lighting, material,
                                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
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
*/

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

    //printSettings();

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

        // Compute face normal for one-sided and double-sided checks
        //bool flipNormals;
        glm::vec3 faceNormal = glm::normalize(glm::cross(worldP1 - worldP0, worldP2 - worldP0));

        // Check if face should be culled (one-sided mode)
        if (currentSidedMode == ONE_SIDED) {
            glm::vec3 viewDir = glm::normalize(camera.getPosition() - worldP0); // Camera position in world space
            if (glm::dot(faceNormal, viewDir) <= 0.0f) {
                //continue; // Cull the back face
            }
        }
        else if (currentSidedMode == DOUBLE_SIDED) {
            glm::vec3 viewDir = glm::normalize(camera.getPosition() - worldP0); // Camera position in world space
            if (glm::dot(faceNormal, viewDir) < 0.0f) {
                // Flip the normal for back-facing triangles
                faceNormal = -faceNormal;
                n0 = -n0;
                n1 = -n1;
                n2 = -n2;
                //light1Dir = -light1Dir;
                //light2Dir = -light2Dir;
            }
        }

        // Compute lighting for Flat and Gouraud shading
        glm::vec3 flatColor, gouraudColor0, gouraudColor1, gouraudColor2;
        if (currentShadingMode == FLAT) {
            flatColor = computeLighting(worldP0, faceNormal, lighting, material,
                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
        }
        else if (currentShadingMode == GOURAUD) {
            gouraudColor0 = computeLighting(worldP0, n0, lighting, material,
                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
            gouraudColor1 = computeLighting(worldP1, n1, lighting, material,
                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
            gouraudColor2 = computeLighting(worldP2, n2, lighting, material,
                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
        }

        // Perspective divide to transform clip space vertices to screen space
        glm::vec4 v0 = clipVertices[face.v[0]];
        glm::vec4 v1 = clipVertices[face.v[1]];
        glm::vec4 v2 = clipVertices[face.v[2]];

        // Perform clipping
        if (isTriangleOutsideFrustum(v0, v1, v2)) {
            continue; // Discard the triangle if it's completely outside the frustum
        }

        // Perspective divide to transform clip space vertices to screen space
        v0 /= clipVertices[face.v[0]].w;
        v1 /= clipVertices[face.v[1]].w;
        v2 /= clipVertices[face.v[2]].w;

        // Map to screen coordinates
        auto toScreen = [screenWidth, screenHeight](const glm::mat4& viewportMatrix, const glm::vec4& v) -> glm::ivec2 {
            glm::vec4 viewportV4 = viewportMatrix * v;
            return glm::ivec2(viewportV4.x, viewportV4.y);
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
                if (area == 0.0f) continue;

                float w0 = (x - s1.x) * (s2.y - s1.y) - (y - s1.y) * (s2.x - s1.x);
                float w1 = (x - s2.x) * (s0.y - s2.y) - (y - s2.y) * (s0.x - s2.x);
                float bary0 = w0 / area, bary1 = w1 / area, bary2 = 1 - bary0 - bary1;

                if (bary0 >= 0 && bary1 >= 0 && bary2 >= 0) {
                    // Interpolate depth and perform Z-buffer test
                    int bufferIndex = y * screenWidth + x;
                    double depth = bary0 * v0.z + bary1 * v1.z + bary2 * v2.z;

                    if (depth < zBuffer[bufferIndex]) {
                        zBuffer[bufferIndex] = depth;

                        // Compute color based on shading mode
                        glm::vec3 color;
                        if (currentShadingMode == FLAT) {
                            color = flatColor;
                        }
                        else if (currentShadingMode == GOURAUD) {
                            color = bary0 * gouraudColor0 + bary1 * gouraudColor1 + bary2 * gouraudColor2;
                        }
                        else if (currentShadingMode == PHONG) {
                            glm::vec3 fragmentPos = bary0 * worldP0 + bary1 * worldP1 + bary2 * worldP2;
                            glm::vec3 interpolatedNormal = glm::normalize(bary0 * n0 + bary1 * n1 + bary2 * n2);
                            color = computeLighting(fragmentPos, interpolatedNormal, lighting, material,
                                light1Dir, light1Pos, light2Dir, light2Pos, dampingFactor, light2enabled);
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


glm::vec3 Shading::reflect(const glm::vec3& lightDir, const glm::vec3& normal) {
    return lightDir - 2.0f * glm::dot(normal, lightDir) * normal;
}

glm::vec3 Shading::computeLighting(const glm::vec3& position, const glm::vec3& normal, const Lighting& lighting, Material material,
    const glm::vec3& light1Dir, const glm::vec3& light1Pos, const glm::vec3& light2Dir, const glm::vec3& light2Pos,
    float dampingFactor, float light2enabled) {
    // Ambient light contribution
    glm::vec3 ambientLightColor = material.K.x * lighting.ambientIntensity * material.color;

    // Light 1 (Directional or Point)
    glm::vec3 diffuseLightColor1(0.0f);
    glm::vec3 specularLightColor1(0.0f);

    if (lighting.light1.type == DIRECTIONAL) {
        glm::vec3 lightDir = glm::normalize(light1Dir); // Light direction for directional light
        float diffuseIntensity = std::max(0.0f, glm::dot(normal, lightDir));
        diffuseLightColor1 = material.K.y * diffuseIntensity * lighting.light1.intensity * material.color;

        glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
        float specularIntensity = std::pow(std::max(glm::dot(glm::normalize(-position), reflectDir), 0.0f), material.n);
        specularLightColor1 = material.K.z * specularIntensity * glm::vec3(1.0f);
    }
    else if (lighting.light1.type == POINTY) {
        glm::vec3 lightDir = glm::normalize(light1Pos - position); // Direction from fragment to light
        float diffuseIntensity = std::max(0.0f, glm::dot(normal, lightDir));
        diffuseLightColor1 = material.K.y * diffuseIntensity * lighting.light1.intensity * material.color;

        glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
        float specularIntensity = std::pow(std::max(glm::dot(glm::normalize(-position), reflectDir), 0.0f), material.n);
        specularLightColor1 = material.K.z * specularIntensity * glm::vec3(1.0f);
    }

    // Light 2 (Directional or Point)
    glm::vec3 diffuseLightColor2(0.0f);
    glm::vec3 specularLightColor2(0.0f);

    if (light2enabled > 0) { // Only calculate if light 2 is enabled
        if (lighting.light2.type == DIRECTIONAL) {
            glm::vec3 lightDir = glm::normalize(light2Dir); // Light direction for directional light
            float diffuseIntensity = std::max(0.0f, glm::dot(normal, lightDir));
            diffuseLightColor2 = material.K.y * diffuseIntensity * lighting.light2.intensity * material.color;

            glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
            float specularIntensity = std::pow(std::max(glm::dot(glm::normalize(-position), reflectDir), 0.0f), material.n);
            specularLightColor2 = material.K.z * specularIntensity * glm::vec3(1.0f);
        }
        else if (lighting.light2.type == POINTY) {
            glm::vec3 lightDir = glm::normalize(light2Pos - position); // Direction from fragment to light
            float diffuseIntensity = std::max(0.0f, glm::dot(normal, lightDir));
            diffuseLightColor2 = material.K.y * diffuseIntensity * lighting.light2.intensity * material.color;

            glm::vec3 reflectDir = glm::reflect(-lightDir, normal);
            float specularIntensity = std::pow(std::max(glm::dot(glm::normalize(-position), reflectDir), 0.0f), material.n);
            specularLightColor2 = material.K.z * specularIntensity * glm::vec3(1.0f);
        }
    }

    // Combine lighting components
    return ambientLightColor + dampingFactor * (diffuseLightColor1 + specularLightColor1 +
        light2enabled * (diffuseLightColor2 + specularLightColor2));
}

bool Shading::isTriangleOutsideFrustum(const glm::vec4& v0, const glm::vec4& v1, const glm::vec4& v2) {
    // Check each vertex against all 6 clipping planes (left, right, top, bottom, near, far)
    int outsideCount[6] = { 0 }; // Count vertices outside each plane

    auto checkVertex = [](const glm::vec4& v, int& left, int& right, int& top, int& bottom, int& mnear, int& mfar) {
        if (v.x < -v.w) ++left;    // Left plane
        if (v.x > v.w) ++right;    // Right plane
        if (v.y < -v.w) ++bottom;  // Bottom plane
        if (v.y > v.w) ++top;      // Top plane
        if (v.z < -v.w) ++mnear;    // Near plane
        if (v.z > v.w) ++mfar;      // Far plane
    };

    checkVertex(v0, outsideCount[0], outsideCount[1], outsideCount[2], outsideCount[3], outsideCount[4], outsideCount[5]);
    checkVertex(v1, outsideCount[0], outsideCount[1], outsideCount[2], outsideCount[3], outsideCount[4], outsideCount[5]);
    checkVertex(v2, outsideCount[0], outsideCount[1], outsideCount[2], outsideCount[3], outsideCount[4], outsideCount[5]);

    // If all three vertices are outside any one plane, discard the triangle
    for (int i = 0; i < 6; ++i) {
        if (outsideCount[i] == 3) return true;
    }

    return false; // Triangle is within the frustum
}





double Shading::restoreZ(double fz, float nearPlane, float farPlane) {
    double A = farPlane - nearPlane;
    double B = farPlane + nearPlane;
    double C = 2.0 * farPlane * nearPlane;

    return - C / (fz * A - B);
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

    // Print Sided Mode
    std::cout << "  Sided Mode: ";
    switch (currentSidedMode) {
    case ONE_SIDED:
        std::cout << "One Sided" << std::endl;
        break;
    case DOUBLE_SIDED:
        std::cout << "Double Sided" << std::endl;
        break;
    }
}

void Shading::setZBufferMode(ZBufferMode mode) {
    this->currentZBufferMode = mode;
}
void Shading::setShadingMode(ShadingMode mode) {
    this->currentShadingMode = mode;
}
void Shading::setSidedMode(SidedMode mode) {
    this->currentSidedMode = mode;
}