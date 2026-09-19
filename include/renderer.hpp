#ifndef RENDERER_H
#define RENDERER_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"

#include "camera.hpp"
#include "compute.hpp"
#include "scene.hpp"
#include "shader.hpp"

class Renderer {
  private:
    unsigned int width;
    unsigned int height;

    unsigned int texture;
    unsigned int accTexture;
    unsigned int quadVAO = 0;
    unsigned int quadVBO;

    unsigned int buffers[6];

    Shader *shader;
    ComputeShader *computeShader;

    GLFWwindow *window;

    Camera &camera;
    Scene &scene;

    void createScreenQuad();
    void renderQuad();
    void initScene();
    void initImguiStyles();
    void updateScene(int index);

  public:
    Renderer(unsigned int width, unsigned int height, Camera &camera, Scene &scene);
    ~Renderer();

    void loadShaders(Shader &shader, ComputeShader &computeShader);
    int initGLFW();
    void initImgui();
    void runRenderLoop();
};

void framebufferSizeCallBack(GLFWwindow *window, int width, int height);

#endif
