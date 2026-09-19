#include "BVH.hpp"
#include "camera.hpp"
#include "compute.hpp"
#include "renderer.hpp"
#include "shader.hpp"
#include <iostream>

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 600

int main() {
    CamConfig cfg = {{0, 0, 0}, {0, 1, 0}, -90.0f, 0.0f, 45.0f, 800.0f, 600.0f};

    Camera camera(cfg);

    Scene scene = buildCube();

    Renderer renderer(WINDOW_WIDTH, WINDOW_HEIGHT, camera, scene);
    renderer.initGLFW();
    renderer.initImgui();

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");

    renderer.loadShaders(shader, compShader);
    renderer.runRenderLoop();

    return 0;
}
