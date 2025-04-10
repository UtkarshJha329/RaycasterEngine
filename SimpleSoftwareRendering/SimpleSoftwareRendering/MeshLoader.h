#pragma once

#include <string>
#include <sstream>
#include <fstream>
#include <iostream>

#include "Geometry.h"
#include "Model.h"

std::vector<std::string> SplitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}


// checks all material textures of a given type and loads the textures if they're not loaded yet.
// the required info is returned as a Texture struct.
void LoadMaterialTextures(std::vector<Texture>& textures, aiMaterial* mat, aiTextureType type, std::string typeName, std::string& directory, Mesh& meshToLoadTexturesTo)
{
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        //std::cout << directory << str.C_Str() << std::endl;

        Texture curTexture;
        LoadTextureFromTextureNameAndDirectory(str.C_Str(), directory, curTexture);

        //LoadTextureFromFile(str.C_Str(), curTexture);
        meshToLoadTexturesTo.textureIndex = textures.size();
        //std::cout << "LOADED := " << meshToLoadTexturesTo.textureIndex << ", " << textures.size() << std::endl;
        textures.push_back(curTexture);
        //std::cout << "texture data size outside := " << textures[textures.size() - 1].data.size() << std::endl;

    }
}

Mesh ProcessMesh(aiMesh* mesh, const aiScene* scene, std::string& directory)
{
    Mesh meshToPopulateWithData;

    // data to fill
    std::vector<Point> points;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Point curPoint;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        curPoint.position = vector;

        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't 
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            curPoint.texCoord = vec;

            //std::cout << "Cur point := " << curPoint.position.x << ", " << curPoint.position.y << ", " << curPoint.position.z << ", Tex coords := " << curPoint.texCoord.x << ", " << curPoint.texCoord.y << std::endl;
        }
        else {
            std::cout << "No texture coordinates." << std::endl;
            curPoint.texCoord = glm::vec2(0.0f, 0.0f);
        }

        points.push_back(curPoint);
    }

    //std::cout << "Mesh loader debug := " << std::endl;
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        if (face.mNumIndices == 3) {
            Triangle curTriangle = { points[face.mIndices[0]], points[face.mIndices[1]], points[face.mIndices[2]], Colours::white };

            //PrintThisTriangleInfo(curTriangle, i);

            meshToPopulateWithData.triangles.push_back(curTriangle);
        }
        else {
            std::cout << "FACES ARE NOT TRIANGLES!!!!!\n\t-> Number of indicies in face excede 3!!! := " << face.mNumIndices << std::endl;
        }
    }

    // process materials
    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER. 
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    //std::vector<Texture> diffuseMaps;
    LoadMaterialTextures(Model::textures, material, aiTextureType_DIFFUSE, "texture_diffuse", directory, meshToPopulateWithData);
    //textures.insert(textures.end(), textures.begin(), textures.end());

    // return a mesh object created from the extracted mesh data
    return meshToPopulateWithData;
}

// processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
void ProcessNode(aiNode* node, const aiScene* scene, Model& modelToLoadInto)
{
    // process each mesh located at the current node
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        // the node object only contains indices to index the actual objects in the scene. 
        // the scene contains all the data, node is just to keep stuff organized (like relations between nodes).
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        modelToLoadInto.meshes.push_back(ProcessMesh(mesh, scene, modelToLoadInto.directory));
    }
    // after we've processed all of the meshes (if any) we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        ProcessNode(node->mChildren[i], scene, modelToLoadInto);
    }

}

void LoadModel(std::string const& path, Model& modelToLoadInto)
{
    // read file via ASSIMP
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(path, aiProcess_FlipUVs );
    // check for errors
    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) // if is Not Zero
    {
        std::cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << std::endl;
        return;
    }
    // retrieve the directory path of the filepath
    modelToLoadInto.directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    ProcessNode(scene->mRootNode, scene, modelToLoadInto);
}


void LoadMeshFromOBJFile(Mesh& meshToFill, std::string filePath, std::string fileName) {

	std::ifstream fileToReadStream(filePath + fileName);

	std::string curLineText = "";

    std::vector<Vector3> vertices;
    std::vector<Vector2> textureCoordinates;

    while (std::getline(fileToReadStream, curLineText)) {

        //std::cout << curLineText << std::endl;
        std::vector<std::string> tokens = SplitString(curLineText, ' ');

        if (tokens[0] == "v") {
            Vector3 curVertex = Vector3{ std::stof(tokens[1]), std::stof(tokens[2]), std::stof(tokens[3]) };
            vertices.push_back(curVertex);
        }
        else if (tokens[0] == "vt") {
            Vector2 curTextureCoordinate = Vector2{ std::stof(tokens[1]), std::stof(tokens[2]) };
            textureCoordinates.push_back(curTextureCoordinate);
        }
        else if (tokens[0] == "f") {
            Triangle curTriangle;

            int indexA = std::stoi(tokens[1]) - 1; 
            int indexB = std::stoi(tokens[2]) - 1;
            int indexC = std::stoi(tokens[3]) - 1;

            curTriangle.a = Point{ vertices[indexA], textureCoordinates[indexA] };
            curTriangle.b = Point{ vertices[indexB], textureCoordinates[indexB] };
            curTriangle.c = Point{ vertices[indexC], textureCoordinates[indexC] };

            meshToFill.triangles.push_back(curTriangle);
        }

		//std::cout << curLineText << std::endl;
	}
    //std::cout << meshToFill.triangles.size() << std::endl;
}
