#include "BVH.hpp"
#include "shapes.hpp"
#include <format>
#include <iostream>
#include <stack>

#define MAX_DEPTH 10

//---

glm::vec3 vertexToGLM(const Vertex &vertex) {
    return glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
}

//---

glm::vec3 Triangle::getCentroid() const {
    float x = (vertices[0].x + vertices[1].x + vertices[2].x) / 3;
    float y = (vertices[0].y + vertices[1].y + vertices[2].y) / 3; 
    float z = (vertices[0].z + vertices[1].z + vertices[2].z) / 3;

    return glm::vec3(x, y, z);
}

//---

BVHBuilder::BVHBuilder(std::vector<Vertex> &vertexList, std::vector<int> &indexList) {
    unsigned int size = indexList.size() / 3;

    for (int i = 0; i < size; i++) {
        int i1 = indexList[i * 3 + 0];
        int i2 = indexList[i * 3 + 1];
        int i3 = indexList[i * 3 + 2];

        glm::vec3 v1 = vertexToGLM(vertexList[i1]);
        glm::vec3 v2 = vertexToGLM(vertexList[i2]);
        glm::vec3 v3 = vertexToGLM(vertexList[i3]);

        triangles.push_back({{v1, v2, v3}, {i1, i2, i3}});
    }

    BVHNode rootNode = {
        .triangleIndex = 0,
        .triangleCount = size 
    };
    
    nodes.push_back(rootNode);
}

bool BVHBuilder::hitsNode(const BVHNode &node, const Ray &ray) {
    glm::vec3 boundsMin = node.boundsMin;
    glm::vec3 boundsMax = node.boundsMax;

    Vec3 raySrc = ray.src;
    Vec3 rayDir = ray.dir;

    std::array<float, 2> xInter = {
        (raySrc.x - boundsMin.x) /rayDir.x,
        (raySrc.x - boundsMax.x) /rayDir.x
    };

    std::array<float, 2> yInter = {
        (raySrc.y - boundsMin.y) /rayDir.y,
        (raySrc.y - boundsMax.y) /rayDir.y
    };

    std::array<float, 2> zInter = {
        (raySrc.z - boundsMin.z) /rayDir.z,
        (raySrc.z - boundsMax.z) /rayDir.z
    };

    std::sort(xInter.begin(), xInter.end());
    std::sort(yInter.begin(), yInter.end());
    std::sort(zInter.begin(), zInter.end());

    float near = std::max({xInter[0], yInter[0], zInter[0]});
    float far = std::min({xInter[1], yInter[1], zInter[1]});

    return near <= far;
}

bool BVHBuilder::isLeaf(const BVHNode &node) {
    return (node.childAIndex == 0 && node.childBIndex == 0);
}

std::tuple<glm::vec3, glm::vec3> BVHBuilder::getBBox(int start, int count) {
    glm::vec3 boundsMin = triangles[start].vertices[0];
    glm::vec3 boundsMax = boundsMin;

    for (int i = start; i < start + count; i++) {
        for (int j = 0; j < 3; j++) {
            updateMin(boundsMin, triangles[i].vertices[j]);
            updateMax(boundsMax, triangles[i].vertices[j]);
        }
    }

    return std::make_tuple(boundsMin, boundsMax);
}

void BVHBuilder::buildTree() {
    if (depth >= MAX_DEPTH) return;

    int currIndex = nodes.size() - 1;
    BVHNode &currNode = nodes[currIndex];
   
    if (currNode.triangleCount <= 1) {
        auto [boundsMin, boundsMax] = getBBox(currNode.triangleIndex, 1);
        currNode.boundsMin = boundsMin;
        currNode.boundsMax = boundsMax;
        return;
    }; 

    //Split along the longest axis
    auto splitFunc = compareX;
    
    float spanX = std::abs(currNode.boundsMax.x - currNode.boundsMin.x);
    float spanY = std::abs(currNode.boundsMax.y - currNode.boundsMin.y);
    float spanZ = std::abs(currNode.boundsMax.z - currNode.boundsMin.z);

    float maxSpan = std::max({spanX, spanY, spanZ});

    if (maxSpan == spanZ) splitFunc = compareZ;
    else if (maxSpan == spanY) splitFunc = compareY;
    
    int start = currNode.triangleIndex;
    int end = start + currNode.triangleCount;
    int mid = (start + end) / 2;

    std::sort(triangles.begin() + start, triangles.begin() + end, splitFunc);

    //Visit the left Node
    BVHNode leftNode = {
        .triangleIndex = start,
        .triangleCount = mid - start
    };
    
    //currNode becomes invalid after this
    nodes.push_back(leftNode);
    int leftNodeIndex = nodes.size() - 1;

    depth += 1;
    buildTree();
    depth -= 1;

    //Visit the right Node
    BVHNode rightNode = {
        .triangleIndex = mid,
        .triangleCount = end - mid
    };

    nodes.push_back(rightNode);
    int rightNodeIndex = nodes.size() - 1;

    depth += 1;
    buildTree();
    depth -= 1;

    //Set remaining attributes for the current Node
    nodes[currIndex].childAIndex = leftNodeIndex;
    nodes[currIndex].childBIndex = rightNodeIndex; 

    auto [boundsMin, boundsMax] = getBBox(start, end - start);

    nodes[currIndex].boundsMin = boundsMin;
    nodes[currIndex].boundsMax = boundsMax;
}

void BVHBuilder::printTriangleIndices() {
    for (const Triangle &tri: triangles) {
        std::array<int, 3> indices = tri.indices;
        std::cout << std::format("{} {} {}\n", indices[0], indices[1], indices[2]); 
    }
}

void BVHBuilder::printNode(int index, int depth) {
    BVHNode &node = nodes[index];

    if (isLeaf(node)) {
        std::string min = std::format(
                "bmin: {} {} {}\n", node.boundsMin.x, node.boundsMin.y, node.boundsMin.z);

        std::string max = std::format(
                "bmax: {} {} {}\n", node.boundsMax.x, node.boundsMax.y, node.boundsMax.z);

        std::cout << min << max << std::format("Leaf: {}\n", node.triangleIndex);
    } else { 
        std::cout <<
            std::format("{}, {} {}\n", node.triangleCount, node.childAIndex, node.childBIndex);
        
        depth += 1;
        printNode(node.childAIndex, depth);
        depth -= 1;

        depth += 1;
        printNode(node.childBIndex, depth);
        depth -= 1;
    }
}

void BVHBuilder::testIntersection(const Ray &ray) {
    std::stack<BVHNode> nodeStack;

    nodeStack.push(nodes.front());
    while (!nodeStack.empty()) {
        BVHNode currNode = nodeStack.top();
        nodeStack.pop();
        
        if (!hitsNode(currNode, ray)) continue;

        if (isLead(currNode)) {
            Triangle tri = triangles[currNode.triangleIndex];
            
            std::cout << "Hit Triangle ...\n";
            for (glm::vec3 &vec: tri.vertices) {
                std::cout << std::format("{}, {}, {}\n", vec.x, vec.y, vec.z);
            }
            std::cout << "\n";

        } else {
            nodeStack.push(nodes[currNode.childAIndex]);
            nodeStack.push(nodes[currNode.childBIndex]);
        }
    }
}

//---

void updateMin(glm::vec3 &min, glm::vec3 &comp) {
    min.x = std::min(min.x, comp.x);
    min.y = std::min(min.y, comp.y);
    min.z = std::min(min.z, comp.z);
}

void updateMax(glm::vec3 &max, glm::vec3 &comp) {
    max.x = std::max(max.x, comp.x);
    max.y = std::max(max.y, comp.y);
    max.z = std::max(max.z, comp.z);
}
