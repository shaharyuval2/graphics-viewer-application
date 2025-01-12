#include <iostream>

#include <AntTweakBar/include/AntTweakBar.h>
#include <Glew/include/gl/glew.h>
#include <freeglut/include/GL/freeglut.h>

#include <vector>
#include <Windows.h>
#include <assert.h>

#include "Utils.h"
#include "Renderer.h"
#include "Obj Parser/wavefront_obj.h"
#include "HW1.h"


#include "MeshModel.h"


#include <glm/mat4x4.hpp>
#include <glm/vec4.hpp>



LARGE_INTEGER StartingTime, EndingTime, ElapsedMicroseconds;
LARGE_INTEGER Frequency;

//HW2 variables
MeshModel* myMesh = nullptr; // Global pointer to MeshModel
std::vector<glm::vec2> screenPoints; // Store projected screen points

bool isObjectCoords = true;
bool showNormals = false;
bool showCoords = false;
bool showBB = false;
float normalFactor = 1.0f;

//camera position
float camera_x = 0.0f;
float camera_y = 0.0f;
float camera_z = 0.0f;

float dcamera_x = 0.0f;
float dcamera_y = 0.0f;
float dcamera_z = 0.0f;


//screen variables
float width;
float height;
float targetAspect;

// frustum variables
float frustum_near = 1.0f;
float frustum_far = 1000.0f;
float frustum_top = 0.414f;
float frustum_right;
float frustum_fov = 45.0f;

//material properties
float ka = 0.1f; // Ambient coefficient
float kd = 0.5f; // Diffuse coefficient
float ks = 0.3f; // Specular coefficient
float n = 16.0f;  // Shininess exponent
uint32_t materialColor = 0xffffffff; // Material color (RGBA format)

//light properties
LightType light1Type = DIRECTIONAL;
LightType light2Type = DIRECTIONAL;
bool light2Enabled = false;
glm::vec3 light1Position(0.0f, 0.0f, 0.0f); // Position for light 1 (if it's a point light)
glm::vec3 light2Position(0.0f, 0.0f, 0.0f); // Position for light 2 (if it's a point light)
glm::vec3 light1Direction(0.0f, 0.0f, -1.0f); // Direction for light 1 (if it's directional)
glm::vec3 light2Direction(0.0f, 0.0f, -1.0f); // Direction for light 2 (if it's directional)
glm::vec3 light1Intensity(1.0f, 1.0f, 1.0f); // RGB intensity for light 1
glm::vec3 light2Intensity(1.0f, 1.0f, 1.0f); // RGB intensity for light 2
glm::vec3 ambientLightIntensity(1.0f, 1.0f, 1.0f); // Global ambient light intensity

//shading properties
ZBufferMode currentZBufferMode = ZBUFFER_MODE_1;
ShadingMode currentShadingMode = PHONG;
SidedMode currentSidedMode = ONE_SIDED;




void TW_CALL loadOBJModel(void* clientData);
void initScene();
void initGraphics(int argc, char* argv[]);
void drawScene();
void Display();
void Reshape(int width, int height);
void MouseButton(int button, int state, int x, int y);
void MouseMotion(int x, int y);
void PassiveMouseMotion(int x, int y);
void Keyboard(unsigned char k, int x, int y);
void Special(int k, int x, int y);
void Terminate(void);
void ApplyTransformations();
void LookAt();
// Callback to enforce frustum constraints
void TW_CALL ValidateFrustumNear(const void* value, void* clientData);
void TW_CALL GetFrustumNear(void* value, void* clientData);
void TW_CALL ValidateFrustumFar(const void* value, void* clientData);
void TW_CALL GetFrustumFar(void* value, void* clientData);
//update frustum using the near far and fov
void UpdateFrustum();
void TW_CALL SetFrustumFOV(const void* value, void* clientData);
void TW_CALL GetFrustumFOV(void* value, void* clientData);

//camera functions
void TW_CALL updateCameraTranslationX(const void* value, void* clientData);
void TW_CALL updateCameraTranslationY(const void* value, void* clientData);
void TW_CALL updateCameraTranslationZ(const void* value, void* clientData);

int main(int argc, char* argv[])
{
	// Initialize openGL, glut, glew
	initGraphics(argc, argv);
	// Initialize AntTweakBar
	TwInit(TW_OPENGL, NULL);
	//initialize the timer frequency
	QueryPerformanceFrequency(&Frequency);
	// Set GLUT callbacks
	glutDisplayFunc(Display);
	glutReshapeFunc(Reshape);
	glutMouseFunc(MouseButton);
	glutMotionFunc(MouseMotion);
	glutPassiveMotionFunc(PassiveMouseMotion);
	glutKeyboardFunc(Keyboard);
	glutSpecialFunc(Special);

	width = static_cast<float>(glutGet(GLUT_WINDOW_WIDTH));
	height = static_cast<float>(glutGet(GLUT_WINDOW_HEIGHT));
	targetAspect = width / height;
	frustum_right = frustum_top * targetAspect;

	//send 'glutGetModifers' function pointer to AntTweakBar.
	//required because the GLUT key event functions do not report key modifiers states.
	//TwGLUTModifiersFunc(glutGetModifiers);


	atexit(Terminate);  //called after glutMainLoop ends


	// Create a tweak bar
	TwBar* bar = TwNewBar("TweakBar");

	TwDefine(" GLOBAL help='This example shows how to integrate AntTweakBar with GLUT and OpenGL.' "); // Message added to the help bar.
	TwDefine(" TweakBar size='200 400' color='96 216 224' "); // Change default tweak bar size and color

	//mode widget
	// Define the enum labels for the tweak bar
	const TwEnumVal modeEV[] = {
		{ DISABLE, "Disable" },
		{ TRIANGLE, "Triangle" },
		{ SQUARE, "Square" },
		{ ROTATED_SQUARE, "Rotated Square" },
		{ STAR, "Star" },
		{ BONUS_CIRCLE, "Bonus Circle" }
	};
	const TwType modeType = TwDefineEnum("ModeType", modeEV, 6);
	TwAddVarRW(bar, "Mode", modeType, &currentMode, " label='Drawing Mode' help='Choose the shape drawing mode.' ");

	//open file widget
	TwAddButton(bar, "open", loadOBJModel, NULL, " label='Open OBJ File...' ");

	// Add transformations group
	TwAddVarRW(bar, "Rotation X", TW_TYPE_FLOAT, &spinX, " min=-360 max=360 step=1 group='Transformations' label='Rotation X' ");
	TwAddVarRW(bar, "Rotation Y", TW_TYPE_FLOAT, &spinY, " min=-360 max=360 step=1 group='Transformations' label='Rotation Y' ");
	TwAddVarRW(bar, "Rotation Z", TW_TYPE_FLOAT, &spinZ, " min=-360 max=360 step=1 group='Transformations' label='Rotation Z' ");
	TwAddVarRW(bar, "Scale", TW_TYPE_FLOAT, &scale, " min=0.1 max=10.0 step=0.1 group='Transformations' label='Scale' ");
	TwAddVarRW(bar, "Translation X", TW_TYPE_FLOAT, &transX, " min=-5 max=5 step=0.1 group='Transformations' label='Translation X' ");
	TwAddVarRW(bar, "Translation Y", TW_TYPE_FLOAT, &transY, " min=-5 max=5 step=0.1 group='Transformations' label='Translation Y' ");
	TwAddVarRW(bar, "Translation Z", TW_TYPE_FLOAT, &transZ, " min=-5 max=5 step=0.1 group='Transformations' label='Translation Z' ");

	//camera translations
	TwAddVarRW(bar, "camera Translation X", TW_TYPE_FLOAT, &dcamera_x, " min=-5 max=5 step=0.1 group='Transformations' label='camera Translation X' ");
	TwAddVarRW(bar, "camera Translation Y", TW_TYPE_FLOAT, &dcamera_y, " min=-5 max=5 step=0.1 group='Transformations' label='camera Translation Y' ");
	TwAddVarRW(bar, "camera Translation Z", TW_TYPE_FLOAT, &dcamera_z, " min=-5 max=5 step=0.1 group='Transformations' label='camera Translation Z' ");
	// Add buttons to select coordinate system
	TwAddButton(bar, "Object Coordinates", [](void*) { isObjectCoords = true; }, nullptr, " group='Transformations' label='Object Coordinates' ");
	TwAddButton(bar, "World Coordinates", [](void*) { isObjectCoords = false; }, nullptr, " group='Transformations' label='World Coordinates' ");

	// Add Apply button
	TwAddButton(bar, "Apply", [](void*) { ApplyTransformations(); }, nullptr, " group='Transformations' label='Apply Transformations' ");

	// add LookAt button
	TwAddButton(bar, "LookAt", [](void*) { LookAt(); }, nullptr, " group='Transformations' label='LookAt' ");

	// "Show" buttons
	// Add the "Show Normals" toggle button to the tweak bar
	TwAddVarRW(bar, "Show Normals", TW_TYPE_BOOLCPP, &showNormals, " label='Show Normals' group='Transformations' help='Toggle to show or hide normals' ");
	TwAddVarRW(bar, "normal Factor", TW_TYPE_FLOAT, &normalFactor, " min=0.1 max=10 step=0.1 group='Transformations' label='normal factor' ");

	// Add the "Show Coordinates" toggle button to the tweak bar
	TwAddVarRW(bar, "Show Coordinates", TW_TYPE_BOOLCPP, &showCoords, " label='Show Coordinates' group='Transformations' help='Toggle to show or hide object coordinates' ");

	// Add the "Show Coordinates" toggle button to the tweak bar
	TwAddVarRW(bar, "Show Bounding Box", TW_TYPE_BOOLCPP, &showBB, " label='Show BB' group='Transformations' help='Toggle to show or hide Bounding Box' ");
	TwDefine(" TweakBar/'Transformations' opened=false "); // Transformations Attributes folder

	// Add frustum widgets
	TwAddVarCB(bar, "Near Plane", TW_TYPE_FLOAT, ValidateFrustumNear, GetFrustumNear, nullptr," min=0.1 max=500.0 step=0.1 group='Frustum properties' help='Adjust the near plane (must be less than far)' ");
	TwAddVarCB(bar, "Far Plane", TW_TYPE_FLOAT, ValidateFrustumFar, GetFrustumFar, nullptr," min=1.0 max=2000.0 step=1.0 group='Frustum properties' help='Adjust the far plane (must be greater than near)' ");
	TwAddVarRW(bar, "Top Plane", TW_TYPE_FLOAT, &frustum_top," min=0.001 max=10.0 step=0.01 group='Frustum properties' help='Adjust the top plane value' ");
	TwAddVarRW(bar, "Right Plane", TW_TYPE_FLOAT, &frustum_right," min=0.001 max=10.0 step=0.01 group='Frustum properties' help='Adjust the right plane value' ");
	TwAddVarCB(bar, "Field of View", TW_TYPE_FLOAT, SetFrustumFOV, GetFrustumFOV, nullptr," min=1.0 max=180.0 step=1.0 group='Frustum properties' help='Adjust the field of view (FOV) angle' ");
	TwDefine(" TweakBar/'Frustum properties' opened=false "); // frustum Attributes folder
	
	// Add material widgets
	TwAddVarRW(bar, "Ka", TW_TYPE_FLOAT, &ka, " min=0 max=1 step=0.01 group='Material Properties' label='Ka' help='Adjust ambient reflectance coefficient.' ");
	TwAddVarRW(bar, "Kd", TW_TYPE_FLOAT, &kd, " min=0 max=1 step=0.01 group='Material Properties' label='Kd' help='Adjust diffuse reflectance coefficient.' ");
	TwAddVarRW(bar, "Ks", TW_TYPE_FLOAT, &ks, " min=0 max=1 step=0.01 group='Material Properties' label='Ks' help='Adjust specular reflectance coefficient.' ");
	TwAddVarRW(bar, "N", TW_TYPE_FLOAT, &n, " min=1 max=100 step=1 group='Material Properties' label='Shininess (N)' help='Adjust shininess exponent for specular highlights.' ");
	TwAddVarRW(bar, "Color", TW_TYPE_COLOR32, &materialColor, " group='Material Properties' label='Color' help='Set the material color.' ");
	TwDefine(" TweakBar/'Material Properties' opened=false "); // material Attributes folder

	//Add light widgets
	TwAddVarRW(bar, "Ambient Light Intensity", TW_TYPE_COLOR3F, &ambientLightIntensity, " label='Ambient Light' group='Light Sources' help='Global ambient light intensity' ");
	// Light 1 Controls
	TwAddVarRW(bar, "Light 1 Type", TwDefineEnum("LightType", nullptr, 0), &light1Type, " enum='0 {Directional}, 1 {Point}' label='Light 1 Type' group='Light Sources' ");
	TwAddVarRW(bar, "Light 1 Direction", TW_TYPE_DIR3F, &light1Direction, " label='Light 1 Direction' group='Light Sources' help='Direction of light 1 (if directional)' ");
	TwAddVarRW(bar, "Light 1 Position X", TW_TYPE_FLOAT, &light1Position.x, " label='Light 1 X' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 1 Position Y", TW_TYPE_FLOAT, &light1Position.y, " label='Light 1 Y' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 1 Position Z", TW_TYPE_FLOAT, &light1Position.z, " label='Light 1 Z' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 1 Intensity", TW_TYPE_COLOR3F, &light1Intensity, " label='Light 1 Intensity' group='Light Sources' help='RGB intensity of light 1' ");

	// Light 2 Controls
	TwAddVarRW(bar, "Light 2 Enabled", TW_TYPE_BOOLCPP, &light2Enabled, " label='Enable Light 2' group='Light Sources' help='Enable or disable light 2' ");
	TwAddVarRW(bar, "Light 2 Type", TwDefineEnum("LightType", nullptr, 0), &light2Type, " enum='0 {Directional}, 1 {Point}' label='Light 2 Type' group='Light Sources' ");
	TwAddVarRW(bar, "Light 2 Direction", TW_TYPE_DIR3F, &light2Direction, " label='Light 2 Direction' group='Light Sources' help='Direction of light 2 (if directional)' ");
	TwAddVarRW(bar, "Light 2 Position X", TW_TYPE_FLOAT, &light2Position.x, " label='Light 2 X' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 2 Position Y", TW_TYPE_FLOAT, &light2Position.y, " label='Light 2 Y' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 2 Position Z", TW_TYPE_FLOAT, &light2Position.z, " label='Light 2 Z' group='Light Sources' step=0.1 min=-100.0 max=100.0 ");
	TwAddVarRW(bar, "Light 2 Intensity", TW_TYPE_COLOR3F, &light2Intensity, " label='Light 2 Intensity' group='Light Sources' help='RGB intensity of light 2' ");
	TwDefine(" TweakBar/'Light Sources' opened=false "); // light Attributes folder

	//shading
	//Z Buffer Modes
	const TwEnumVal zBufferModes[] = {
		{ ZBUFFER_MODE_1, "Nonlinear Interpolation (f(z))" },
		{ ZBUFFER_MODE_2, "Invert f(z) Per Pixel" },
		{ ZBUFFER_MODE_3, "Invert f(z) Per Vertex, Linear Interpolation" }
	};
	TwType zBufferModeType = TwDefineEnum("ZBufferModeType", zBufferModes, 3);
	TwAddVarRW(bar, "Z-Buffer Mode", zBufferModeType, &currentZBufferMode, " label='Z-Buffer Mode' group='Shading' " " help='Select the z-buffer mode for depth comparison.' ");

	// Shading Modes
	const TwEnumVal shadingModes[] = {
		{ WIREFRAME, "Wireframe" },
		{ FLAT, "Flat Shading" },
		{ GOURAUD, "Gouraud Shading" },
		{ PHONG, "Phong Shading" }
	};
	TwType shadingModeType = TwDefineEnum("ShadingModeType", shadingModes, 4);
	TwAddVarRW(bar, "Shading Mode", shadingModeType, &currentShadingMode, " label='Shading Mode' group='Shading' " " help='Select the shading mode for rendering.' ");

	//Sided Modes
	const TwEnumVal SidedModes[] = {
		{ ONE_SIDED, "One Sided" },
		{ DOUBLE_SIDED, "Double Sided" },
	};
	TwType sidedModeType = TwDefineEnum("SidedModeType", SidedModes, 2);
	TwAddVarRW(bar, "Sided Mode", sidedModeType, &currentSidedMode, " label='Sided Mode' group='Shading' " " help='Select the sided mode for rendering (One Sided or Double Sided).' ");
	TwDefine(" TweakBar/'Shading' opened=false ");

	//color
	TwAddVarRW(bar, "Color", TW_TYPE_COLOR32, &Color, " label='Color' help='Choose the color of the line.' ");

	// Group for Line Attributes
	TwAddVarRW(bar, "P1.x", TW_TYPE_UINT16, &g_P1x, " min=0 max=1500 step=1 keyIncr=x keyDecr=X group='Line Attributes' help='Point 1 x coordinate' ");
	TwAddVarRW(bar, "P1.y", TW_TYPE_UINT16, &g_P1y, " min=0 max=750 step=1 keyIncr=y keyDecr=Y group='Line Attributes' help='Point 1 y coordinate' ");
	TwAddVarRW(bar, "P2.x", TW_TYPE_UINT16, &g_P2x, " min=0 max=1500 step=1 keyIncr=a keyDecr=A group='Line Attributes' help='Point 2 x coordinate' ");
	TwAddVarRW(bar, "P2.y", TW_TYPE_UINT16, &g_P2y, " min=0 max=750 step=1 keyIncr=b keyDecr=B group='Line Attributes' help='Point 2 y coordinate' ");
	TwDefine(" TweakBar/'Line Attributes' opened=false "); // Line Attributes folder

	// Group for Circle Attributes
	TwAddVarRW(bar, "Cx", TW_TYPE_UINT16, &g_Cx, " min=0 max=1500 step=1 keyIncr=x keyDecr=X group='Circle Attributes' help='Circle center x coordinate' ");
	TwAddVarRW(bar, "Cy", TW_TYPE_UINT16, &g_Cy, " min=0 max=750 step=1 keyIncr=y keyDecr=Y group='Circle Attributes' help='Circle center y coordinate' ");
	TwAddVarRW(bar, "R", TW_TYPE_UINT16, &g_R, " min=1 max=400 step=1 keyIncr=a keyDecr=A group='Circle Attributes' help='Radius of the circle' ");
	TwDefine(" TweakBar/'Circle Attributes' opened=false "); // Circle Attributes folder


	//time widget
	TwAddVarRO(bar, "time (us)", TW_TYPE_UINT32, &ElapsedMicroseconds.LowPart, "help='shows the drawing time in micro seconds'");

	// Call the GLUT main loop
	glutMainLoop();

	return 0;
}


void TW_CALL loadOBJModel(void* data)
{
	std::wstring str = getOpenFileName();

	Camera camera = Camera(frustum_near, frustum_far, frustum_top, frustum_right);
	Material material = Material(ka, kd, ks, n, materialColor);
	Light light1 = Light(light1Type, 1, light1Position, light1Direction, light1Intensity);
	Light light2 = Light(light2Type, light2Enabled, light2Position, light2Direction, light2Intensity);
	Lighting sceneLighting = Lighting(light1, light2, ambientLightIntensity);
	Shading sceneShading = Shading(currentZBufferMode, currentShadingMode, currentSidedMode);

	myMesh = new MeshModel(str, width, height, camera, material, sceneLighting, sceneShading);
}


//do not change this function unless you really know what you are doing!
void initGraphics(int argc, char* argv[])
{
	// Initialize GLUT
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	glutInitWindowSize(960, 640);
	glutCreateWindow("Computer Graphics Skeleton using AntTweakBar and freeGlut");
	glutCreateMenu(NULL);

	// Initialize OpenGL
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_NORMALIZE);
	glDisable(GL_LIGHTING);
	glColor3f(1.0, 0.0, 0.0);

	// Initialize GLEW
	GLenum err = glewInit();
	if (err != GLEW_OK)
	{
		assert(0);
		return;
	}
}



//this is just an example of usage for the Renderer::drawPixels function.
//remove these lines and use your own code for drawing the scene.
//it is also a good idea to move the drawScene() function to another file/class
void drawScene()
{
	if (myMesh) { // Check if a model is loaded
		myMesh->updateProjectMatrix(frustum_near, frustum_far, frustum_top, frustum_right);
		myMesh->updateMaterial(ka,kd,ks,n, materialColor);
		myMesh->updateLighting(
			light1Type, light1Position, light1Direction, light1Intensity,
			light2Type, light2Enabled, light2Position, light2Direction, light2Intensity,
			ambientLightIntensity);
		myMesh->updateShading(currentZBufferMode, currentShadingMode, currentSidedMode);
		auto result = myMesh->ProjectToScreen(normalFactor);
		std::vector<glm::vec2> screenPoints = result[0];
		std::vector<glm::vec2> screenCoordinates = result[1];
		std::vector<glm::vec2> screenNormals = result[2];
		std::vector<glm::vec2> screenBB = result[3];

		if (currentShadingMode == WIREFRAME) {
			myMesh->renderObj(screenPoints);
		}
		else {
			myMesh->rasterize();
		}

		if (showCoords) {
			myMesh->renderCoords(screenCoordinates);
		}
		if (showNormals) {
			myMesh->renderNormals(screenNormals, screenPoints);
		}
		if (showBB) {
			myMesh->renderBB(screenBB);
		}
		
	}
	else {
		switch (currentMode) {
		case DISABLE:
			plotLine(g_P1x, g_P2x, g_P1y, g_P2y);
			break;
		case TRIANGLE:
			plotTriangle();
			break;
		case SQUARE:
			plotSquare();
			break;
		case ROTATED_SQUARE:
			plotRotatedSquare();
			break;
		case STAR:
			plotStar();
			break;
		case BONUS_CIRCLE:
			plotCircle(g_Cx, g_Cy, g_R);
			break;
		}
	}
}

void ApplyTransformations() {
	if (isObjectCoords) {
		// Apply transformations in object coordinates
		myMesh->applyObjectTransformations(spinX, spinY, spinZ, scale, transX, transY, transZ);
	}
	else {
		// Apply transformations in world coordinates
		myMesh->applyWorldTransformations(spinX, spinY, spinZ, scale, transX, transY, transZ);
	}
	myMesh->moveCameraPosition(dcamera_x, dcamera_y, dcamera_z);
	
	glutPostRedisplay(); // Request redraw
}

void LookAt() {
	myMesh->lookAt();
	glutPostRedisplay(); // Request redraw
}


//this will make sure that integer coordinates are mapped exactly to corresponding pixels on screen
void glUseScreenCoordinates(int width, int height)
{
	// Set OpenGL viewport and camera
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, width, 0, height, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}


// Callback function called by GLUT to render screen
void Display()
{

	glClearColor(0, 0, 0, 1); //background color
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//time measuring - don't delete
	QueryPerformanceCounter(&StartingTime);

	drawScene();

	//time measuring - don't delete
	QueryPerformanceCounter(&EndingTime);
	ElapsedMicroseconds.QuadPart = EndingTime.QuadPart - StartingTime.QuadPart;
	ElapsedMicroseconds.QuadPart *= 1000000;
	ElapsedMicroseconds.QuadPart /= Frequency.QuadPart;

	// Draw tweak bars
	TwDraw();

	//swap back and front frame buffers
	glutSwapBuffers();
}


// Callback function called by GLUT when window size changes
void Reshape(int width, int height)
{
	glUseScreenCoordinates(width,height);
	TwWindowSize(width, height);

	// Update the viewport-related transformations in the MeshModel
	if (myMesh) {
		myMesh->updateViewPort(width, height);
	}
	// Request a redisplay to apply changes
	glutPostRedisplay();
}




void MouseButton(int button, int state, int x, int y)
{
	TwEventMouseButtonGLUT(button, state, x, y);
	glutPostRedisplay();
}

void MouseMotion(int x, int y)
{
	TwEventMouseMotionGLUT(x, y);
	glutPostRedisplay();
}

void PassiveMouseMotion(int x, int y)
{
	TwEventMouseMotionGLUT(x, y);
}

void Keyboard(unsigned char k, int x, int y)
{
	TwEventKeyboardGLUT(k, x, y);
	glutPostRedisplay();
}

void Special(int k, int x, int y)
{
	TwEventSpecialGLUT(k, x, y);
	glutPostRedisplay();
}

// Function called at exit
void Terminate(void)
{
	if (myMesh) {
		delete myMesh;
		myMesh = nullptr;
	}

	TwTerminate();
}



/// Callback to enforce frustum constraints
void TW_CALL ValidateFrustumNear(const void* value, void* clientData) {
	frustum_near = *(const float*)value;
	if (frustum_near >= frustum_far) {
		frustum_near = frustum_far - 0.1f; // Ensure near < far
	}
}

void TW_CALL GetFrustumNear(void* value, void* clientData) {
	*(float*)value = frustum_near;
}

void TW_CALL ValidateFrustumFar(const void* value, void* clientData) {
	frustum_far = *(const float*)value;
	if (frustum_near >= frustum_far) {
		frustum_far = frustum_near + 0.1f; // Ensure far > near
	}
}

void TW_CALL GetFrustumFar(void* value, void* clientData) {
	*(float*)value = frustum_far;
}

void UpdateFrustum() {
	float aspectRatio = frustum_right / frustum_top; // Assuming frustum is symmetrical
	float halfFovRad = glm::radians(frustum_fov / 2.0f);
	frustum_top = frustum_near * tan(halfFovRad);
	frustum_right = frustum_top * aspectRatio;
}

// Callback for setting FOV
void TW_CALL SetFrustumFOV(const void* value, void* clientData) {
	frustum_fov = *(const float*)value;
	UpdateFrustum(); // Update right and top based on the new FOV
}

// Callback for getting FOV
void TW_CALL GetFrustumFOV(void* value, void* clientData) {
	*(float*)value = frustum_fov;
}

void TW_CALL updateCameraTranslationX(const void* value, void* clientData) {
	float delta = *(const float*)value; // Value from the tweak bar
	float* cameraX = (float*)clientData;
	*cameraX += delta; // Add the delta to the existing value
}

void TW_CALL updateCameraTranslationY(const void* value, void* clientData) {
	float delta = *(const float*)value;
	float* cameraY = (float*)clientData;
	*cameraY += delta;
}

void TW_CALL updateCameraTranslationZ(const void* value, void* clientData) {
	float delta = *(const float*)value;
	float* cameraZ = (float*)clientData;
	*cameraZ += delta;
}