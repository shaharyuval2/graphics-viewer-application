#include "Camera.h"

Camera::Camera() {
	
}

Camera::Camera(float mnear, float mfar, float top, float right) {
	viewMatrix = glm::mat4(1.0f);
	front = glm::vec3(0, 0, -1);
	position = glm::vec3(0, 0, 0);
	Camera::fov = fov;
	Camera::aspect = aspect;
	nearPlane = mnear;
	farPlane = mfar;

	//calc projection matrix
	const float m00 = mnear / right;
	const float m11 = mnear / top;
	const float m22 = -(mfar + mnear) / (mfar - mnear);
	const float m43 = -(2 * mfar * mnear) / (mfar - mnear);

	projectionMatrix = glm::mat4(
		m00, 0, 0, 0,
		0, m11, 0, 0,
		0, 0, m22, -1,
		0, 0, m43, 0
	);
}

// Set camera position and orientation
void Camera::movePosition(float dcx,float dcy,float dcz) {
	position = position + glm::vec3(dcx, dcy, dcz);
	updateViewMatrix();
}

void Camera::lookAt(const glm::vec4& target) {
	front = glm::normalize(nonhomogenous(target) - position);
	updateViewMatrix();
}

void Camera::updateProjectionMatrix(float mnear, float mfar, float top, float right) {
	//calc projection matrix
	const float m00 = mnear / right;
	const float m11 = mnear / top;
	const float m22 = -(mfar + mnear) / (mfar - mnear);
	const float m43 = -(2 * mfar * mnear) / (mfar - mnear);

	projectionMatrix = glm::mat4(
		m00, 0, 0, 0,
		0, m11, 0, 0,
		0, 0, m22, -1,
		0, 0, m43, 0
	);
}

// Get transformation matrices
glm::mat4 Camera::getViewMatrix() const{
	return viewMatrix;
}
glm::mat4 Camera::getProjectionMatrix() const{
	return projectionMatrix;
}

glm::vec3 Camera::nonhomogenous(glm::vec4 homoV) {
	return glm::vec3(homoV.x / homoV.w, homoV.y / homoV.w, homoV.z / homoV.w);
}

void Camera::updateViewMatrix() {
	//calculate R and U
	glm::vec3 up = glm::vec3(0, 1, 0);
	glm::vec3 right = glm::normalize(glm::cross(front, up));
	up = glm::normalize(glm::cross(right, front));

	glm::mat4 M = glm::mat4(
		right.x, up.x, -front.x, 0.0f,                          // First column
		right.y, up.y, -front.y, 0.0f,                          // Second column
		right.z, up.z, -front.z, 0.0f,                          // Third column
		-glm::dot(right, position), -glm::dot(up, position), glm::dot(front, position), 1.0f                                                  // Fourth column
	);

	viewMatrix = M;
}