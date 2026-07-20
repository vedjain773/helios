#ifndef RENDERER_H
#define RENDERER_H

#include "glad/glad.h"
#include <GLFW/glfw3.h>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "shader.hpp"
#include "compute.hpp"
#include "camera.hpp"

class Renderer {
    private:
        unsigned int width;
        unsigned int height;
        
        unsigned int texture;
        unsigned int quadVAO = 0;
        unsigned int quadVBO;
        
        Shader *shader;
        ComputeShader *computeShader;

        GLFWwindow *window;
        ImGuiIO io;
        
        Camera &camera;

        void createScreenQuad();
        void renderQuad();
    public:
        Renderer(unsigned int width, unsigned int height, Camera &camera);
        ~Renderer();
        
        void loadShaders(Shader &shader, ComputeShader &computeShader);
        int initGLFW();
        void initImgui(); 
        void runRenderLoop();
};

void framebufferSizeCallBack(GLFWwindow *window, int width, int height);

#endif
