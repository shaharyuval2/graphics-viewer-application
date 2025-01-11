#ifndef SHADING_H
#define SHADING_H

#include <vector>
#include <glm/glm.hpp>
#include <iostream>
#include <algorithm>
#include <limits>
#include "Renderer.h"
#include "Obj Parser/wavefront_obj.h"
#include "Material.h"
#include "Lighting.h"

// Enumerations for Z-Buffer Modes and Shading Modes
enum ZBufferMode { ZBUFFER_MODE_1, ZBUFFER_MODE_2, ZBUFFER_MODE_3 };
enum ShadingMode { WIREFRAME, FLAT, GOURAUD, PHONG };

// Shading Class Declaration
class Shading {
public:
    // Constructor
    Shading();

    Shading(ZBufferMode ZMode, ShadingMode ShadingMode);

    // Setters
    void setZBufferMode(ZBufferMode mode);
    void setShadingMode(ShadingMode mode);

    //resterier
    void rasterize(
        const std::vector<glm::vec4>& vertices,             // Object vertices in projected space space
        const std::vector<glm::vec4>& normals,              // Object normals in projected space
        const std::vector<Wavefront_obj::Face>& faces,      // Triangles as indices into the vertex array
        const Lighting& lighting,                           // Lighting properties
        Material material,                                  // Material properties
        int screenWidth,                                    // Screen width in pixels
        int screenHeight                                    // Screen height in pixels
    );


    //print
    void printSettings() const;
private:
    ZBufferMode currentZBufferMode;
    ShadingMode currentShadingMode;
};

#endif // SHADING_H#pragma once 
