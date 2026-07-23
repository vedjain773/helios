#include "renderer.hpp"
#include <iostream>

void framebufferSizeCallBack(GLFWwindow *window, int nwidth, int nheight) {
    glViewport(0, 0, nwidth, nheight); 
}

Renderer::Renderer(unsigned int width, unsigned int height, Camera &camera)
    :width(width), height(height), camera(camera) {}

int Renderer::initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Shaders", nullptr, nullptr);
    
    if (window == nullptr) {
        std::cout << "Failed to create GLFW window\n";
        glfwTerminate();
        return -1;
    }
    
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
 
    glViewport(0, 0, width, height);
    
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);

    return 0;
}

void Renderer::initImgui() {
    std::string glsl_version = "#version 450";  

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    ImGui::StyleColorsDark();
}

void Renderer::loadShaders(Shader &shader, ComputeShader &computeShader) {
    this->shader = &shader;
    this->computeShader = &computeShader;
}

void Renderer::createScreenQuad() {
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, 
            GL_FLOAT, NULL);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);

    computeShader->use();

    glDispatchCompute(width, height, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
}

void Renderer::renderQuad() {
    if (quadVAO == 0) {
        float quadVertices[] = {
            // positions        // texture Coords
            -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
            -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
            1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
            1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
        };

        glGenVertexArrays(1, &quadVAO);
        glGenBuffers(1, &quadVBO);
        
        glBindVertexArray(quadVAO);
        glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float),
                (void*)(3 * sizeof(float)));
    }

    glBindVertexArray(quadVAO);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    glBindVertexArray(0);
}


void Renderer::runRenderLoop() {
    createScreenQuad();
    bool camWindow = true;
    
    CamConfig tempCfg = {
        {0, 0, 0},
        {0, 1, 0},
        -90.0f,
        0.0f,
        45.0f,
        800.0f,
        600.0f
    };

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
         
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
          
        camera.update();
        computeShader->use();
        camera.updateParams(computeShader->ID);
        glDispatchCompute(width, height, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        glActiveTexture(GL_TEXTURE0);
        shader->use();
        shader->setInt("tex", 0);
        glBindTexture(GL_TEXTURE_2D, texture);
        renderQuad();
        
        if (camWindow) {
            bool posUpdate = false;
            bool eulerUpdate = false;

            ImGui::Begin("Camera", &camWindow);

            posUpdate |= ImGui::SliderFloat("x", &tempCfg.position.x, -10.0f, 10.0f);
            posUpdate |= ImGui::SliderFloat("y", &tempCfg.position.y, -10.0f, 10.0f); 
            posUpdate |= ImGui::SliderFloat("z", &tempCfg.position.z, -10.0f, 10.0f); 
            
            eulerUpdate |= ImGui::SliderFloat("yaw", &tempCfg.yaw, -180.0f, 180.0f);
            eulerUpdate |= ImGui::SliderFloat("pitch", &tempCfg.pitch, -89.9f, 89.9f);
    
            if (posUpdate) camera.setCamPos(tempCfg.position);
            if (eulerUpdate) camera.setCamDir(tempCfg.yaw, tempCfg.pitch);

            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
            GLFWwindow* backup_current_context = glfwGetCurrentContext();
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
            glfwMakeContextCurrent(backup_current_context);
        } 
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

Renderer::~Renderer() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
}
