#include "shader.hpp"
#include "compute.hpp"
#include "camera.hpp"
#include "renderer.hpp"
#include <iostream>

int main() {  
    Renderer renderer(800, 600);
    renderer.initGLFW();
    renderer.initImgui();

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");  

    renderer.runRenderLoop();
    return 0;
}
