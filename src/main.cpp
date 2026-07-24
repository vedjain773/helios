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

    Sphere sp1 = {{0, 0, -1}, 0.5, 0};
    Sphere sp2 = {{0, -100.5, -1}, 100.0, 1};
    Material mat1 = {{1.0, 0.0, 0.0}, 0.25, 0.25};
    Material mat2 = {{0.0, 1.0, 0.0}, 0.25, 0.25};
    
    Scene scene;
    scene.addSphere(sp1);
    scene.addSphere(sp2);

    scene.addMaterial(mat1);
    scene.addMaterial(mat2);
        
    Renderer renderer(800, 600, camera, scene);
    renderer.initGLFW();
    renderer.initImgui(); 

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");   

    renderer.loadShaders(shader, compShader);
    renderer.runRenderLoop();
    return 0;
}
