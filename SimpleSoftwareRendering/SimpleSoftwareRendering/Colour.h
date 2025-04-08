#pragma once

const unsigned int NUM_COMPONENTS_IN_PIXEL = 4;
struct Colour {

public:

    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;
};

class Colours {

public:

    static Colour white;
    static Colour black;
    static Colour red;
    static Colour green;
    static Colour blue;
    static Colour yellow;
    static Colour pink;

};