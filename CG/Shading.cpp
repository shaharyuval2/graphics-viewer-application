#include "Shading.h"

Shading::Shading() {
	currentZBufferMode = ZBUFFER_MODE_1;
	currentShadingMode = WIREFRAME;
}

Shading::Shading(ZBufferMode ZMode, ShadingMode ShadingMode) {
	currentZBufferMode = ZMode;
	currentShadingMode = ShadingMode;
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