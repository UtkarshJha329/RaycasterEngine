#pragma once

#include <iostream>
#include <string>
#include <vector>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Geometry.h"
#include "Texture.h"

class Mesh {

public:

    std::vector<Triangle> triangles;
    int textureIndex;
};


class Model {

public:

    std::vector<Mesh> meshes;

    std::string directory;
    std::vector<std::string> loadedTexturesFilePaths;	// stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.

    static std::vector<Texture> textures;
};

void PrintThisTriangleInfo(const Triangle& curTriangle, int triangleIndex) {
    std::cout << "\tCur triangle := " << triangleIndex;
    std::cout << "\n\t\t Point A Position : " << curTriangle.a.position.x << ", " << curTriangle.a.position.y << ", " << curTriangle.a.position.z;
    std::cout << "\n\t\t Point A Tex Coords : " << curTriangle.a.texCoord.x << ", " << curTriangle.a.texCoord.y;
    std::cout << std::endl;
    std::cout << "\n\t\t Point B Position : " << curTriangle.b.position.x << ", " << curTriangle.b.position.y << ", " << curTriangle.b.position.z;
    std::cout << "\n\t\t Point B Tex Coords : " << curTriangle.b.texCoord.x << ", " << curTriangle.b.texCoord.y;
    std::cout << std::endl;
    std::cout << "\n\t\t Point C Position : " << curTriangle.c.position.x << ", " << curTriangle.c.position.y << ", " << curTriangle.c.position.z;
    std::cout << "\n\t\t Point C Tex Coords : " << curTriangle.c.texCoord.x << ", " << curTriangle.c.texCoord.y;
    std::cout << std::endl;
}