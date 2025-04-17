#pragma once

#include "Colour.h"
#include "Geometry.h"

const unsigned int screenWidth = 800;
const unsigned int screenHeight = 600;


// Planes
const float nearPlaneDistance = 0.1f;

const Plane planeNear = { {0.0f, 0.0f, 1.0f}, {{0.0f, 0.0f, nearPlaneDistance}} };    // CAMERA SPACE!!!

const float bias = 0.0f;
const Plane planeBottom = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, -(1.0f + bias) , 0.0f }} };             // NDC SPACE!!!!!!
const Plane planeTop = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, 1.0f + bias, 0.0f }} };
const Plane planeLeft = { { 1.0f, 0.0f, 0.0f }, {{ -(1.0f + bias), 0.0f, 0.0f }} };
const Plane planeRight = { { -1.0f, 0.0f, 0.0f }, {{ 1.0f + bias, 0.0f, 0.0f } } };

const Plane planeBottomScreenSpace = { {0.0f, 1.0f, 0.0f }, {{ 0.0f, 0.0f, 0.0f }} };             // SCREEN SPACE!!!!!!
const Plane planeTopScreenSpace = { { 0.0f, -1.0f, 0.0f }, {{ 0.0f, screenHeight - 1, 0.0f }} };
const Plane planeLeftScreenSpace = { { 1.0f, 0.0f, 0.0f }, {{ 0.0f, 0.0f, 0.0f }} };
const Plane planeRightScreenSpace = { { -1.0f, 0.0f, 0.0f }, {{ screenWidth - 1, 0.0f, 0.0f } } };


// Colours
const Colour colour_white = { 255, 255, 255, 255 };
const Colour colour_black = { 0, 0, 0, 255 };
const Colour colour_red = { 255, 0, 0, 255 };
const Colour colour_green = { 0, 255, 0, 255 };
const Colour colour_blue = { 0, 0, 255, 255 };
const Colour colour_yellow = { 255, 255, 0, 255 };
const Colour colour_pink = { 238, 130, 238, 255 };


// Directions
const Vector3 worldUP = Vector3{ 0.0f, 1.0f, 0.0f };
const Vector3 worldForward = Vector3{ 0.0f, 0.0f, 1.0f };


// UI Collision Grid
const Vector2Int collisionGridCellSize = { 80.0f, 80.0f };
const Vector2Int numGridsOnScreen = { (int)screenWidth / (int)collisionGridCellSize.x, (int)screenHeight / (int)collisionGridCellSize.y };