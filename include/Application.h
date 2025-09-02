
#ifndef APPLICATION_H
#define APPLICATION_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <memory>

class ClothSystem;
class Renderer;
class Camera;
enum class SimulationMode;

class Application {
private:
    GLFWwindow* window;
    std::unique_ptr<ClothSystem> clothSystem;
    std::unique_ptr<Renderer> renderer;
    std::unique_ptr<Camera> camera;
    
    // application state
    SimulationMode currentMode;
    bool wireframe = false;
    bool showUI = true;
    bool paused = false;
    
    // mouse interaction state
    bool leftMousePressed = false;
    bool rightMousePressed = false;
    glm::vec2 lastMousePos;
    bool firstMouse = true;
    
    // window properties
    int windowWidth = 1920;
    int windowHeight = 1080;
    
    // perf monitoring
    float frameTime = 0.0f;
    int frameCount = 0;
    float fpsTimer = 0.0f;
    float averageFPS = 0.0f;
    
public:
    Application();
    ~Application();
    
    bool initialize();
    void run();
    void shutdown();
    
    // getters
    GLFWwindow* GetWindow() const { return window; }
    
private:
    void update(float deltaTime);
    void render();
    void renderUI();
    void updatePerformanceStats(float deltaTime);
    
    // mouse interaction helpers
    glm::vec3 screenToWorldPos(double screenX, double screenY);
    void handleClothInteraction(double mouseX, double mouseY);
    
    // UI sections
    void renderSimulationControls();
    void renderPhysicsParameters();
    void renderUIOptions();
    void renderPerformanceInfo();
    void renderInstructions();
    
    // static callbacks
    static void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
    static void cursorPosCallback(GLFWwindow* window, double xpos, double ypos);
    static void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);
    static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
    static void framebufferSizeCallback(GLFWwindow* window, int width, int height);
    static void errorCallback(int error, const char* description);
    
    // helpers
    bool initializeGraphics();
    bool initializePhysics();
    bool initializeUI();
    void setupCallbacks();
    void printSystemInfo();
};

#endif
