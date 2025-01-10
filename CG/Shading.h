#ifndef SHADING_H
#define SHADING_H

#include <iostream>

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

    //print
    void printSettings() const;
private:
    ZBufferMode currentZBufferMode;
    ShadingMode currentShadingMode;
};

#endif // SHADING_H#pragma once 
