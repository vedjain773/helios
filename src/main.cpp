#include "shader.hpp"
#include "compute.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include "BVH.hpp"
#include <iostream>

int main() {  
    CamConfig cfg = {
        {0, 0, -50},
        {0, 1, 0},
        -90.0f,
        0.0f,
        45.0f,
        800.0f,
        600.0f
    };

    Camera camera(cfg);

    Scene scene = buildCube();

    Renderer renderer(800, 600, camera, scene);
    renderer.initGLFW();
    renderer.initImgui(); 

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");   

    renderer.loadShaders(shader, compShader);
    renderer.runRenderLoop();

    /*Vertex v0 = {{-0.5, -0.5, -0.5}};
    Vertex v1 = {{ 0.5, -0.5, -0.5}};
    Vertex v2 = {{ 0.5,  0.5, -0.5}};
    Vertex v3 = {{-0.5,  0.5, -0.5}};
    Vertex v4 = {{-0.5, -0.5,  0.5}};
    Vertex v5 = {{ 0.5, -0.5,  0.5}};
    Vertex v6 = {{ 0.5,  0.5,  0.5}};
    Vertex v7 = {{-0.5,  0.5,  0.5}};

    int base = 0;

    std::vector<Vertex> vertexList = {v0, v1, v2, v3, v4, v5, v6, v7}; 

    std::vector<int> indices = {
        base + 0, base + 1, base + 2,   base + 0, base + 2, base + 3,
        base + 4, base + 0, base + 3,   base + 4, base + 3, base + 7,
        base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
        base + 4, base + 5, base + 1,   base + 4, base + 1, base + 0,
        base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
    };

    BVHBuilder builder(vertexList, indices);
    std::cout << "Before tree construction...\n";
    builder.printTriangleIndices();

    builder.buildTree(0, 0);

    std::cout << "\nAfter tree construction...\n";
    builder.printNode(0, 0);
    builder.testIntersection({{0.0, 0.0, 2.0}, {0.1, 0.1, -1.0}});*/

    return 0;
}
