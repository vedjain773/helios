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

    Sphere sp_gl = {{-0.5, 0, -1}, 0.5, 0};
    Sphere sp_r = {{0.5, 0, -1}, 0.5, 1};
    Sphere sp_gr = {{0, -100.5, -1}, 100.0, 2};

    Material mat_gl = {{0.752, 0.752, 0.752}, 0.25, 0.25, 1.33, 1};
    Material mat_r = {{1.0, 0.0, 0.0}, 0.25, 0.25, 0.00, 0};
    Material mat_gr = {{0.0, 1.0, 0.0}, 0.25, 0.25, 0.00, 0};
    
    Scene scene;
    scene.addSphere(sp_gl);
    scene.addSphere(sp_r);
    scene.addSphere(sp_gr);

    scene.addMaterial(mat_gl);
    scene.addMaterial(mat_r);
    scene.addMaterial(mat_gr);
        
    Renderer renderer(800, 600, camera, scene);
    renderer.initGLFW();
    renderer.initImgui(); 

    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    ComputeShader compShader("../src/shaders/compute.glsl");   

    renderer.loadShaders(shader, compShader);
    renderer.runRenderLoop();
    return 0;
}
