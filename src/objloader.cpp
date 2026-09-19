#include "objloader.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

ObjLoader::ObjLoader(const std::string &filePath)
    : filePath(filePath) {}

ObjData ObjLoader::parseObjFile() {
    ObjData result;
    std::ifstream file(filePath);

    if (!file.is_open()) {
        throw std::runtime_error("Failed to open OBJ file: " + filePath);
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream ss(line);
        std::string prefix;
        ss >> prefix;

        if (prefix == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            result.vertices.push_back({{x, y, z}});
        } else if (prefix == "f") {
            std::vector<int> faceIndices;
            std::string token;

            while (ss >> token) {
                size_t slash = token.find('/');

                if (slash != std::string::npos)
                    token = token.substr(0, slash);

                int vIndex = std::stoi(token);

                if (vIndex < 0)
                    vIndex = int(result.vertices.size()) + vIndex + 1;

                faceIndices.push_back(vIndex - 1);
            }

            for (size_t i = 1; i + 1 < faceIndices.size(); i++) {
                result.indices.push_back(faceIndices[0]);
                result.indices.push_back(faceIndices[i]);
                result.indices.push_back(faceIndices[i + 1]);
            }
        }
    }

    file.close();
    return result;
}

Mesh ObjLoader::createObjMesh(Scene &scene, int matId) {
    ObjData objData = parseObjFile();
    std::cout << "Vertices: " << objData.vertices.size() << " Indices: " << objData.indices.size()
              << "\n";

    int vertexOffset = scene.vertices.size();
    int indexOffset = scene.indices.size();

    scene.addVertices(objData.vertices);

    std::vector<int> offsetIndices;
    offsetIndices.reserve(objData.indices.size());

    for (int idx : objData.indices)
        offsetIndices.push_back(idx + vertexOffset);

    scene.addIndices(offsetIndices);

    int indexCount = scene.indices.size() - indexOffset;
    int vertexCount = scene.vertices.size() - vertexOffset;

    std::vector<int> triMatList(indexCount / 3, matId);
    scene.addTriMatIds(triMatList);

    return Mesh{vertexOffset, vertexCount, indexOffset, indexCount, matId};
}
