#pragma once

#include <iostream>

#include "Colour.h"
#include "Geometry.h"

void PrintColour(const Colour& colourToPrint){

	std::cout << "Colour := " << (int)colourToPrint.r << ", " << (int)colourToPrint.g << ", " << (int)colourToPrint.b << ", " << (int)colourToPrint.a << std::endl;

}

void PrintMat4x4(const Mat4x4& matrixToPrint) {

	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			std::cout << matrixToPrint[x][y] << ", ";
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}

void PrintMat4x4Pos0(const Mat4x4& matrixToPrint) {

	for (int x = 0; x < 4; x++)
	{
		for (int y = 0; y < 4; y++)
		{
			if (x != 3)
			{
				std::cout << matrixToPrint[x][y] << ", ";
			}
			else {
				std::cout << 0 << ", ";
			}
		}
		std::cout << std::endl;
	}

	std::cout << std::endl;
}