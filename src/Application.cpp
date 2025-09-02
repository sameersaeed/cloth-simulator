#include "Application.h"
#include "ClothSystem.h"
#include "Renderer.h"
#include "Camera.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>

#include <iostream>
#include <array>

Application::Application() 
    : window(nullptr), currentMode(SimulationMode::TEAR) {}

Application::~Application() {
    shutdown();
}

bool Application::initialize() {
    glfwSetErrorCallback(errorCallback);
    
    // GLFW initializaiton
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return false;
    }
    
    // configure GLFW
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_SAMPLES, 4); 
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);
    
    // create window
    window = glfwCreateWindow(windowWidth, windowHeight, "Cloth Simulator", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return false;
    }
    
    glfwMakeContextCurrent(window);
    glfwSetWindowUserPointer(window, this);
    
    // GLEW initialization
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW\n";
        return false;
    }
    
    printSystemInfo();
    
    if (!initializeGraphics() || !initializePhysics() || !initializeUI()) {
        return false;
    }
    
    setupCallbacks();
    
    return true;
}

bool Application::initializeGraphics() {
    // enable OpenGL features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_MULTISAMPLE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glFrontFace(GL_CCW);
    
    // renderer initialization
    renderer = std::make_unique<Renderer>();
    if (!renderer->initialize()) {
        std::cerr << "Failed to initialize renderer\n";
        return false;
    }

    // camera initialization
    camera = std::make_unique<Camera>();
    camera->setOrbitalMode(true);

    return true;
}

bool Application::initializePhysics() {
    // cloth system initialization
    clothSystem = std::make_unique<ClothSystem>(25, 25, 4.0f, 4.0f);
    clothSystem->setMode(currentMode);
    
    return true;
}

bool Application::initializeUI() {
    // ImGui initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    
    // setup ImGui styling
    ImGui::StyleColorsDark();
    
    // platform / renderer backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window, false)) {  
        std::cerr << "Failed to initialize ImGui GLFW backend\n";
        return false;
    }
    
    if (!ImGui_ImplOpenGL3_Init("#version 460")) {
        std::cerr << "Failed to initialize ImGui OpenGL backend\n";
        return false;
    }
    
    return true;
}

void Application::setupCallbacks() {
    glfwSetMouseButtonCallback(window, mouseButtonCallback);
    glfwSetCursorPosCallback(window, cursorPosCallback);
    glfwSetScrollCallback(window, scrollCallback);
    glfwSetKeyCallback(window, keyCallback);
    glfwSetFramebufferSizeCallback(window, framebufferSizeCallback);
}

void Application::printSystemInfo() {
    std::cout << "OpenGL Version: " << glGetString(GL_VERSION) << '\n';
    std::cout << "GLSL Version: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';
    std::cout << "Renderer: " << glGetString(GL_RENDERER) << '\n';
    std::cout << "Vendor: " << glGetString(GL_VENDOR) << '\n';
}

void Application::run() {
    float lastFrame = 0.0f;
    
    while (!glfwWindowShouldClose(window)) {
        float currentFrame = static_cast<float>(glfwGetTime());
        float deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        
        updatePerformanceStats(deltaTime);
        
        if (!paused) {
            update(deltaTime);
        }
        
        render();
        
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void Application::update(float deltaTime) {
    deltaTime = std::min(deltaTime, 0.016f); // max 60 FPS
    
    clothSystem->update(deltaTime);
}

void Application::render() {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // update viewport
    int width, height;
    glfwGetFramebufferSize(window, &width, &height);
    glViewport(0, 0, width, height);
    
    renderer->createScene(*clothSystem, *camera, wireframe);
    
    if (showUI) {
        renderUI();
    }
}

void Application::renderUI() {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
    
    renderSimulationControls();
    renderPhysicsParameters();
    renderUIOptions();
    renderPerformanceInfo();
    renderInstructions();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void Application::renderSimulationControls() {
    ImGui::Begin("Simulation Controls");
    
    // mode selection
    const char* modeNames[] = { "Tear Mode", "Collision Mode", "Flag Mode" };
    int currentModeInt = static_cast<int>(currentMode);
    if (ImGui::Combo("Simulation Mode", &currentModeInt, modeNames, 3)) {
        currentMode = static_cast<SimulationMode>(currentModeInt);
        clothSystem->setMode(currentMode);
    }
    
    ImGui::Separator();
    
    if (ImGui::Button("Reset Simulation")) {
        clothSystem->reset();
    }
    
    ImGui::SameLine();
    if (ImGui::Button(paused ? "Resume" : "Pause")) {
        paused = !paused;
    }
    
    ImGui::End();
}

void Application::renderPhysicsParameters() {
    ImGui::Begin("Physics Parameters");
    
    float gravity = clothSystem->getGravity();
    if (ImGui::SliderFloat("Gravity", &gravity, -20.0f, 0.0f)) {
        clothSystem->setGravity(gravity);
    }
    
    float damping = clothSystem->getDamping();
    if (ImGui::SliderFloat("Damping", &damping, 0.9f, 1.0f)) {
        clothSystem->setDamping(damping);
    }
    
    if (currentMode == SimulationMode::FLAG) {
        float windStrength = clothSystem->getWindStrength();
        if (ImGui::SliderFloat("Wind Strength", &windStrength, 0.0f, 15.0f)) {
            clothSystem->setWindStrength(windStrength);
        }
        
        glm::vec3 windDir = clothSystem->getWindDirection();
        float windDirArray[3] = { windDir.x, windDir.y, windDir.z };
        if (ImGui::SliderFloat3("Wind Direction", windDirArray, -1.0f, 1.0f)) {
            clothSystem->setWindDirection(glm::vec3(windDirArray[0], windDirArray[1], windDirArray[2]));
        }
    }
    
    if (currentMode == SimulationMode::TEAR) {
        float tearThreshold = clothSystem->getTearThreshold();
        if (ImGui::SliderFloat("Tear Threshold", &tearThreshold, 1.5f, 5.0f)) {
            clothSystem->setTearThreshold(tearThreshold);
        }
    }
    
    ImGui::End();
}

void Application::renderUIOptions() {
    ImGui::Begin("Rendering");
    
    ImGui::Checkbox("Wireframe", &wireframe);
    
    bool orbitalMode = camera->isOrbitalMode();
    if (ImGui::Checkbox("Orbital Camera", &orbitalMode)) {
        camera->setOrbitalMode(orbitalMode);
    }
    
    ImGui::End();
}

void Application::renderPerformanceInfo() {
    ImGui::Begin("Performance");
    
    ImGui::Text("FPS: %.1f", averageFPS);
    ImGui::Text("Frame Time: %.3f ms", frameTime * 1000.0f);
    ImGui::Text("Particles: %zu", clothSystem->getVertices().size() / 8); // 8 floats per vertex
    ImGui::Text("Triangles: %zu", clothSystem->getIndices().size() / 3);
    
    ImGui::End();
}

void Application::renderInstructions() {
    ImGui::Begin("Instructions");
    
    ImGui::Text("Keyboard Controls:");
    ImGui::BulletText("1/2/3 - Switch simulation modes");
    ImGui::BulletText("Tab - Toggle wireframe");
    ImGui::BulletText("R - Reset simulation");
    ImGui::BulletText("F1 - Toggle UI");
    ImGui::BulletText("Space - Pause/Resume");
    ImGui::BulletText("ESC - Exit");
    
    ImGui::Separator();
    ImGui::Text("Mouse Controls:");
    ImGui::BulletText("Right Mouse + Drag - Orbit camera");
    ImGui::BulletText("Mouse Wheel - Zoom in/out");
    
    if (currentMode == SimulationMode::TEAR) {
        ImGui::BulletText("Left Click - Tear cloth");
    }
    
    ImGui::End();
}

void Application::updatePerformanceStats(float deltaTime) {
    frameTime = deltaTime;
    frameCount++;
    fpsTimer += deltaTime;
    
    if (fpsTimer >= 1.0f) {
        averageFPS = frameCount / fpsTimer;
        frameCount = 0;
        fpsTimer = 0.0f;
    }
}

glm::vec3 Application::screenToWorldPos(double screenX, double screenY) {
    // convert screen coords to normalized device coords
    float x = (2.0f * screenX) / windowWidth - 1.0f;
    float y = 1.0f - (2.0f * screenY) / windowHeight;
    
    // simplified raycasting conversion
    glm::vec4 rayClip = glm::vec4(x, y, -1.0f, 1.0f);
    
    glm::mat4 projection = camera->getProjectionMatrix(windowWidth / float(windowHeight));
    glm::mat4 view = camera->getViewMatrix();
    
    glm::mat4 invProjection = glm::inverse(projection);
    glm::mat4 invView = glm::inverse(view);
    
    glm::vec4 rayEye = invProjection * rayClip;
    rayEye = glm::vec4(rayEye.x, rayEye.y, -1.0f, 0.0f);
    
    glm::vec4 rayWorld = invView * rayEye;
    glm::vec3 rayDir = glm::normalize(glm::vec3(rayWorld));
    
    // cloth plane intersection
    float t = -camera->getPosition().z / rayDir.z;
    return camera->getPosition() + rayDir * t;
}

void Application::handleClothInteraction(double mouseX, double mouseY) {
    if (currentMode == SimulationMode::TEAR) {
        glm::vec3 worldPos = screenToWorldPos(mouseX, mouseY);
        clothSystem->handleMouseInteraction(worldPos, true);
    }
}

void Application::shutdown() {
    // ImGui cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    
    renderer.reset();
    clothSystem.reset();
    camera.reset();
    
    // GLFW cleanup
    if (window) {
        glfwDestroyWindow(window);
    }
    glfwTerminate();
}

// static callback
void Application::mouseButtonCallback(GLFWwindow* window, int button, int action, int /*mods*/) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    
    // pass to ImGui first
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            app->leftMousePressed = true;
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            app->handleClothInteraction(xpos, ypos);
        } else if (action == GLFW_RELEASE) {
            app->leftMousePressed = false;
        }
    }
    
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        app->rightMousePressed = (action == GLFW_PRESS);
    }
}

void Application::cursorPosCallback(GLFWwindow* window, double xpos, double ypos) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    
    if (app->firstMouse) {
        app->lastMousePos = glm::vec2(xpos, ypos);
        app->firstMouse = false;
    }
    
    glm::vec2 currentPos(xpos, ypos);
    glm::vec2 delta = currentPos - app->lastMousePos;
    app->lastMousePos = currentPos;
    
    if (app->rightMousePressed) {
        app->camera->processMouseMovement(delta.x, -delta.y);
    }
    
    // continuous tearing when dragging in tear mode
    if (app->leftMousePressed && app->currentMode == SimulationMode::TEAR) {
        app->handleClothInteraction(xpos, ypos);
    }
}

void Application::scrollCallback(GLFWwindow* window, double /*xoffset*/, double yoffset) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    
    // pass to ImGui first
    ImGuiIO& io = ImGui::GetIO();
    if (io.WantCaptureMouse) return;
    
    app->camera->processMouseScroll(static_cast<float>(yoffset));
}

void Application::keyCallback(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));

    if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                glfwSetWindowShouldClose(window, true);
                break;
            case GLFW_KEY_F1:
                app->showUI = !app->showUI;
                break;
            case GLFW_KEY_TAB:
                app->wireframe = !app->wireframe;
                break;
            case GLFW_KEY_1:
                app->currentMode = SimulationMode::TEAR;
                app->clothSystem->setMode(app->currentMode);
                break;
            case GLFW_KEY_2:
                app->currentMode = SimulationMode::COLLISION;
                app->clothSystem->setMode(app->currentMode);
                break;
            case GLFW_KEY_3:
                app->currentMode = SimulationMode::FLAG;
                app->clothSystem->setMode(app->currentMode);
                break;
            case GLFW_KEY_R:
                app->clothSystem->reset();
                break;
            case GLFW_KEY_SPACE:
                app->paused = !app->paused;
                break;
            case GLFW_KEY_C:
                app->camera->setOrbitalMode(!app->camera->isOrbitalMode());
                break;
            case GLFW_KEY_W:
            case GLFW_KEY_S:
            case GLFW_KEY_A:
            case GLFW_KEY_D:
            case GLFW_KEY_Q:
            case GLFW_KEY_E:
                break;
        }
    }
}

void Application::framebufferSizeCallback(GLFWwindow* window, int width, int height) {
    Application* app = static_cast<Application*>(glfwGetWindowUserPointer(window));
    app->windowWidth = width;
    app->windowHeight = height;
    glViewport(0, 0, width, height);
}

void Application::errorCallback(int error, const char* description) {
    std::cerr << "GLFW Error " << error << ": " << description << '\n';
}
