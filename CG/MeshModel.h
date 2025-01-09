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
#include "Camera.h"
#include "Material.h"

#include <set>
#include <tuple>

// MeshModel Class Declaration
class MeshModel {
private:
    Wavefront_obj meshData; // Stores mesh data
    Transform transform;
    Camera camera;
    Material material;

    glm::mat4 objectMatrix;
    glm::mat4 worldMatrix;
    std::vector<glm::vec4> coordinates; // center, unit x, unit y, unit z
    std::vector<glm::vec4> BB; // holds initial BB coordinates (XYZ XYz XyZ Xyz xYZ xYz xyZ xyz)

    //camera stuff
    glm::mat4 viewMatrix;
    glm::mat4 projectionMatrix;
    glm::vec4 objectCentroid;

    glm::mat4 viewportMatrix;
    float targetAspect;

    glm::mat4 totalMatrix;

    //HW3
    //glm::vec3 material; // (ka,kd,ks)



    // Private methods
    void canonicalize();
    void initializeNormals();
    glm::vec3 nonhomogenous(glm::vec4 vec4);

public:
    // Constructor
    MeshModel(std::wstring filename, float width, float height, float mnear, float mfar, float top, float right);

    // Public methods
    std::vector<std::vector<glm::vec2>> MeshModel::ProjectToScreen(float normalFactor);
    void renderObj(std::vector<glm::vec2> screenPoints);
    void renderCoords(std::vector<glm::vec2> screenCoordinates);
    void renderNormals(std::vector<glm::vec2> screenNormals, std::vector<glm::vec2> screenPoints);
    void renderBB(std::vector<glm::vec2> screenBB);

    //translation functions
    void applyObjectTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz);
    void applyWorldTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz);


    //new camera
    void moveCameraPosition(float dcx, float dcy, float dcz);
    void lookAt();
    void updateProjectMatrix(float mnear, float mfar, float top, float right);

    void updateViewPort(float width,float height);
};

