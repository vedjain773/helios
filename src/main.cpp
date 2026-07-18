#include "glad/glad.h"
#include "shader.hpp"
#include "camera.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>

void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow *window = glfwCreateWindow(800, 600, "Shaders", nullptr, nullptr);
    
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

    std::string glsl_version = "#version 450";  

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    ImGui::StyleColorsDark();

    glViewport(0, 0, 800, 600);
    
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    
    float vertices[] = {
        -0.5f, -0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        -0.5f, 0.5f, 0.0f,
        0.5f, 0.5f, 0.0f,
        0.5f, -0.5f, 0.0f,
    };

    unsigned int VAO, VBO;
    glGenBuffers(1, &VBO);
    glGenVertexArrays(1, &VAO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
    glEnableVertexAttribArray(0);
    
    Shader shader("../src/shaders/vertex.glsl", "../src/shaders/fragment.glsl");
    shader.use(); 
    
    CamConfig config = {
        {0.0f, 0.0f, 10.0f},
        {0.0f, 1.0f, 0.0f},
        -90.0f, 0.0f,
        45.0f, 0.1f, 100.0f
    };

    Camera camera(config);

    glm::mat4 model = glm::mat4(1.0f); 
    
    camera.setView(shader);
    camera.setProj(shader);

    Vec3 cam_pos = {0.0, 0.0, 10.0};
    
    float fov = 45.0f;
    float near = 0.1f;
    float far = 100.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    bool cam_window = true;
    
    model = glm::rotate(model, glm::radians(90.0f),
            glm::vec3(1.0f, 0.0f, 0.0f));
    glUniformMatrix4fv(glGetUniformLocation(shader.ID, "model"), 1, GL_FALSE,
            glm::value_ptr(model)); 

    while(!glfwWindowShouldClose(window)) {
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);  
         
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, 6); 

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    
        if (cam_window)
        {  
            bool camChanged = false;
            bool cfgChanged = false;
            bool angChanged = false;

            ImGui::Begin("Camera", &cam_window);
            camChanged |= ImGui::SliderFloat("x", &cam_pos.x, 0.0f, 10.0f);
            camChanged |= ImGui::SliderFloat("y", &cam_pos.y, 0.0f, 10.0f);
            camChanged |= ImGui::SliderFloat("z", &cam_pos.z, 3.0f, 10.0f);

            cfgChanged |= ImGui::SliderFloat("fov", &fov, 45.0f, 90.0f);
            cfgChanged |= ImGui::SliderFloat("near", &near, 0.1f, 100.0f);
            cfgChanged |= ImGui::SliderFloat("far", &far, 0.1f, 100.0f);

            angChanged |= ImGui::SliderFloat("yaw", &yaw, -180.0f, 180.0f);
            angChanged |= ImGui::SliderFloat("pitch", &pitch, -90.0f, 90.0f);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 
                    1000.0f / io.Framerate, io.Framerate);
            
            if (camChanged) camera.setCamPos(cam_pos);
            if (cfgChanged) camera.setProjCfg(fov, near, far);
            if (angChanged) camera.setCamDir(yaw, pitch);

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
        
        camera.update();
        camera.setView(shader);
        camera.setProj(shader);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
   
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwTerminate();
    return 0;
}
