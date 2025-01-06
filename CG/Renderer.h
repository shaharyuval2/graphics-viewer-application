#pragma once

#include <vector>
#include <freeglut/include/GL/freeglut.h>

struct Pixel
{
	int  x, y; //pixel in screen coordinates
	unsigned int color; //RGBA color format - 4 components of 8 bits each - 0xAABBGGRR - AA alpha, BB blue, RR red
};

class Renderer
{
public:
	Renderer();
	~Renderer();

	void drawPixels(const std::vector<Pixel>& pixels);

protected:

};
