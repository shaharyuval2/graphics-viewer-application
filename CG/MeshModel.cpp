#include "MeshModel.h"
#include <iostream>
#include <algorithm>
#include <cmath>
#include "HW1.h"

const float PI = 3.141592653589793;
// Definitions in meshmodel.cpp
float spinX = 0.0f, spinY = 0.0f, spinZ = 0.0f;
float scale = 1.0f;
float transX = 0.0f, transY = 0.0f, transZ = 0.0f;


// Constructor: Loads the OBJ file and canonicalizes the coordinates
MeshModel::MeshModel(std::wstring filename, float width, float height, const Camera& camera,
    const Material& material, const Lighting& lighting, const Shading& shading) {

    this->width = width;
    this->height = height;

    transform = Transform();
    this->camera = camera;
    this->material = material;
    this->lighting = lighting;
    this->shading = shading;

    //initialize viewport matrix
    targetAspect = width / height;
    updateViewPort(width, height);

    //initialize coordinates
    coordinates.push_back(glm::vec4(0, 0, 0, 1));
    coordinates.push_back(glm::vec4(2, 0, 0, 1));
    coordinates.push_back(glm::vec4(0, 2, 0, 1));
    coordinates.push_back(glm::vec4(0, 0, 2, 1));


    bool result = meshData.load_file(filename);

    std::cout << "load_file returned" << std::endl;

    if (result) {
        std::cout << "The obj file was loaded successfully" << std::endl;
    }
    else {
        std::cerr << "Failed to load obj file" << std::endl;
    }

    std::cout << "The number of vertices in the model is: " << meshData.m_points.size() << std::endl;
    std::cout << "The number of triangles in the model is: " << meshData.m_faces.size() << std::endl;

    canonicalize(); // Prepare the object and find BB
    initializeNormals();
}

// Canonicalizes the object to fit within a bounding box and centers it
void MeshModel::canonicalize() {
    float x_max = meshData.m_points[0].x;
    float x_min = meshData.m_points[0].x;
    float y_max = meshData.m_points[0].y;
    float y_min = meshData.m_points[0].y;
    float z_max = meshData.m_points[0].z;
    float z_min = meshData.m_points[0].z;

    for (const auto& point : meshData.m_points) {
        x_max = std::max(x_max, point.x);
        x_min = std::min(x_min, point.x);
        y_max = std::max(y_max, point.y);
        y_min = std::min(y_min, point.y);
        z_max = std::max(z_max, point.z);
        z_min = std::min(z_min, point.z);
    }


    // Calculate centroid
    glm::vec3 centroid = glm::vec3(
        (x_max + x_min) / 2.0f,
        (y_max + y_min) / 2.0f,
        (z_max + z_min) / 2.0f
    );

    // Calculate initial BB (add centroid to BB)
    BB.push_back(glm::vec4(x_max, y_max, z_max, 1));
    BB.push_back(glm::vec4(x_max, y_max, z_min, 1));
    BB.push_back(glm::vec4(x_max, y_min, z_max, 1));
    BB.push_back(glm::vec4(x_max, y_min, z_min, 1));
    BB.push_back(glm::vec4(x_min, y_max, z_max, 1));
    BB.push_back(glm::vec4(x_min, y_max, z_min, 1));
    BB.push_back(glm::vec4(x_min, y_min, z_max, 1));
    BB.push_back(glm::vec4(x_min, y_min, z_min, 1));
    BB.push_back(glm::vec4(centroid.x, centroid.y, centroid.z, 1));

    // Center object at origin
    for (auto& point : meshData.m_points) {
        point.x -= centroid.x;
        point.y -= centroid.y;
        point.z -= centroid.z;
    }

    for (auto& point : BB) {
        point.x -= centroid.x;
        point.y -= centroid.y;
        point.z -= centroid.z;
    }

    // Scale to fit within bounding box
    float max_length = std::max({ x_max - x_min, y_max - y_min, z_max - z_min });
    float scale_factor = 10.0f / max_length;

    for (auto& point : meshData.m_points) {
        point.x *= scale_factor;
        point.y *= scale_factor;
        point.z *= scale_factor;
    }

    for (auto& point : BB) {
        point.x *= scale_factor;
        point.y *= scale_factor;
        point.z *= scale_factor;
    }


    transform.objectTranslate(0, 0, -20);
    
}

void MeshModel::initializeNormals() {
    //for every point save a list of the adjacent normals
    std::vector<std::vector<glm::vec3>> adjacent;
    adjacent.resize(meshData.m_points.size());
    for (const auto& face : meshData.m_faces) {
        //calculate normal
        glm::vec3 P1 = nonhomogenous(meshData.m_points[face.v[0]]);
        glm::vec3 P2 = nonhomogenous(meshData.m_points[face.v[1]]);
        glm::vec3 P3 = nonhomogenous(meshData.m_points[face.v[2]]);
        glm::vec3 V1 = P2 - P1;
        glm::vec3 V2 = P3 - P1;

        glm::vec3 normal = glm::cross(V1, V2);
        //save this normal in all of the points that are apart of the face
        adjacent[face.v[0]].push_back(normal);
        adjacent[face.v[1]].push_back(normal);
        adjacent[face.v[2]].push_back(normal);
    }

    //calculate the avarage normal for each point
    meshData.m_normals.resize(meshData.m_points.size());
    for (size_t i = 0; i < adjacent.size(); ++i) {
        const auto& adjacentNormals = adjacent[i];

        // Sum up all adjacent normals
        glm::vec3 sum(0.0f, 0.0f, 0.0f);
        for (const auto& normal : adjacentNormals) {
            sum += normal;
        }

        // Calculate the average
        glm::vec3 avg = sum / static_cast<float>(adjacentNormals.size());
        avg = glm::normalize(avg) / 2.0f;

        //calculate the normal initial coordinate
        meshData.m_normals[i] = glm::vec4(avg.x, avg.y, avg.z, 0);
        
    }
}

glm::vec3 MeshModel::nonhomogenous(glm::vec4 vec4) {
    return glm::vec3(vec4.x / vec4.w, vec4.y / vec4.w, vec4.z / vec4.w);
}



/*
void printMatrix(const glm::mat4& matrix, const std::string& name) {
    std::cout << name << ":\n";
    for (int i = 0; i < 4; ++i) {
        std::cout << "| ";
        for (int j = 0; j < 4; ++j) {
            std::cout << matrix[j][i] << " ";
        }
        std::cout << "|\n";
    }
    std::cout << std::endl;
}
*/




// Projects points to screen coordinates
std::vector<std::vector<glm::vec2>> MeshModel::ProjectToScreen(float normalFactor) {
    //scaling the normals according to the factor that the user inputed
    std::vector<glm::vec4> movedNormals;
    movedNormals.resize(meshData.m_points.size());
    for (size_t i = 0; i < meshData.m_points.size(); ++i) {
        movedNormals[i] =  normalFactor * meshData.m_normals[i] + meshData.m_points[i];
    }

    //calculate the total matrix
    objectMatrix = transform.getObjectTransformationMatrix();
    worldMatrix = transform.getWorldTransformationMatrix();
    viewMatrix = camera.getViewMatrix();
    projectionMatrix = camera.getProjectionMatrix();

    totalMatrix = projectionMatrix * (viewMatrix * (worldMatrix * objectMatrix));
    glm::mat4 untilWorldMatrix = worldMatrix * objectMatrix;


    almostClipVertices.reserve(meshData.m_points.size());
    std::vector<glm::vec2> finalScreenPoints;
    finalScreenPoints.reserve(meshData.m_points.size());
    for (auto& vec : meshData.m_points) {
        glm::vec4 v4 = totalMatrix * vec;
        almostClipVertices.push_back(v4);

        v4 /= v4.w;
        v4 = viewportMatrix * v4;
        finalScreenPoints.push_back(glm::vec2(v4.x, v4.y));
    }

    std::vector<glm::vec2> finalScreenCoordinates;
    for (auto& vec : coordinates) {
        glm::vec4 v4 = totalMatrix * vec;
        v4 /= v4.w;
        v4 = viewportMatrix * v4;
        finalScreenCoordinates.push_back(glm::vec2(v4.x, v4.y));
    }

    almostClipNormals.reserve(movedNormals.size());
    std::vector<glm::vec2> finalScreenNormals;
    finalScreenNormals.reserve(movedNormals.size());

    for (size_t i = 0; i < movedNormals.size(); ++i) {
        glm::vec4 transformedMovedNormal = totalMatrix * movedNormals[i];
        glm::vec4 clipVertex = almostClipVertices[i];
        glm::vec4 clipNormal = transformedMovedNormal - clipVertex;
        clipNormal = glm::vec4(glm::normalize(glm::vec3(clipNormal)), 0.0f); // Ensure w = 0 for directional vectors

        // Save the result in almostClipNormals
        almostClipNormals.push_back(clipNormal);

        glm::vec4 v4 = transformedMovedNormal;
        v4 /= v4.w;
        v4 = viewportMatrix * v4;
        finalScreenNormals.push_back(glm::vec2(v4.x, v4.y));
    }

    std::vector<glm::vec2> finalScreenBB;
    for (auto& vec : BB) {
        glm::vec4 v4 = totalMatrix * vec;
        v4 /= v4.w;
        v4 = viewportMatrix * v4;
        finalScreenBB.push_back(glm::vec2(v4.x, v4.y));
    }

    //calculate the BB centroid in world coordinates
    std::vector<glm::vec4> worldBB;
    for (auto& vec : BB) {
        worldBB.push_back(untilWorldMatrix * vec);
    }
    objectCentroid = worldBB[8];


    return { finalScreenPoints, finalScreenCoordinates, finalScreenNormals, finalScreenBB};
}

// Renders the object using the provided screen points
void MeshModel::renderObj(std::vector<glm::vec2> screenPoints) {
    std::set<std::tuple<int, int>> plottedEdges;

    for (const auto& face : meshData.m_faces) {
        int v0 = face.v[0];
        int v1 = face.v[1];
        int v2 = face.v[2];

        // Helper lambda to plot line and track it
        auto plotUniqueLine = [&](int a, int b) {
            if (a > b) std::swap(a, b);  // Ensure (a,b) == (b,a)
            auto edge = std::make_tuple(a, b);
            if (plottedEdges.find(edge) == plottedEdges.end()) {
                plottedEdges.insert(edge);
                plotLine((int)screenPoints[a].x, (int)screenPoints[b].x,
                    (int)screenPoints[a].y, (int)screenPoints[b].y);
            }
        };

        // Plot lines, ensuring each is plotted once
        plotUniqueLine(v0, v1);
        plotUniqueLine(v1, v2);
        plotUniqueLine(v2, v0);
    }
}


void MeshModel::renderCoords(std::vector<glm::vec2> screenCoordinates) {
    
    Color = 0xff0000ff; // x - red
    plotLine((int)screenCoordinates[0].x, (int)screenCoordinates[1].x,
        (int)screenCoordinates[0].y, (int)screenCoordinates[1].y);

    Color = 0xff00ff00; // y - green
    plotLine((int)screenCoordinates[0].x, (int)screenCoordinates[2].x,
        (int)screenCoordinates[0].y, (int)screenCoordinates[2].y);

    Color = 0xffff0000; // z - blue
    plotLine((int)screenCoordinates[0].x, (int)screenCoordinates[3].x,
        (int)screenCoordinates[0].y, (int)screenCoordinates[3].y);

    Color = 0xffffffff; // return to white
}

void MeshModel::renderNormals(std::vector<glm::vec2> screenNormals, std::vector<glm::vec2> screenPoints) {
    Color = 0xff00ff00; // green
    for (size_t i = 0; i < meshData.m_points.size(); ++i) {
        plotLine((int)screenPoints[i].x, (int)screenNormals[i].x,
            (int)screenPoints[i].y, (int)screenNormals[i].y);
    }
    Color = 0xffffffff; // return to white
}

void MeshModel::renderBB(std::vector<glm::vec2> screenBB) {
    //plot the 12 appropriate lines in lightblue
    Color  = 0xffff00ff;
    
    //XYZ
    plotLine((int)screenBB[0].x, (int)screenBB[1].x,
        (int)screenBB[0].y, (int)screenBB[1].y); //XYZ to XYz

    plotLine((int)screenBB[0].x, (int)screenBB[2].x,
        (int)screenBB[0].y, (int)screenBB[2].y); //XYZ to XyZ

    plotLine((int)screenBB[0].x, (int)screenBB[4].x,
        (int)screenBB[0].y, (int)screenBB[4].y); //XYZ to xYZ

    //Xyz
    plotLine((int)screenBB[3].x, (int)screenBB[2].x,
        (int)screenBB[3].y, (int)screenBB[2].y); //Xyz to XyZ

    plotLine((int)screenBB[3].x, (int)screenBB[1].x,
        (int)screenBB[3].y, (int)screenBB[1].y); //Xyz to XYz

    plotLine((int)screenBB[3].x, (int)screenBB[7].x,
        (int)screenBB[3].y, (int)screenBB[7].y); //Xyz to xyz


    //xyZ
    plotLine((int)screenBB[6].x, (int)screenBB[7].x,
        (int)screenBB[6].y, (int)screenBB[7].y); //xyZ to xyz

    plotLine((int)screenBB[6].x, (int)screenBB[4].x,
        (int)screenBB[6].y, (int)screenBB[4].y); //xyZ to xYZ

    plotLine((int)screenBB[6].x, (int)screenBB[2].x,
        (int)screenBB[6].y, (int)screenBB[2].y); //xyZ to XyZ

    //xYz
    plotLine((int)screenBB[5].x, (int)screenBB[4].x,
        (int)screenBB[5].y, (int)screenBB[4].y); //xYz to xYZ

    plotLine((int)screenBB[5].x, (int)screenBB[7].x,
        (int)screenBB[5].y, (int)screenBB[7].y); //xYz to xyz

    plotLine((int)screenBB[5].x, (int)screenBB[1].x,
        (int)screenBB[5].y, (int)screenBB[1].y); //xYz to XYz

    
    Color = 0xffffffff; // return to white
}

void MeshModel::applyObjectTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz) {
    transform.objectRotate(sx, sy, sz);
    transform.objectScale(factor);
    transform.objectTranslate(tx, ty, tz);
}

void MeshModel::applyWorldTransformations(float sx, float sy, float sz, float factor, float tx, float ty, float tz) {
    transform.worldRotate(sx, sy, sz);
    transform.worldScale(factor);
    transform.worldTranslate(tx, ty, tz);
}

void MeshModel::moveCameraPosition(float dcx, float dcy, float dcz) {
    camera.movePosition(dcx, dcy, dcz);
}

void MeshModel::lookAt() {
    camera.lookAt(objectCentroid);
}

void MeshModel::updateProjectMatrix(float mnear, float mfar, float top, float right) {
    camera.updateProjectionMatrix(mnear, mfar, top, right);
}

void MeshModel::updateViewPort(float width, float height) {
    this->width = width;
    this->height = height;

    if (width <= 0 || height <= 0) {
        return; // Prevent invalid window sizes
    }
    // Define the target aspect ratio (e.g., 16:9 or any desired aspect ratio)
    float windowAspect = static_cast<float>(width) / static_cast<float>(height);

    int viewportWidth = width, viewportHeight = height;

    // Adjust viewport to maintain aspect ratio
    if (windowAspect > targetAspect) {
        // Window is wider than target aspect ratio
        viewportWidth = static_cast<int>(height * targetAspect);
    }
    else if (windowAspect < targetAspect) {
        // Window is taller than target aspect ratio
        viewportHeight = static_cast<int>(width / targetAspect);
    }
    // Calculate the center-based offsets for symmetrical letterboxing
    float centerOffsetX = width / 2.0f;
    float centerOffsetY = height / 2.0f;

    // Construct the viewport matrix with symmetrical letterboxing
    viewportMatrix = glm::mat4(
        viewportWidth /2.0f, 0.0f, 0.0f, 0.0f,           // Scale X
        0.0f, viewportHeight /2.0f, 0.0f, 0.0f,          // Scale Y
        0.0f, 0.0f, 1.0f, 0.0f,                   // Scale Z
        centerOffsetX, centerOffsetY, 0.0f, 1.0f  // Translate to the center
    );
}


void MeshModel::rasterize() {
    shading.rasterize(almostClipVertices, almostClipNormals, meshData.m_faces, lighting.ambientIntensity, material.color,material.K.z, material.n, width, height);
}
