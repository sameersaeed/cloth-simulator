#ifndef RENDERER_H
#define RENDERER_H

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <vector>
#include <string>
#include <memory>


class Camera;
class ClothSystem;

class Shader {
private:
    unsigned int ID;
    
public:
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();
    
    void use() const;
    unsigned int getID() const { return ID; }
    
    // setters (utils)
    void setBool(const std::string& name, bool value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const;
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setMat4(const std::string& name, const glm::mat4& mat) const;
    
private:
    unsigned int compileShader(const char* source, unsigned int type);
    void checkCompileErrors(unsigned int shader, const std::string& type);
};

class Skybox {
private:
    unsigned int VAO, VBO;
    unsigned int textureID;
    std::unique_ptr<Shader> shader;
    
public:
    Skybox();
    ~Skybox();
    
    bool initialize();
    void render(const glm::mat4& view, const glm::mat4& projection);
    
private:
    unsigned int loadCubemap();
};

class Renderer {
private:
    // shaders
    std::unique_ptr<Shader> clothShader;
    std::unique_ptr<Shader> objectShader;
    std::unique_ptr<Skybox> skybox;
    
    // cloth rendering
    unsigned int clothVAO, clothVBO, clothEBO;
    unsigned int clothTexture;
    
    // collision object rendering
    unsigned int sphereVAO, sphereVBO, sphereEBO;
    
    // sphere mesh data
    std::vector<float> sphereVertices;
    std::vector<unsigned int> sphereIndices;
    
public:
    Renderer();
    ~Renderer();
    
    bool initialize();
    void createScene(const ClothSystem& cloth, const Camera& camera, bool wireframe);
    void cleanup();
    
private:
    void setupClothBuffers();
    void setupCollisionObjectBuffers();
    void renderCloth(const ClothSystem& cloth, const Camera& camera, bool wireframe);
    void renderCollisionObjects(const ClothSystem& cloth, const Camera& camera);
    void generateSphereMesh(float radius, int segments);
    
    // embedded shaders
    const char* getClothVertexShader();
    const char* getClothFragmentShader();
    const char* getObjectVertexShader();
    const char* getObjectFragmentShader();
};

#endif 
