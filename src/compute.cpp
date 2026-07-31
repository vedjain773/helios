#include "compute.hpp"
#include <fstream>
#include <filesystem>
#include <sstream>
#include <string>
#include <iostream>

ComputeShader::ComputeShader(const std::string &computeShaderPath) {
    std::filesystem::path rootPath("../src/shaders/");
    std::ifstream computeShaderFile(computeShaderPath);
    
    if (!computeShaderFile) {
        throw std::runtime_error("Compute shader source file not found");
    }

    std::string csBufStr = "";
    std::string line;
    while (std::getline(computeShaderFile, line)) {
        if (line.size() > 8 && !line.substr(0, 8).compare("#include")) {
            int start = line.find('"');
            int end = line.rfind('"');
            
            std::filesystem::path filepath(line.substr(start + 1, end - start - 1));
            filepath = rootPath / filepath;
            std::ifstream incFile(filepath);

            std::stringstream csBuf;
            csBuf << incFile.rdbuf();

            csBufStr.append(csBuf.str());
        } else {
            csBufStr.append(line);
        }

        csBufStr += '\n';
    }

    const char *computeShaderSource = csBufStr.c_str();
    computeShaderFile.close();

    unsigned int computeShader;
    computeShader = glCreateShader(GL_COMPUTE_SHADER);

    glShaderSource(computeShader, 1, &computeShaderSource, NULL);
    glCompileShader(computeShader);
    checkCompileStatus(computeShader);

    ID = glCreateProgram();

    glAttachShader(ID, computeShader);
    glLinkProgram(ID);

    glDeleteShader(computeShader);
}

void ComputeShader::checkCompileStatus(unsigned int id) {
    int success;
    char infoLog[512];

    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if(!success) {
        glGetShaderInfoLog(id, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    }
}

void ComputeShader::setBool(const std::string &name, bool value) {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void ComputeShader::setInt(const std::string &name, int value) {
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value); 
}

void ComputeShader::setFloat(const std::string &name, float value) {
    glUniform1f(glGetUniformLocation(ID, name.c_str()), value); 
}

void ComputeShader::setVec2(const std::string &name, float x, float y) {
    glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y);
}

void ComputeShader::use() {
    glUseProgram(ID);
}
