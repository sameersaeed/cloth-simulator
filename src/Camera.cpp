#include "Camera.h"
#include <algorithm>

Camera::Camera(glm::vec3 pos, glm::vec3 up, float yawAngle, float pitchAngle)
    : position(pos), worldUp(up), yaw(yawAngle), pitch(pitchAngle) {
    orbitalMode = false;
    updateCameraVectors();
}

glm::mat4 Camera::getViewMatrix() const {
    return glm::lookAt(position, position + front, up);
}

glm::mat4 Camera::getProjectionMatrix(float aspectRatio, float nearPlane, float farPlane) const {
    return glm::perspective(glm::radians(fov), aspectRatio, nearPlane, farPlane);
}

void Camera::processKeyboard(CameraMovement direction, float deltaTime) {
    if (orbitalMode) return; // disable keyboard movement in orbital mode
    
    float velocity = movementSpeed * deltaTime;
    
    switch (direction) {
        case CameraMovement::FORWARD:
            position += front * velocity;
            break;
        case CameraMovement::BACKWARD:
            position -= front * velocity;
            break;
        case CameraMovement::LEFT:
            position -= right * velocity;
            break;
        case CameraMovement::RIGHT:
            position += right * velocity;
            break;
        case CameraMovement::UP:
            position += up * velocity;
            break;
        case CameraMovement::DOWN:
            position -= up * velocity;
            break;
    }
}

void Camera::processMouseMovement(float xOffset, float yOffset, bool constrainPitch) {
    xOffset *= mouseSensitivity;
    yOffset *= mouseSensitivity;
    
    yaw += xOffset;
    pitch += yOffset;
    
    if (constrainPitch) {
        pitch = std::clamp(pitch, -89.0f, 89.0f);
    }
    
    if (orbitalMode) {
        updateOrbitalPosition();
    } else {
        updateCameraVectors();
    }
}

void Camera::processMouseScroll(float yOffset) {
    if (orbitalMode) {
        orbitRadius -= yOffset * zoomSensitivity;
        orbitRadius = std::clamp(orbitRadius, 1.0f, 50.0f);
        updateOrbitalPosition();
    } else {
        fov -= yOffset;
        fov = std::clamp(fov, 1.0f, 90.0f);
    }
}

void Camera::updateCameraVectors() {
    // calculate new front vector
    glm::vec3 newFront;
    newFront.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    newFront.y = sin(glm::radians(pitch));
    newFront.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    front = glm::normalize(newFront);
    
    // Re-calculate the right and up vector
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::updateOrbitalPosition() {
    // convert spherical coordinates to Cartesian
    float x = orbitRadius * cos(glm::radians(pitch)) * cos(glm::radians(yaw));
    float y = orbitRadius * sin(glm::radians(pitch));
    float z = orbitRadius * cos(glm::radians(pitch)) * sin(glm::radians(yaw));
    
    position = orbitTarget + glm::vec3(x, y, z);
    
    // look at orbit target
    front = glm::normalize(orbitTarget - position);
    right = glm::normalize(glm::cross(front, worldUp));
    up = glm::normalize(glm::cross(right, front));
}

void Camera::setOrbitalMode(bool orbital) {
    orbitalMode = orbital; 
    if (orbitalMode) {
        orbitRadius = glm::length(position - orbitTarget);
        updateOrbitalPosition(); 
    }
}
