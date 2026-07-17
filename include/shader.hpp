#ifndef SHADER_H
#define SHADER_H

#include "glad/glad.h"
#include <string>

class Shader {
  public:
    unsigned int ID;

    Shader(const std::string &vertexShaderPath, const std::string &fragmentShaderPath);

    void checkCompileStatus(unsigned int id);
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, float x, float y);

    void use();
};

#endif
