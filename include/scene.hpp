#ifndef SCENE_H
#define SCENE_H

#include "glad/glad.h"
#include "shapes.hpp"
#include <vector>

class Scene {
  public:
    std::vector<GPUSphere> spheres;
    std::vector<GPUVertex> vertices;
    std::vector<int> indices;

    std::vector<Material> cpuMaterials;
    std::vector<GPUMaterial> materials;
    
    void addSphere(Sphere &sphere);
    void addMaterial(Material &material);
    void addVertices(Vertex &vertex);
    void addIndices(int index);

    void update(int index);
};

#endif
