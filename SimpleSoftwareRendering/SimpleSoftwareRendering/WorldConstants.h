#pragma once

#include "Colour.h"
#include "Geometry.h"

const float nearPlaneDistance = 0.1f;
const float screenWidth = 800.0f;
const float screenHeight = 600.0f;

Plane planeNear = { {0.0f, 0.0f, 1.0f}, {{0.0f, 0.0f, nearPlaneDistance}} };    // CAMERA SPACE!!!

float bias = 0.0f;
Plane planeBottom = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, -(1.0f + bias) , 0.0f }} };             // NDC SPACE!!!!!!
Plane planeTop = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, 1.0f + bias, 0.0f }} };
Plane planeLeft = { { 1.0f, 0.0f, 0.0f }, {{ -(1.0f + bias), 0.0f, 0.0f }} };
Plane planeRight = { { -1.0f, 0.0f, 0.0f }, {{ 1.0f + bias, 0.0f, 0.0f } } };

Plane planeBottomScreenSpace = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, 0.0f, 0.0f }} };             // SCREEN SPACE!!!!!!
Plane planeTopScreenSpace = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, screenHeight - 1, 0.0f }} };
Plane planeLeftScreenSpace = { { 1.0f, 0.0f, 0.0f }, {{ 0.0f, 0.0f, 0.0f }} };
Plane planeRightScreenSpace = { { -1.0f, 0.0f, 0.0f }, {{ screenWidth - 1, 0.0f, 0.0f } } };




// Colours
Colour colour_white = { 255, 255, 255, 255 };
Colour colour_black = { 0, 0, 0, 255 };
Colour colour_red = { 255, 0, 0, 255 };
Colour colour_green = { 0, 255, 0, 255 };
Colour colour_blue = { 0, 0, 255, 255 };
Colour colour_yellow = { 255, 255, 0, 255 };
Colour colour_pink = { 238, 130, 238, 255 };