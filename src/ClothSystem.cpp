#include "ClothSystem.h"

#include <random>
#include <algorithm>
#include <cmath>
#include <limits>

Particle::Particle(const glm::vec3& pos) 
    : position(pos), oldPosition(pos), velocity(0.0f), force(0.0f) {}

Spring::Spring(int p1, int p2, float length, float k, SpringType t)
    : particle1(p1), particle2(p2), restLength(length), stiffness(k), type(t) {}

CollisionSphere::CollisionSphere(const glm::vec3& c, float r) : center(c), radius(r) {}

ClothSystem::ClothSystem(int width, int height, float w, float h)
    : gridWidth(width), gridHeight(height), clothWidth(w), clothHeight(h) {
    createClothGrid();
}

void ClothSystem::createClothGrid() {
    particles.clear();
    springs.clear();
    
    // create particles in a grid
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            float px = (x / float(gridWidth - 1)) * clothWidth - clothWidth * 0.5f;
            float py = (y / float(gridHeight - 1)) * clothHeight;
            float pz = 0.0f;
            
            particles.emplace_back(glm::vec3(px, py, pz));
            
            // basic cloth behavior - pin top row
            if (y == gridHeight - 1) {
                particles.back().pinned = true;
            }
        }
    }
    
    // create springs with different types and stiffness values
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int current = y * gridWidth + x;
            
            // structural springs
            if (x < gridWidth - 1) {
                int right = y * gridWidth + (x + 1);
                float length = glm::length(particles[right].position - particles[current].position);
                springs.emplace_back(current, right, length, 0.7f, Spring::STRUCTURAL);  
            }
            if (y < gridHeight - 1) {
                int below = (y + 1) * gridWidth + x;
                float length = glm::length(particles[below].position - particles[current].position);
                springs.emplace_back(current, below, length, 0.7f, Spring::STRUCTURAL);  
            }
            
            // shear springs (diagonals)
            if (x < gridWidth - 1 && y < gridHeight - 1) {
                int diagRight = (y + 1) * gridWidth + (x + 1);
                float length = glm::length(particles[diagRight].position - particles[current].position);
                springs.emplace_back(current, diagRight, length, 0.3f, Spring::SHEAR);  
            }
            if (x > 0 && y < gridHeight - 1) {
                int diagLeft = (y + 1) * gridWidth + (x - 1);
                float length = glm::length(particles[diagLeft].position - particles[current].position);
                springs.emplace_back(current, diagLeft, length, 0.3f, Spring::SHEAR);  
            }
            
            // bend springs 
            if (x < gridWidth - 2) {
                int farRight = y * gridWidth + (x + 2);
                float length = glm::length(particles[farRight].position - particles[current].position);
                springs.emplace_back(current, farRight, length, 0.15f, Spring::BEND);  
            }
            if (y < gridHeight - 2) {
                int farBelow = (y + 2) * gridWidth + x;
                float length = glm::length(particles[farBelow].position - particles[current].position);
                springs.emplace_back(current, farBelow, length, 0.15f, Spring::BEND);  
            }
        }
    }
    
    updateVertexData();
}

void ClothSystem::update(float deltaTime) {
    elapsedTime += deltaTime;                       
    while (elapsedTime >= fixedTimeStep) {
        applyForces();
        integrateVerlet(fixedTimeStep);
        
        // stabilize with multiple constraint satisfactions
        for (int i = 0; i < 3; ++i) {
            satisfyConstraints();
        }
        
        handleCollisions();
        elapsedTime -= fixedTimeStep;
    }
    
    updateObjectMovement(deltaTime);
    updateWindVariation(deltaTime);
    
    updateVertexData();
}

void ClothSystem::applyForces() {
    for (auto& particle : particles) {
        if (!particle.active || particle.pinned) continue;
        
        particle.force = glm::vec3(0.0f);               // reset forces
        particle.force.y += gravity * particle.mass;    // gravity
        
        if (windStrength > 0.0f) {                      // wind force
            applyWindForce(particle);
        }
    }
}

void ClothSystem::applyWindForce(Particle& particle) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_real_distribution<float> turbulence(-1.0f, 1.0f);
    
    // base wind force
    glm::vec3 wind = windDirection * windStrength;
    
    // add turbulence for more wind realism
    glm::vec3 turbulenceVec(
        turbulence(gen) * 0.3f,
        turbulence(gen) * 0.2f,
        turbulence(gen) * 0.3f
    );
    wind += turbulenceVec * windStrength;
    
    // wind resistance based on velocity
    glm::vec3 relativeVelocity = wind - particle.velocity;
    float velocityMagnitude = glm::length(relativeVelocity);
    
    if (velocityMagnitude > 0.0f) {
        glm::vec3 normalizedVelocity = relativeVelocity / velocityMagnitude;
        float dragCoefficient = 0.1f;
        glm::vec3 windForce = normalizedVelocity * velocityMagnitude * velocityMagnitude * dragCoefficient;
        particle.force += windForce * particle.mass;
    }
}

void ClothSystem::integrateVerlet(float deltaTime) {
    for (auto& particle : particles) {
        if (particle.pinned || !particle.active) continue;
        
        glm::vec3 acceleration = particle.force / particle.mass;
        glm::vec3 newPosition = particle.position + 
                               (particle.position - particle.oldPosition) * damping + 
                               acceleration * deltaTime * deltaTime;
        
        particle.oldPosition = particle.position;
        particle.position = newPosition;
    }
}

void ClothSystem::satisfyConstraints() {
    for (auto& spring : springs) {
        if (!spring.active) continue;
        
        Particle& p1 = particles[spring.particle1];
        Particle& p2 = particles[spring.particle2];
        
        if (!p1.active || !p2.active) continue;
        
        glm::vec3 delta = p2.position - p1.position;
        float distance = glm::length(delta);
        
        if (distance < 1e-6f) continue; 
        
        if (checkTearing(spring)) {
            spring.active = false;
            continue;
        }
        
        float difference = (spring.restLength - distance) / distance;
        glm::vec3 translate = delta * difference * spring.stiffness;
        
        // apply correction based on mass ratio
        float totalMass = p1.mass + p2.mass;
        float ratio1 = p2.mass / totalMass;
        float ratio2 = p1.mass / totalMass;
        
        if (!p1.pinned) p1.position -= translate * ratio1;
        if (!p2.pinned) p2.position += translate * ratio2;
    }
}

bool ClothSystem::checkTearing(const Spring& spring) {
    const Particle& p1 = particles[spring.particle1];
    const Particle& p2 = particles[spring.particle2];
    
    float distance = glm::length(p2.position - p1.position);
    return distance > spring.restLength * tearThreshold;
}

void ClothSystem::handleCollisions() {
    for (auto& particle : particles) {
        if (!particle.active) continue;
        
        // sphere collisions
        for (const auto& sphere : spheres) {
            glm::vec3 diff = particle.position - sphere.center;
            float distance = glm::length(diff);
            
            if (distance < sphere.radius) {
                glm::vec3 normal = (distance > 1e-6f) ? diff / distance : glm::vec3(0.0f, 1.0f, 0.0f);
                particle.position = sphere.center + normal * sphere.radius;
                glm::vec3 velocity = particle.position - particle.oldPosition;

                float vn = glm::dot(velocity, normal);
                glm::vec3 vNormal = vn * normal;
                glm::vec3 vTangent = velocity - vNormal;

                float bounce = 0.2f;        
                float friction = 0.9f;      
                glm::vec3 newVelocity = vTangent * friction - vNormal * bounce;

                particle.oldPosition = particle.position - newVelocity;
            }
        }
        
        // bounce for ground collision w/ ground plane
        if (particle.position.y < -5.0f) {
            particle.position.y = -5.0f;
            glm::vec3 velocity = particle.position - particle.oldPosition;
            particle.oldPosition = particle.position - velocity * 0.4f; 
        }
    }
}

void ClothSystem::updateVertexData() {
    vertices.clear();
    indices.clear();
    
    // map from grid pos to vertex index for active particles
    std::vector<int> gridToVertex(gridWidth * gridHeight, -1);
    int vertexCount = 0;
    
    // vertices with normals and texture coords
    for (int y = 0; y < gridHeight; ++y) {
        for (int x = 0; x < gridWidth; ++x) {
            int gridIndex = y * gridWidth + x;
            const Particle& p = particles[gridIndex];
            
            if (!p.active) continue;
            
            gridToVertex[gridIndex] = vertexCount++;
            
            // position
            vertices.push_back(p.position.x);
            vertices.push_back(p.position.y);
            vertices.push_back(p.position.z);
            
            // smooth normal
            glm::vec3 normal = calculateNormal(x, y);
            vertices.push_back(normal.x);
            vertices.push_back(normal.y);
            vertices.push_back(normal.z);
            
            // texture coords
            vertices.push_back(x / float(gridWidth - 1));
            vertices.push_back(y / float(gridHeight - 1));
        }
    }
    
    // triangle indices
    for (int y = 0; y < gridHeight - 1; ++y) {
        for (int x = 0; x < gridWidth - 1; ++x) {
            int topLeft = y * gridWidth + x;
            int topRight = y * gridWidth + (x + 1);
            int bottomLeft = (y + 1) * gridWidth + x;
            int bottomRight = (y + 1) * gridWidth + (x + 1);
            
            // check if all particles in quad are active + have valid vertex indices
            if (particles[topLeft].active && particles[topRight].active &&
                particles[bottomLeft].active && particles[bottomRight].active &&
                gridToVertex[topLeft] != -1 && gridToVertex[topRight] != -1 &&
                gridToVertex[bottomLeft] != -1 && gridToVertex[bottomRight] != -1) {
                
                // first triangle
                indices.push_back(gridToVertex[topLeft]);
                indices.push_back(gridToVertex[bottomLeft]);
                indices.push_back(gridToVertex[topRight]);
                
                // second triangle
                indices.push_back(gridToVertex[topRight]);
                indices.push_back(gridToVertex[bottomLeft]);
                indices.push_back(gridToVertex[bottomRight]);
            }
        }
    }
}

glm::vec3 ClothSystem::calculateNormal(int x, int y) const {
    int index = y * gridWidth + x;
    if (!particles[index].active) return glm::vec3(0.0f, 0.0f, 1.0f);
    
    glm::vec3 normal(0.0f);
    int validNeighbors = 0;
    
    // sample neighboring particles for normal calculation
    std::vector<glm::vec2> offsets = {
        {1, 0}, {-1, 0}, {0, 1}, {0, -1},
        {1, 1}, {-1, -1}, {1, -1}, {-1, 1}
    };
    
    for (size_t i = 0; i < offsets.size() - 1; ++i) {
        int x1 = x + offsets[i].x;
        int y1 = y + offsets[i].y;
        int x2 = x + offsets[i + 1].x;
        int y2 = y + offsets[i + 1].y;
        
        if (x1 >= 0 && x1 < gridWidth && y1 >= 0 && y1 < gridHeight &&
            x2 >= 0 && x2 < gridWidth && y2 >= 0 && y2 < gridHeight) {
            
            int idx1 = y1 * gridWidth + x1;
            int idx2 = y2 * gridWidth + x2;
            
            if (particles[idx1].active && particles[idx2].active) {
                glm::vec3 v1 = particles[idx1].position - particles[index].position;
                glm::vec3 v2 = particles[idx2].position - particles[index].position;
                normal += glm::cross(v1, v2);
                validNeighbors++;
            }
        }
    }
    
    if (validNeighbors > 0) {
        normal = glm::normalize(normal);
    } else {
        normal = glm::vec3(0.0f, 0.0f, 1.0f);
    }
    
    return normal;
}

void ClothSystem::setMode(SimulationMode mode) {
    reset();
    
    switch (mode) {
        case SimulationMode::TEAR:
            clearCollisionObjects();

            windStrength = 0.0f;
            // standard - top side pinned
            for (int x = 0; x < gridWidth; ++x) {
                particles[(gridHeight - 1) * gridWidth + x].pinned = true;
            }
            break;
            
        case SimulationMode::COLLISION:
            clearCollisionObjects();

            addSphere(glm::vec3(0.0f, 1.0f, 6.0f), 0.8f);
            windStrength = 0.0f;
            
            // pin top corners so cloth "hangs"
            particles[(gridHeight - 1) * gridWidth].pinned = true;                      // top-left
            particles[(gridHeight - 1) * gridWidth + (gridWidth - 1)].pinned = true;    // top-right
            break;
            
        case SimulationMode::FLAG:
            clearCollisionObjects();

            windStrength = 6.0f;  
            windDirection = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));  // blow in -Z direction (towards viewer)
            
            // for flag, pin top edge
            for (int x = 0; x < gridWidth; ++x) {
                particles[(gridHeight - 1) * gridWidth + x].pinned = true;
            }
            break;
    }
}

void ClothSystem::handleMouseInteraction(const glm::vec3& mousePos, bool tearing) {
    if (!tearing) return;
    
    // find particles within tear radius
    float tearRadius = 0.08f;
    
    for (size_t i = 0; i < particles.size(); ++i) {
        if (!particles[i].active) continue;
        
        float distance = glm::length(particles[i].position - mousePos);
        if (distance < tearRadius) {
            // deactivate particle
            particles[i].active = false;
            
            // deactivate connected springs
            for (auto& spring : springs) {
                int j = static_cast<int>(i);
                if (spring.particle1 == j || spring.particle2 == j) {
                    spring.active = false;
                }
            }
        }
    }
}

void ClothSystem::reset() {
    createClothGrid();
}

void ClothSystem::addSphere(const glm::vec3& center, float radius) {
    spheres.emplace_back(center, radius);
}

void ClothSystem::clearCollisionObjects() {
    spheres.clear();
}

void ClothSystem::updateObjectMovement(float deltaTime) {
    if (spheres.empty()) return;

    static glm::vec3 startPos = spheres[0].center;
    static float angle = 0.0f;   // tracks semicircle motion
    static bool goingForward = true;

    objectMoveTime += deltaTime * objectMoveSpeed;
    float radius = objectMoveRange * 0.5f;  // half of old back-and-forth

    if (goingForward) {
        // move straight toward camera along Z
        spheres[0].center.z = startPos.z - objectMoveTime;

        // once we reach the forward point, start semicircle
        if (spheres[0].center.z <= startPos.z - objectMoveRange) {
            goingForward = false;
            angle = 0.0f; 
        }

    } else {
        // semicircle around back to original pos
        angle += deltaTime * objectMoveSpeed;

        float x = startPos.x + radius * sin(angle);                  
        float z = (startPos.z - objectMoveRange) + radius * (1 - cos(angle)); 

        spheres[0].center = glm::vec3(x, startPos.y, z);

        if (angle >= 3.14159f) {
            goingForward = true;
            objectMoveTime = 0.0f;
            spheres[0].center = startPos;
        }
    }
}

void ClothSystem::updateWindVariation(float deltaTime) {
    // only add wind variation in flag mode
    if (windStrength < 1.0f) return;

    windVariationTime += deltaTime * 3.0f; 
    
    float variationX = sin(windVariationTime * 1.5f) * windVariationStrength;
    float variationY = sin(windVariationTime * 2.3f) * windVariationStrength * 0.5f;
    float variationZ = cos(windVariationTime * 1.8f) * windVariationStrength * 0.3f;
    
    // apply variations to base wind direction
    glm::vec3 baseWind = glm::normalize(glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 variedWind = baseWind + glm::vec3(variationX, variationY, variationZ);

    windDirection = glm::normalize(variedWind);
}
