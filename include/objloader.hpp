#ifndef OBJLOADER_H
#define OBJLOADER_H

#include <string>
#include <vector>
#include "scene.hpp"
#include "shapes.hpp"

struct ObjData {
    std::vector<Vertex> vertices;
    std::vector<int> indices;
};

class ObjLoader {
    private:
    std::string filePath;

    public:
    ObjLoader(const std::string &filePath);
    ObjData parseObjFile();
    Mesh createObjMesh(Scene &scene, int matId);
};

#endif
