#include "Renderer.h"
#include "ClothSystem.h"
#include "Camera.h"
#include <iostream>
#include <fstream>
#include <sstream>

Shader::Shader(const std::string& vertexPath, const std::string& fragmentPath) {
    std::string vertexCode, fragmentCode;
    std::ifstream vShaderFile, fShaderFile;
    
    vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
    
    // try to open shader file if it exists
    try {
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();
        
        vShaderFile.close();
        fShaderFile.close();
        
        vertexCode = vShaderStream.str();
        fragmentCode = fShaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << '\n';
    }
    
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();
    
    // compile and attach shader
    ID = 0;
    unsigned int vertex = compileShader(vShaderCode, GL_VERTEX_SHADER);
    unsigned int fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER);
    
    if (vertex && fragment) {
        ID = glCreateProgram();
        glAttachShader(ID, vertex);
        glAttachShader(ID, fragment);
        glLinkProgram(ID);
        
        checkCompileErrors(ID, "PROGRAM");
    }
    
    if (vertex)     glDeleteShader(vertex);
    if (fragment)   glDeleteShader(fragment);
}


Shader::~Shader() {
    if (ID) glDeleteProgram(ID);
}

void Shader::use() const {
    glUseProgram(ID);
}

unsigned int Shader::compileShader(const char* source, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    std::string typeStr = (type == GL_VERTEX_SHADER) ? "VERTEX" : "FRAGMENT";
    checkCompileErrors(shader, typeStr);
    
    return shader;
}

void Shader::checkCompileErrors(unsigned int shader, const std::string& type) {
    int success;
    char infoLog[1024];
    
    if (type != "PROGRAM") {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << '\n';
        }
    } else {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << '\n';
        }
    }
}

void Shader::setBool(const std::string& name, bool value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}

void Shader::setInt(const std::string& name, int value) const {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string& name, float value) const {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setVec3(const std::string& name, const glm::vec3& value) const {
    glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
}

void Shader::setMat4(const std::string& name, const glm::mat4& mat) const {
    glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
}

Skybox::Skybox() : VAO(0), VBO(0), textureID(0) {}

Skybox::~Skybox() {
    if (VAO) glDeleteVertexArrays(1, &VAO);
    if (VBO) glDeleteBuffers(1, &VBO);
    if (textureID) glDeleteTextures(1, &textureID);
}

bool Skybox::initialize() {
    // skybox vertices
    float skyboxVertices[] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        
        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        
        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,
        
        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,
        
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f
    };
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    
    textureID = 1; 
    shader = std::make_unique<Shader>("../shaders/skybox.vert", "../shaders/skybox.frag");
    
    return shader->getID() != 0;
}

void Skybox::render(const glm::mat4& view, const glm::mat4& projection) {
    if (!shader || VAO == 0) return;
    
    glDepthFunc(GL_LEQUAL);
    shader->use();
    
    // remove translation from view matrix
    glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
    
    shader->setMat4("view", skyboxView);
    shader->setMat4("projection", projection);
    
    glBindVertexArray(VAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    
    glDepthFunc(GL_LESS);
}

unsigned int Skybox::loadCubemap() {
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    
    for (unsigned int i = 0; i < 6; i++) {
        const int size = 256;
        std::vector<unsigned char> data(size * size * 3);
        
        for (int y = 0; y < size; ++y) {
            for (int x = 0; x < size; ++x) {
                int idx = (y * size + x) * 3;
                
                switch (i) {
                    case 0: // right - warm sunset
                        data[idx] = 255;
                        data[idx + 1] = std::max(0, 200 - y/2);
                        data[idx + 2] = std::max(0, 150 - y/2);
                        break;
                    case 1: // left - cool dawn
                        data[idx] = std::max(0, 150 - y/3);
                        data[idx + 1] = std::max(0, 200 - y/3);
                        data[idx + 2] = 255;
                        break;
                    case 2: // top - bright sky
                        data[idx] = 135;
                        data[idx + 1] = 206;
                        data[idx + 2] = 235;
                        break;
                    case 3: // bottom - horizon
                        data[idx] = 70;
                        data[idx + 1] = 130;
                        data[idx + 2] = 180;
                        break;
                    case 4: // front - day sky
                        data[idx] = std::max(0, 180 - y/3);
                        data[idx + 1] = std::max(0, 190 - y/3);
                        data[idx + 2] = std::max(0, 220 - y/4);
                        break;
                    case 5: // back - evening sky
                        data[idx] = std::max(0, 170 - y/3);
                        data[idx + 1] = std::max(0, 180 - y/3);
                        data[idx + 2] = std::max(0, 210 - y/4);
                        break;
                }
            }
        }
        
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, 
                     size, size, 0, GL_RGB, GL_UNSIGNED_BYTE, data.data());
    }
    
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return textureID;
}

Renderer::Renderer() : clothVAO(0), clothVBO(0), clothEBO(0), clothTexture(0),
                      sphereVAO(0), sphereVBO(0), sphereEBO(0) {}

Renderer::~Renderer() {
    cleanup();
}

bool Renderer::initialize() {
    // cloth shader
    clothShader = std::make_unique<Shader>("../shaders/cloth.vert", "../shaders/cloth.frag");
    objectShader = std::make_unique<Shader>("../shaders/object.vert", "../shaders/object.frag");
    
    if (!clothShader->getID() || !objectShader->getID()) {
        std::cout << "Failed to create shaders\n";
        return false;
    }
    
    setupClothBuffers();
    setupCollisionObjectBuffers();
    
    generateSphereMesh(1.0f, 64);
    
    // skybox 
    skybox = std::make_unique<Skybox>();
    skybox->initialize();

    return true;
}

void Renderer::setupClothBuffers() {
    glGenVertexArrays(1, &clothVAO);
    glGenBuffers(1, &clothVBO);
    glGenBuffers(1, &clothEBO);
    
    glBindVertexArray(clothVAO);
    glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothEBO);
    
    // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    
    // normal 
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    // texture coord
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
}

void Renderer::setupCollisionObjectBuffers() {
    // sphere VAO
    glGenVertexArrays(1, &sphereVAO);
    glGenBuffers(1, &sphereVBO);
    glGenBuffers(1, &sphereEBO);
}

void Renderer::generateSphereMesh(float radius, int segments) {
    sphereVertices.clear();
    sphereIndices.clear();
    
    // vertices
    for (int lat = 0; lat <= segments; ++lat) {
        float theta = lat * M_PI / segments;
        float sinTheta = sin(theta);
        float cosTheta = cos(theta);
        
        for (int lon = 0; lon <= segments; ++lon) {
            float phi = lon * 2 * M_PI / segments;
            float sinPhi = sin(phi);
            float cosPhi = cos(phi);
            
            float x = cosPhi * sinTheta;
            float y = cosTheta;
            float z = sinPhi * sinTheta;
            
            // position
            sphereVertices.push_back(x * radius);
            sphereVertices.push_back(y * radius);
            sphereVertices.push_back(z * radius);
            
            // normal
            sphereVertices.push_back(x);
            sphereVertices.push_back(y);
            sphereVertices.push_back(z);
            
            // texture coords
            sphereVertices.push_back((float)lon / segments);
            sphereVertices.push_back((float)lat / segments);
        }
    }
    
    // sphere indices
    for (int lat = 0; lat < segments; ++lat) {
        for (int lon = 0; lon < segments; ++lon) {
            int first = lat * (segments + 1) + lon;
            int second = first + segments + 1;
            
            sphereIndices.push_back(first);
            sphereIndices.push_back(second);
            sphereIndices.push_back(first + 1);
            
            sphereIndices.push_back(second);
            sphereIndices.push_back(second + 1);
            sphereIndices.push_back(first + 1);
        }
    }
}

void Renderer::createScene(const ClothSystem& cloth, const Camera& camera, bool wireframe) {
    // render skybox first (background)
    if (skybox) {
        skybox->render(camera.getViewMatrix(), camera.getProjectionMatrix(1920.0f / 1080.0f));
    }
    
    renderCloth(cloth, camera, wireframe);
    renderCollisionObjects(cloth, camera);
}

void Renderer::renderCloth(const ClothSystem& cloth, const Camera& camera, bool wireframe) {
    clothShader->use();
    
    // set uniforms
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(1920.0f / 1080.0f);
    
    clothShader->setMat4("model", model);
    clothShader->setMat4("view", view);
    clothShader->setMat4("projection", projection);
    
    clothShader->setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    clothShader->setVec3("viewPos", camera.getPosition());
    clothShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    clothShader->setVec3("clothColor", glm::vec3(0.9f, 0.9f, 0.95f)); 
    clothShader->setBool("wireframe", wireframe);
    
    // bind cloth data - now using fiber data instead of triangular mesh
    glBindVertexArray(clothVAO);
    
    const auto& fiberVertices = cloth.getVertices();
    const auto& fiberIndices = cloth.getIndices();
    
    if (!fiberVertices.empty() && !fiberIndices.empty()) {
        glBindBuffer(GL_ARRAY_BUFFER, clothVBO);
        glBufferData(GL_ARRAY_BUFFER, fiberVertices.size() * sizeof(float), fiberVertices.data(), GL_DYNAMIC_DRAW);
        
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, clothEBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, fiberIndices.size() * sizeof(unsigned int), fiberIndices.data(), GL_DYNAMIC_DRAW);
        
        // so we can render cloth from both sides
        glDisable(GL_CULL_FACE);
        
        // render
        if (wireframe) {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            glLineWidth(1.0f);
        } else {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
        
        glDrawElements(GL_TRIANGLES, fiberIndices.size(), GL_UNSIGNED_INT, 0);
        
        // reset polygon mode and re-enable face culling
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        glEnable(GL_CULL_FACE);
    }
    
    glBindVertexArray(0);
}

void Renderer::renderCollisionObjects(const ClothSystem& cloth, const Camera& camera) {
    objectShader->use();
    
    glm::mat4 view = camera.getViewMatrix();
    glm::mat4 projection = camera.getProjectionMatrix(1920.0f / 1080.0f);
    
    objectShader->setMat4("view", view);
    objectShader->setMat4("projection", projection);
    objectShader->setVec3("lightPos", glm::vec3(5.0f, 5.0f, 5.0f));
    objectShader->setVec3("viewPos", camera.getPosition());
    objectShader->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    
    // render sphere
    glBindVertexArray(sphereVAO);
    glBindBuffer(GL_ARRAY_BUFFER, sphereVBO);
    glBufferData(GL_ARRAY_BUFFER, sphereVertices.size() * sizeof(float), sphereVertices.data(), GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, sphereEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphereIndices.size() * sizeof(unsigned int), sphereIndices.data(), GL_STATIC_DRAW);
    
    // sphere vertex attribs
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    // render sphere collision
    for (const auto& sphere : cloth.getSpheres()) {
        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, sphere.center);
        model = glm::scale(model, glm::vec3(sphere.radius));
        
        objectShader->setMat4("model", model);
        objectShader->setVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.0f)); 
        
        glDrawElements(GL_TRIANGLES, sphereIndices.size(), GL_UNSIGNED_INT, 0);
    }
    
    glBindVertexArray(0);
}

void Renderer::cleanup() {
    if (clothVAO)       glDeleteVertexArrays(1, &clothVAO);
    if (clothVBO)       glDeleteBuffers(1, &clothVBO);
    if (clothEBO)       glDeleteBuffers(1, &clothEBO);
    if (sphereVAO)      glDeleteVertexArrays(1, &sphereVAO);
    if (sphereVBO)      glDeleteBuffers(1, &sphereVBO);
    if (sphereEBO)      glDeleteBuffers(1, &sphereEBO);
    if (clothTexture)   glDeleteTextures(1, &clothTexture);
}
