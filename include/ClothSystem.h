#ifndef CLOTH_SYSTEM_H
#define CLOTH_SYSTEM_H

#include <glm/glm.hpp>
#include <vector>
#include <memory>

enum class SimulationMode {
    TEAR,
    COLLISION,
    FLAG
};

struct Particle {
    glm::vec3 position;
    glm::vec3 oldPosition;
    glm::vec3 velocity;
    glm::vec3 force;
    
    float mass = 1.0f;
    bool pinned = false;
    bool active = true;
    
    Particle(const glm::vec3& pos);
};

struct Spring {
    enum SpringType {
        STRUCTURAL,
        SHEAR,
        BEND
    };
    
    int particle1, particle2;
    float restLength;
    float stiffness;
    SpringType type;
    bool active = true;
    
    Spring(int p1, int p2, float length, float k, SpringType t);
};

struct CollisionSphere {
    glm::vec3 center;
    float radius;
    
    CollisionSphere(const glm::vec3& c, float r);
};

class ClothSystem {
private:
    std::vector<Particle> particles;
    std::vector<Spring> springs;
    std::vector<CollisionSphere> spheres;
    
    // physics sim params
    float gravity = -9.81f;
    float damping = 0.99f;
    float windStrength = 0.0f;
    float tearThreshold = 2.0f;
    float elapsedTime = 0.0f;
    const float fixedTimeStep = 1.0f / 60.0f;
    glm::vec3 windDirection = glm::vec3(1.0f, 0.0f, 0.5f);
    
    // grid properties
    int gridWidth, gridHeight;
    float clothWidth, clothHeight;
    
    // object movement for collision mode
    float objectMoveTime = 4.0f;
    float objectMoveSpeed = 0.8f;  
    float objectMoveRange = 8.0f;  
    
    // wind variation for flag mode
    float windVariationTime = 0.0f;
    float windVariationStrength = 0.3f;
    
    // vertex data
    std::vector<float> vertices;
    std::vector<unsigned int> indices;
    
public:
    ClothSystem(int width, int height, float w, float h);
    
    void update(float deltaTime);
    void setMode(SimulationMode mode);
    void handleMouseInteraction(const glm::vec3& mousePos, bool tearing);
    void reset();
    
    // getters (rendering)
    const std::vector<float>& getVertices() const { return vertices; }
    const std::vector<unsigned int>& getIndices() const { return indices; }
    const std::vector<CollisionSphere>& getSpheres() const { return spheres; }
    
    // setters (UI)
    void setGravity(float g) { gravity = g; }
    void setDamping(float d) { damping = d; }
    void setWindStrength(float w) { windStrength = w; }
    void setWindDirection(const glm::vec3& dir) { windDirection = glm::normalize(dir); }
    void setTearThreshold(float t) { tearThreshold = t; }
    
    // getters (UI)
    float getGravity() const { return gravity; }
    float getDamping() const { return damping; }
    float getWindStrength() const { return windStrength; }
    glm::vec3 getWindDirection() const { return windDirection; }
    float getTearThreshold() const { return tearThreshold; }
    
    // collision object manipulation
    void addSphere(const glm::vec3& center, float radius);
    void clearCollisionObjects();
    
    // object movement for collision mode
    void updateObjectMovement(float deltaTime);
    
    // wind variation for flag mode
    void updateWindVariation(float deltaTime);
    
private:
    void createClothGrid();
    void applyForces();
    void satisfyConstraints();
    void handleCollisions();
    void updateVertexData();
    void integrateVerlet(float deltaTime);
    void applyWindForce(Particle& particle);

    bool checkTearing(const Spring& spring);
    
    glm::vec3 calculateNormal(int x, int y) const;
};

#endif 
