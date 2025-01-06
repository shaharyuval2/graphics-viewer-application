#include "HW1.h"
#include <cmath>

// Initialize variables
uint32_t Color = 0xffffffff;
Mode currentMode = DISABLE;
int g_P1x = 600, g_P1y = 500;
int g_P2x = 300, g_P2y = 300;
int g_Cx = 600, g_Cy = 350;
int g_R = 100;

void plotCircle(int cx, int cy, int r)
{
    Renderer renderer;
    std::vector<Pixel> pixels;
    pixels.reserve(100000);

    int x = 0, y = -r;
    int d = -r;

    while (x < -y) {
        if (d > 0) {
            y++;
            d += 2 * (x + y) + 1;
        }
        else {
            d += 2 * x + 1;
        }

        pixels.push_back({ cx + x, cy + y, Color });
        pixels.push_back({ cx - x, cy + y, Color });
        pixels.push_back({ cx + x, cy - y, Color });
        pixels.push_back({ cx - x, cy - y, Color });
        pixels.push_back({ cx + y, cy + x, Color });
        pixels.push_back({ cx + y, cy - x, Color });
        pixels.push_back({ cx - y, cy + x, Color });
        pixels.push_back({ cx - y, cy - x, Color });
        x++;
    }
    renderer.drawPixels(pixels);
}

void plotLine(int x_start, int x_end, int y_start, int y_end)
{
    int dx = std::abs(x_end - x_start);
    int dy = std::abs(y_end - y_start);
    if (dx < dy) {
        plotLineSteep(x_start, x_end, y_start, y_end);
    }
    else {
        plotLineNotSteep(x_start, x_end, y_start, y_end);
    }
}

void plotLineSteep(int x_start, int x_end, int y_start, int y_end)
{
    Renderer renderer;
    std::vector<Pixel> pixels;
    pixels.reserve(100000);

    int dx, dy, de, dne;
    int x, y, d;

    if (y_start > y_end) {
        std::swap(x_start, x_end);
        std::swap(y_start, y_end);
    }
    x = x_start;
    y = y_start;
    dx = x_end - x_start;
    dy = y_end - y_start;

    int dir = 1;
    if (dx < 0) {
        dir = -1;
    }
    dx *= dir;

    d = 2 * dx - dy;
    de = 2 * dx;
    dne = 2 * (dx - dy);

    pixels.push_back({ x, y, Color });

    while (y < y_end) {
        if (d < 0) {
            d += de;
        }
        else {
            d += dne;
            x += dir;
        }
        y++;
        pixels.push_back({ x, y, Color });
    }
    renderer.drawPixels(pixels);
}

void plotLineNotSteep(int x_start, int x_end, int y_start, int y_end)
{
    Renderer renderer;
    std::vector<Pixel> pixels;
    pixels.reserve(100000);

    int dx, dy, de, dne;
    int x, y, d;

    if (x_start > x_end) {
        std::swap(x_start, x_end);
        std::swap(y_start, y_end);
    }
    x = x_start;
    y = y_start;
    dx = x_end - x_start;
    dy = y_end - y_start;

    int dir = 1;
    if (dy < 0) {
        dir = -1;
    }
    dy *= dir;

    d = 2 * dy - dx;
    de = 2 * dy;
    dne = 2 * (dy - dx);

    pixels.push_back({ x, y, Color });

    while (x < x_end) {
        if (d < 0) {
            d += de;
        }
        else {
            d += dne;
            y += dir;
        }
        x++;
        pixels.push_back({ x, y, Color });
    }
    renderer.drawPixels(pixels);
}

void plotTriangle() {
    plotLine(600, 817, 600, 225);
    plotLine(600, 383, 600, 225);
    plotLine(817, 383, 225, 225);
}

void plotSquare() {
    plotLine(423, 777, 527, 527);
    plotLine(423, 423, 527, 173);
    plotLine(777, 423, 173, 173);
    plotLine(777, 777, 527, 173);
}

void plotRotatedSquare() {
    plotLine(450, 600, 350, 500);
    plotLine(750, 600, 350, 500);
    plotLine(450, 600, 350, 200);
    plotLine(750, 600, 350, 200);
}

void plotStar() {
    plotLine(600, 640, 535, 404);
    plotLine(776, 640, 407, 404);
    plotLine(776, 664, 407, 329);
    plotLine(709, 664, 200, 329);
    plotLine(709, 600, 200, 283);
    plotLine(491, 600, 200, 283);
    plotLine(491, 536, 200, 329);
    plotLine(424, 536, 407, 329);
    plotLine(424, 561, 407, 404);
    plotLine(600, 561, 535, 404);
}