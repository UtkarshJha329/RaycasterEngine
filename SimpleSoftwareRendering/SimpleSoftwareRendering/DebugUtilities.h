#pragma once

#include <iostream>

#include "Colour.h"

void PrintColour(const Colour& colourToPrint){

	std::cout << "Colour := " << (int)colourToPrint.r << ", " << (int)colourToPrint.g << ", " << (int)colourToPrint.b << ", " << (int)colourToPrint.a << std::endl;

}