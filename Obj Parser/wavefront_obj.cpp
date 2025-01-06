#include <iostream>
#include <fstream>
#include <assert.h>

#include "Wavefront_obj.h"

bool Wavefront_obj::load_file(std::wstring filename)
{
	char line[8192];

	std::ifstream objFile;
	objFile.open(filename.c_str());

	if(!objFile.is_open())
	{
		std::cerr << "Could not open file " << filename.c_str() << std::endl; 
		return false;
	}
	while(objFile.getline(line, 8192))
	{          

		// Vertex position extraction
		if (!strncmp(line, "v ", 2)) {
			glm::vec4 v = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f); // Initialize with default values
			sscanf_s(line + 2, "%f %f %f", &v.x, &v.y, &v.z); // Use %f for float values
			m_points.push_back(v); // Store the extracted vertex
		}

		//texture coordinates
		if (!strncmp(line, "vt ", 2)) {
			Vector v;
			sscanf_s(line+2, "%lf %lf %lf", &v[0], &v[1], &v[2]);
			m_textureCoordinates.push_back(v);	    
		}

		//normal vector
		if (!strncmp(line, "vn ", 2)) {
			glm::vec4 n = glm::vec4(0.0f, 0.0f, 0.0f, 1.0f);
			sscanf_s(line+2, "%lf %lf %lf", &n[0], &n[1], &n[2]);
			m_normals.push_back(n);	    
		}

		//face
		if (!strncmp(line, "f ", 2)) {
			Face f;

			//each vertex of this triangle has position, texture coordinates and normal
			if(sscanf_s(line+2, "%d/%d/%d %d/%d/%d %d/%d/%d", &f.v[0], &f.t[0], &f.n[0], &f.v[1], &f.t[1], &f.n[1], &f.v[2], &f.t[2], &f.n[2]) == 9)
			{

			}
			//each vertex of this triangle only has position (no normals and no texture coordinates)
			else if(sscanf_s(line+2, "%d %d %d", &f.v[0], &f.v[1], &f.v[2]) == 3)
			{

			}
			//each vertex of this triangle has position and normal (but not texture coordinates)
			else if(sscanf_s(line+2, "%d//%d %d//%d %d//%d", &f.v[0], &f.n[0], &f.v[1], &f.n[1], &f.v[2], &f.n[2]) == 6)
			{

			}
			else
			{
				//unsupported mode
				assert(0);
			}

			//subtract 1 since obj file format uses 1-based indexing and C++ uses 0-based indexing.
			//since all indices are initialized to 0, a missing index value will be converted from 0 to -1.
			f.v[0]--;
			f.v[1]--;
			f.v[2]--;

			f.t[0]--;
			f.t[1]--;
			f.t[2]--;

			f.n[0]--;
			f.n[1]--;
			f.n[2]--;

			m_faces.push_back(f);	    
		}	    

	}        	     
	objFile.close();

	//validity check
	int numFaces = m_faces.size();
	int numVertices = m_points.size();
	int numNormals = m_normals.size();
	int numTextureCoordinates = m_textureCoordinates.size();

	assert(numFaces >= 1); //a valid model has at least one face
	assert(numVertices >= 3); //a valid model has at least three vertices

	for(int i = 0; i < numFaces; i++)
	{
		Face& f = m_faces[i];

		assert(f.v[0] >= 0 && f.v[0] < numVertices);
		assert(f.v[1] >= 0 && f.v[1] < numVertices);
		assert(f.v[2] >= 0 && f.v[2] < numVertices);

		if(numNormals > 0)
		{
			assert(f.n[0] >= 0 && f.n[0] < numNormals);
			assert(f.n[1] >= 0 && f.n[1] < numNormals);
			assert(f.n[2] >= 0 && f.n[2] < numNormals);
		}
		else //no normals provided
		{
			assert(f.n[0] == -1);
			assert(f.n[1] == -1);
			assert(f.n[2] == -1);
		}

		if(numTextureCoordinates > 0)
		{
			assert(f.t[0] >= 0 && f.t[0] < numTextureCoordinates);
			assert(f.t[1] >= 0 && f.t[1] < numTextureCoordinates);
			assert(f.t[2] >= 0 && f.t[2] < numTextureCoordinates);
		}
		else //no texture coordinates provided
		{
			assert(f.t[0] == -1);
			assert(f.t[1] == -1);
			assert(f.t[2] == -1);
		}
	}

	return true;
}
