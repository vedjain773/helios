#ifndef SCENE_H
#define SCENE_H

#include "glad/glad.h"
#include "shapes.hpp"
#include "BVH.hpp"

#include <vector>
#include <array>
#include <initializer_list>

class Scene {
  public:
    std::vector<Vertex> cpuVertices;
    std::vector<BVHNode> nodes;
    std::vector<GPUSphere> spheres;
    std::vector<GPUVertex> vertices;

    std::vector<int> indices;
    std::vector<int> triMatIds;

    std::vector<Material> cpuMaterials;
    std::vector<GPUMaterial> materials;
    
    void addSphere(Sphere &sphere);
    void addMaterial(Material &material);

    void addVertices(std::initializer_list<Vertex> vertexList);
    void addVertices(std::vector<Vertex> &vertexList);

    void addIndices(std::initializer_list<int> indexList);
    void addIndices(const std::array<int, 36> &indexList);
    void addIndices(std::vector<int> &indexList);
    
    void addTriMatIds(std::initializer_list<int> triMatList);
    void addTriMatIds(std::vector<int> &triMatList);

    void buildBVH();

    void update(int index);
};

Scene buildThreeSpheres();
Scene buildCube();
Scene buildCubeAlt();
Scene buildObj();

Mesh createCubeMesh(Scene &scene, const Vec3 &corner, const Vec3 &dim, int matId);
void rotateMesh(Scene &scene, Mesh &mesh, float angle);

std::array<int, 36> getCubeIndices(int base);

#endif
