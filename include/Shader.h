#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <glm/glm.hpp> // For glm::mat4
// Make sure these are present if your Shader.cpp uses them and they aren't pulled by Shader.h itself
#include <fstream> 
#include <sstream>
#include <iostream>
#include <glad/glad.h>


class Shader {
public:
    // Constructor generates the shader on the fly
    Shader(const char* vertexPath, const char* fragmentPath);
    
    // Deletes the copy constructor and copy assignment operator (if you added these)
    // Shader(const Shader&) = delete;
    // Shader& operator=(const Shader&) = delete;
    // Allows moving the shader (move constructor and move assignment operator) (if you added these)
    // Shader(Shader&& other) noexcept;
    // Shader& operator=(Shader&& other) noexcept;
    
    ~Shader();

    void use();
    void setBool(const std::string &name, bool value) const;
    void setInt(const std::string &name, int value) const;
    void setFloat(const std::string &name, float value) const;
    void setMat4(const std::string &name, const glm::mat4 &mat) const;
    void setVec3(const std::string &name, const glm::vec3 &value) const; // Added
    void setVec3(const std::string &name, float x, float y, float z) const; // Overload
    
    unsigned int getID() const; // <<< ADD OR ENSURE THIS LINE IS PRESENT AND PUBLIC

private:
    unsigned int ID; // ID should be private if using a getter
    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif // SHADER_H