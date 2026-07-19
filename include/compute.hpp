#ifndef COMPUTE_H
#define COMPUTE_H

#include "glad/glad.h"
#include <string>

class ComputeShader {
  public:
    unsigned int ID;

    ComputeShader(const std::string &computeShaderPath);

    void checkCompileStatus(unsigned int id);
    void setBool(const std::string &name, bool value);
    void setInt(const std::string &name, int value);
    void setFloat(const std::string &name, float value);
    void setVec2(const std::string &name, float x, float y);

    void use();
};

#endif
