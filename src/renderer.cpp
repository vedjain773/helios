#include "renderer.hpp"
#include "shapes.hpp"
#include <iostream>

void framebufferSizeCallBack(GLFWwindow *window, int nwidth, int nheight) {
    glViewport(0, 0, nwidth, nheight); 
}

Renderer::Renderer(unsigned int width, unsigned int height, Camera &camera, Scene &scene)
    :width(width), height(height), camera(camera), scene(scene) {}

int Renderer::initGLFW() {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    int nWidth = 1.340f * width;
    int nHeight = 1.033f * height;

    window = glfwCreateWindow(nWidth, nHeight, "Shaders", nullptr, nullptr);
    
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
 
    glViewport(0, 0, nWidth, nHeight);
    
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallBack);

    return 0;
}

void Renderer::initImguiStyles() { 
    ImGuiStyle &style = ImGui::GetStyle();
    style.ScrollbarRounding = 0.0f;
    style.TabRounding = 0.0f;

    style.DockingNodeHasCloseButton = false;
    style.DockingSeparatorSize = 0.0f;

    auto& colors = ImGui::GetStyle().Colors;

    colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.0f);
    colors[ImGuiCol_ChildBg] = ImVec4(0.12f, 0.12f, 0.12f, 1.0f);
    colors[ImGuiCol_FrameBg] = ImVec4(0.16f, 0.16f, 0.16f, 1.0f);

    colors[ImGuiCol_FrameBgHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    colors[ImGuiCol_FrameBgActive] = ImVec4(0.23f, 0.23f, 0.23f, 1.0f);

    colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.0f);
    colors[ImGuiCol_SliderGrabActive] = ImVec4(0.60f, 0.60f, 0.60f, 1.0f);

    colors[ImGuiCol_Tab] = ImVec4(0.13f, 0.13f, 0.13f, 1.0f);
    colors[ImGuiCol_TabHovered] = ImVec4(0.20f, 0.20f, 0.20f, 1.0f);
    colors[ImGuiCol_TabActive] = ImVec4(0.18f, 0.18f, 0.18f, 1.0f);

    colors[ImGuiCol_Text] = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
}

void Renderer::initImgui() {
    std::string glsl_version = "#version 450";  

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable; 

    ImFont* font = io.Fonts->AddFontFromFileTTF(
        "../assets/fonts/inter_reg.ttf",
        13.0f
    );

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version.c_str());

    ImGui::StyleColorsDark();
    initImguiStyles();
}

void Renderer::loadShaders(Shader &shader, ComputeShader &computeShader) {
    this->shader = &shader;
    this->computeShader = &computeShader;
}

void Renderer::createScreenQuad() {
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, 
            GL_FLOAT, NULL);

    glBindImageTexture(0, texture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        
    glGenTextures(1, &accTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, accTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, width, height, 0, GL_RGBA, 
            GL_FLOAT, NULL);

    glBindImageTexture(3, accTexture, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
}

void Renderer::initScene() {
    glGenBuffers(6, buffers); 
    
    auto initBuffer = [this](unsigned int i, int bufId,
            std::size_t sizeInBytes, void const *data) {
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[i]);
        glBufferData(GL_SHADER_STORAGE_BUFFER, sizeInBytes, data, GL_STATIC_DRAW);
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, bufId, buffers[i]);
    };  

    initBuffer(0, 1, scene.spheres.size() * sizeof(GPUSphere), scene.spheres.data());
    initBuffer(1, 2, scene.materials.size() * sizeof(GPUMaterial), scene.materials.data());
    initBuffer(2, 4, scene.vertices.size() * sizeof(GPUVertex), scene.vertices.data());
    initBuffer(3, 5, scene.indices.size() * sizeof(int), scene.indices.data());
    initBuffer(4, 6, scene.triMatIds.size() * sizeof(int), scene.triMatIds.data());
    initBuffer(5, 7, scene.nodes.size() * sizeof(BVHNode), scene.nodes.data());
}

void Renderer::updateScene(int index) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffers[1]);
    glBufferSubData(GL_SHADER_STORAGE_BUFFER, index * sizeof(GPUMaterial),
                sizeof(GPUMaterial), &scene.materials[index]);
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
    initScene();
    createScreenQuad();
    
    int matSize = scene.materials.size();
    int count = 0;
    int numBounces = 5;
    
    bool camWindow = true;
    bool pbrWindow = true;
    bool perfWindow = true;

    ImGuiDockNodeFlags DockSpaceFlags = ImGuiDockNodeFlags_PassthruCentralNode;

    CamConfig tempCfg = {
        {0, 0, 0},
        {0, 1, 0},
        -90.0f,
        0.0f,
        45.0f,
        800.0f,
        600.0f
    }; 

    computeShader->use();
    scene.uploadLight(computeShader->ID);

    while(!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);  
         
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGuiID dockspaceId= ImGui::DockSpaceOverViewport(0, viewport, DockSpaceFlags);

        static bool firstRun = true;
        if (firstRun) {
            firstRun = false;

            ImGui::DockBuilderRemoveNode(dockspaceId);
            ImGui::DockBuilderAddNode(dockspaceId, DockSpaceFlags | ImGuiDockNodeFlags_DockSpace);
            ImGui::DockBuilderSetNodeSize(dockspaceId, viewport->Size);

            ImGuiID dockRightId = ImGui::DockBuilderSplitNode(
                    dockspaceId, ImGuiDir_Right, 0.25f, nullptr, &dockspaceId);

            ImGuiID dockRightTopId = ImGui::DockBuilderSplitNode(
                    dockRightId, ImGuiDir_Up, 0.3f, nullptr, &dockRightId);

            ImGuiID dockBottomRightId = ImGui::DockBuilderSplitNode(
                    dockRightId, ImGuiDir_Down, 0.3f, nullptr, &dockRightId);

            ImGui::DockBuilderDockWindow("Camera", dockRightTopId);
            ImGui::DockBuilderDockWindow("PBR", dockRightId);
            ImGui::DockBuilderDockWindow("Performance", dockBottomRightId);

            ImGui::DockBuilderDockWindow("Viewport", dockspaceId);

            ImGui::DockBuilderFinish(dockspaceId);
        }
          
        camera.update();
        computeShader->use();
        computeShader->setInt("frameCounter", count++);
        camera.updateParams(computeShader->ID);
        glDispatchCompute((width + 15) / 16, (height + 15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::Begin("Viewport");
        ImGui::Image(
            (ImTextureID)(intptr_t)texture,
            ImVec2(width, height),
            ImVec2(0, 1),
            ImVec2(1, 0)
        );
        ImGui::End();
        ImGui::PopStyleVar();

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
           
            if (posUpdate || eulerUpdate) count = 0;

            ImGui::End();
        } 

        if (pbrWindow) {
            ImGui::Begin("PBR", &pbrWindow);
            for (int i = 0; i < matSize; i++) {
                bool metUpdate = false;
                bool rghUpdate = false;
                ImGui::PushID(i);

                metUpdate |= ImGui::SliderFloat("metallic",
                         &scene.cpuMaterials[i].metallic, 0.00f, 1.00f);
                rghUpdate |= ImGui::SliderFloat("roughness", 
                         &scene.cpuMaterials[i].roughness, 0.00f, 1.00f);
                ImGui::Separator();

                if (metUpdate || rghUpdate) {
                    scene.update(i);
                    updateScene(i);
                    count = 0;
                }

                ImGui::PopID();
            }
            ImGui::End();
        }

        if (perfWindow) {
            ImGui::Begin("Performance", &perfWindow);

            bool bounceUpdate = false;
            bounceUpdate |= ImGui::SliderInt("Bounces", &numBounces, 0, 10);
            ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);

            if (bounceUpdate) scene.uploadBounces(computeShader->ID, numBounces); 
            
            ImGui::End();
        }
        
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable) {
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
