#pragma once
#ifndef HW1_H
#define HW1_H

#include <vector>
#include <stdint.h>
#include "Renderer.h"

// Enums for drawing modes
enum Mode {
    DISABLE = 0,
    TRIANGLE,
    SQUARE,
    ROTATED_SQUARE,
    STAR,
    BONUS_CIRCLE
};

// Extern declarations for variables
extern uint32_t Color;
extern Mode currentMode;
extern int g_P1x, g_P1y, g_P2x, g_P2y;
extern int g_Cx, g_Cy, g_R;

// Function declarations
void plotLine(int x_start, int x_end, int y_start, int y_end);
void plotLineSteep(int x_start, int x_end, int y_start, int y_end);
void plotLineNotSteep(int x_start, int x_end, int y_start, int y_end);
void plotCircle(int cx, int cy, int r);
void plotTriangle();
void plotSquare();
void plotRotatedSquare();
void plotStar();

#endif // HW1_H