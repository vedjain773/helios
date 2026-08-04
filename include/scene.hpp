#ifndef SCENE_H
#define SCENE_H

#include "glad/glad.h"
#include "shapes.hpp"
#include <vector>
#include <array>
#include <initializer_list>

class Scene {
  public:
    std::vector<GPUSphere> spheres;
    std::vector<GPUVertex> vertices;
    std::vector<int> indices;
    std::vector<int> triMatIds;

    std::vector<Material> cpuMaterials;
    std::vector<GPUMaterial> materials;
    
    void addSphere(Sphere &sphere);
    void addMaterial(Material &material);
    void addVertices(std::initializer_list<Vertex> vertexList);
    void addIndices(std::initializer_list<int> indexList);
    void addIndices(const std::array<int, 36> &indexList);
    void addTriMatIds(std::initializer_list<int> triMatList);

    void update(int index);
};

Scene buildThreeSpheres();
Scene buildCube();
Scene buildCubeAlt();

std::array<int, 36> getCubeIndices(int base);

#endif
