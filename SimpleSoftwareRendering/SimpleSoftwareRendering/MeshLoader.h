#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "Geometry.h"

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

void LoadMeshFromOBJFile(Mesh& meshToFill, std::string filePath, std::string fileName) {

	std::ifstream fileToReadStream(filePath + fileName);

	std::string curLineText = "";

    std::vector<Point> vertices;

    while (std::getline(fileToReadStream, curLineText)) {

        //std::cout << curLineText << std::endl;
        std::vector<std::string> tokens = SplitString(curLineText, ' ');

        if (tokens[0] == "v") {
            Point curVertex = { Vector3{ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) } };
            vertices.push_back(curVertex);
        }
        else if (tokens[0] == "f") {
            Triangle curTriangle;

            curTriangle.a = vertices[std::stoi(tokens[1]) - 1];
            curTriangle.b = vertices[std::stoi(tokens[2]) - 1];
            curTriangle.c = vertices[std::stoi(tokens[3]) - 1];

            meshToFill.triangles.push_back(curTriangle);
        }

		//std::cout << curLineText << std::endl;
	}
    //std::cout << meshToFill.triangles.size() << std::endl;
}
