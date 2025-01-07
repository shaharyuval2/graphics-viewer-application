#pragma once
// Definitions in meshmodel.cpp
extern float spinX, spinY, spinZ;
extern float scale;
extern float transX, transY, transZ;
#pragma once

#include <vector>
#include <string>

#include <glm/glm.hpp>

#include "Obj Parser/wavefront_obj.h"
#include "HW1.h"
#include "Transform.h"

#include <set>
#include <tuple>

// MeshModel Class Declaration
class MeshModel {
private:
    Wavefront_obj meshData; // Stores mesh data
    Transform transform;

    glm::mat4 objectMatrix;
    glm::mat4 worldMatrix;
    std::vector<glm::vec4> coordinates; // center, unit x, unit y, unit z
    std::vector<glm::vec4> BB; // holds initial BB coordinates (XYZ XYz XyZ Xyz xYZ xYz xyZ xyz)

    glm::mat4 translationViewMatrix;
    glm::mat4 rotationViewMatrix;
    glm::mat4 viewMatrix;
    glm::vec3 nonhomoF;
    glm::vec4 objectCentroid;

    glm::mat4 viewportMatrix;
    float targetAspect;

    glm::mat4 totalMatrix;



    // Private methods
    void canonicalize();
    void initializeNormals();
    glm::vec3 nonhomogenous(glm::vec4 vec4);
    
    glm::mat4 buildViewMatrix(float cx, float cy, float cz);

public:
    // Constructor
    MeshModel(std::wstring filename, float width, float height);

    // Public methods
    std::vector<std::vector<glm::vec2>> MeshModel::ProjectToScreen(float n, float f, float t, float r, float normalFactor);
    void renderObj(std::vector<glm::vec2> screenPoints);
    void renderCoords(std::vector<glm::vec2> screenCoordinates);
    void renderNormals(std::vector<glm::vec2> screenNormals, std::vector<glm::vec2> screenPoints);
    void renderBB(std::vector<glm::vec2> screenBB);


    //old translations functions
    void objectRotation(float sx, float sy, float sz);
    void objectScaling(float a);

    void translation(float tx, float ty, float tz);
    void worldRotation(float sx, float sy, float sz);
    void worldScaling(float a);
    //new translation functions
    void applyObjectTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz);
    void applyWorldTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz);


    void applyViewMatrix(float cx, float cy, float cz);
    void lookAtObject(float cx, float cy, float cz);

    void updateViewPort(float width,float height);
};

