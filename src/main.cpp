#include "shader.hpp"
#include "compute.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include <iostream>

int main() {  
    CamConfig cfg = {
        {0, 0, 0},
        {0, 1, 0},
        -90.0f,
        0.0f,
        45.0f,
        800.0f,
        600.0f
    };

    Camera camera(cfg);
    Renderer renderer(800, 600, camera);
    renderer.initGLFW();
    renderer.initImgui(); 

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");   

    renderer.loadShaders(shader, compShader);
    renderer.runRenderLoop();
    return 0;
}
