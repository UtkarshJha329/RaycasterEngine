#pragma once

const unsigned int NUM_COMPONENTS_IN_PIXEL = 4;
struct Colour {

public:

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

Colour red      = { 255, 0, 0, 255 };
Colour green    = { 0, 255, 0, 255 };
Colour blue     = { 0, 0, 255, 255 };
Colour yellow   = { 255, 255, 0, 255 };