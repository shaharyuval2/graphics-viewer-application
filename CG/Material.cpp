#include "Material.h"

Material::Material() {
	K = glm::vec3(0.1f, 0.5f, 0.3f);
	color = glm::vec3(0.5f, 0.5f, 0.5f); //grey
	n = 16;
}

Material::Material(float ka, float kd, float ks, float colorR, float colorG, float colorB, float n) {
	K = glm::vec3(ka,kd,ks);
	color = glm::vec3(colorR, colorG, colorB);
	Material::n = n;
}