#include "shader.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

Shader::Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath) {
    std::ifstream vertexShaderFile(vertexShaderPath);
    std::ifstream fragmentShaderFile(fragmentShaderPath);
    
    if (!vertexShaderFile || !fragmentShaderFile) {
        throw std::runtime_error("Vertex/fragment shader source file not found");
    }

    std::stringstream vsBuf, fsBuf;
    vsBuf << vertexShaderFile.rdbuf();
    fsBuf << fragmentShaderFile.rdbuf();
    
    std::string vsBufStr = vsBuf.str();
    std::string fsBufStr = fsBuf.str();

    const char *vertexShaderSource = vsBufStr.c_str();
    const char *fragmentShaderSource = fsBufStr.c_str(); 

    vertexShaderFile.close();
    fragmentShaderFile.close(); 

    unsigned int vertexShader, fragmentShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    checkCompileStatus(vertexShader);

    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    checkCompileStatus(fragmentShader); 

    ID = glCreateProgram();

    glAttachShader(ID, vertexShader);
    glAttachShader(ID, fragmentShader);
    glLinkProgram(ID);

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

void Shader::checkCompileStatus(unsigned int id) {
    int success;
    char infoLog[512];

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void Shader::setBool(const std::string &name, bool value) {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setFloat(const std::string &name, float value) {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void Shader::setVec2(const std::string &name, float x, float y) {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void Shader::use() {
    glUseProgram(ID);
}
