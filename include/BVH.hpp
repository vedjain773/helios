#ifndef BVH_H
#define BVH_H

#include "shapes.hpp"
#include <array>
#include <vector>
#include <tuple>

struct Triangle {
    std::array<glm::vec3, 3> vertices;
    std::array<int, 3> indices;
    int matId = 0;

    glm::vec3 getCentroid() const; 
};

struct Ray {
    Vec3 src;
    Vec3 dir;
};

struct BVHNode {
    glm::vec3 boundsMin;
    glm::vec3 boundsMax;
    int triangleIndex;
    int triangleCount;
    int childAIndex = 0;
    int childBIndex = 0;
};

class BVHBuilder {
  private:
    std::vector<Triangle> triangles;
    std::vector<BVHNode> nodes;

    static bool compareX(const Triangle &a, const Triangle &b) {
        return a.getCentroid().x < b.getCentroid().x;
    }

    static bool compareY(const Triangle &a, const Triangle &b) {
        return a.getCentroid().y < b.getCentroid().y;
    }
    
    static bool compareZ(const Triangle &a, const Triangle &b) {
        return a.getCentroid().z < b.getCentroid().z;
    }
 
    bool isLeaf(const BVHNode &node);

    std::tuple<glm::vec3, glm::vec3> getBBox(int start, int count);
    std::tuple<glm::vec3, glm::vec3> getBBoxFromNodes(int n1, int n2);
    bool hitsNode(const BVHNode &node, const Ray &ray);

  public:
    BVHBuilder(std::vector<Vertex> &vertexList, std::vector<int> &indexList,
            std::vector<int> &matIds);
    
    void printTriangleIndices();
    void printNode(int index, int depth);
    void buildTree(int nodeIndex, int depth);

    std::vector<int> getIndices();
    std::vector<int> getMatIDs();

    void testIntersection(const Ray &ray);
};

void updateMin(glm::vec3 &min, glm::vec3 &comp);
void updateMax(glm::vec3 &max, glm::vec3 &comp);

#endif
