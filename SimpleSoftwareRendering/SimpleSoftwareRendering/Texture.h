#pragma once

#include <iostream>

#include <string>
#include <vector>

#include "stb_image.h"
#include "Colour.h"

class Texture {

public:

	int width;
	int height;
	int nrChannels;

	std::vector<unsigned char> data;

	std::string filePath;
};

int texCoordDebugPrintCounter = 0;

Colour GetColourFromTexCoord(Texture& texture, Vector2 texCoord) {

	if (texCoord.x >= 0.0f && texCoord.x <= 1.0f && texCoord.y >= 0.0f && texCoord.y <= 1.0f) {

		Colour returnColour = Colours::pink;

		//std::cout << "Tex coord := " << texCoord.x << ", " << texCoord.y << std::endl;
		Vector2Int magnifiedTexCoord = Vector2{ (texCoord.x * (float)texture.width), (texCoord.y * (float)texture.height) };
		
		//std::cout << texCoord.x << ", " << texCoord.y << " | " << magnifiedTexCoord.x << ", " << magnifiedTexCoord.y << std::endl;


		int r = (magnifiedTexCoord.x + magnifiedTexCoord.y * texture.width) * 4;

		//std::cout << "R := " << r << std::endl;

		if (r >= 0 && r + 3 < texture.data.size()) {
			returnColour = Colour{ texture.data[r + 0], texture.data[r + 1], texture.data[r + 2], texture.data[r + 3] };
		}

		//texCoordDebugPrintCounter++;
		//if (texCoordDebugPrintCounter % 5000 == 0) {
		//	std::cout << "Tex coord := " << texCoord.x << ", " << texCoord.y 
		//				<< "\n\tMagnified Tex Coord := " << magnifiedTexCoord.x << ", " << magnifiedTexCoord.y 
		//				<< "\n\tColour := " << (int)returnColour.r << ", " << (int)returnColour.g << ", " << (int)returnColour.b << ", " << (int)returnColour.a << std::endl;
		//}

		return returnColour;
	}
	else {
		return Colours::black;
	}


}

bool LoadTextureFromFile(std::string filePath, Texture& texture) {

	unsigned char* readData = stbi_load(filePath.c_str(), &texture.width, &texture.height, &texture.nrChannels, 0);

	if (readData) {
		//texture.data.reserve(texture.width * texture.height * texture.nrChannels);
		for (int i = 0; i < texture.height * texture.width * texture.nrChannels; i++)
		{
			texture.data.push_back(readData[i]);
		}

		texture.filePath = filePath;

		stbi_image_free(readData);

		std::cout << "texture data size := " << texture.data.size() << std::endl;

		return true;
	}
	else {
		std::cout << "Failed to load texture!" << std::endl;
		stbi_image_free(readData);
		return false;
	}
}

bool LoadTextureFromTextureNameAndDirectory(std::string fileName, std::string directory, Texture& texture) {

	return LoadTextureFromFile(directory + "/" + fileName, texture);

}