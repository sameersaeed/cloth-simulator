#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum class CameraMovement {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    UP,
    DOWN
};

class Camera {
private:
    glm::vec3 position;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    glm::vec3 worldUp;
    
    float yaw = -90.0f;
    float pitch = 0.0f;
    float fov = 45.0f;
    
    // camera options
    float movementSpeed = 5.0f;
    float mouseSensitivity = 0.1f;
    float zoomSensitivity = 2.0f;
    
    // orbital camera mode
    bool orbitalMode = true;
    float orbitRadius = 8.0f;
    glm::vec3 orbitTarget = glm::vec3(0.0f, 0.0f, 0.0f);
    
public:
    Camera(glm::vec3 position = glm::vec3(0.0f, 2.0f, 15.0f),
           glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f),
           float yaw = -135.0f, float pitch = 12.5f);
    
    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspectRatio, float nearPlane = 0.1f, float farPlane = 100.0f) const;
    
    void processKeyboard(CameraMovement direction, float deltaTime);
    void processMouseMovement(float xOffset, float yOffset, bool constrainPitch = true);
    void processMouseScroll(float yOffset);
    
    // getters
    glm::vec3 getPosition() const { return position; }
    glm::vec3 getFront() const { return front; }
    glm::vec3 getUp() const { return up; }
    glm::vec3 getRight() const { return right; }
    float getFOV() const { return fov; }
    
    // orbital camera controls
    void setOrbitalMode(bool orbital);
    void setOrbitTarget(const glm::vec3& target) { orbitTarget = target; }
    void setOrbitRadius(float radius) { orbitRadius = glm::max(1.0f, radius); }
    bool isOrbitalMode() const { return orbitalMode; }
    
    // camera settings
    void setMovementSpeed(float speed) { movementSpeed = speed; }
    void setMouseSensitivity(float sensitivity) { mouseSensitivity = sensitivity; }
    
private:
    void updateCameraVectors();
    void updateOrbitalPosition();
};

#endif 
