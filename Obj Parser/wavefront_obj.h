/////////////////////////////////////////////////////////////////////////////////////////////////////////
// This simple obj parser assumes  that all polygons in the mesh are triangles and will fail otherwise //
/////////////////////////////////////////////////////////////////////////////////////////////////////////
#pragma once

#include <vector>
#include <glm/glm.hpp>

struct Wavefront_obj
{
	struct Face //each face is a triangle
	{
		Face() {v[0] = 0; v[1] = 0; v[2] = 0; t[0] = 0; t[1] = 0; t[2] = 0; n[0] = 0; n[1] = 0; n[2] = 0;}

		int v[3];
		int t[3];
		int n[3];
	};

	struct Vector
	{
		Vector() {e[0] = 0.0; e[1] = 0.0; e[2] = 0.0;}

		double e[3];

		inline double& operator[](int index) { return e[index]; }
		inline const double& operator[](int index) const { return e[index]; }
	};

	std::vector<glm::vec4> m_points;
	std::vector<glm::vec4> m_normals;
	std::vector<Vector> m_textureCoordinates;
	std::vector<Face> m_faces;

	bool load_file(std::wstring filename);

};

