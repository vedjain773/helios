#ifndef SHAPES_H
#define SHAPES_H

#include "vector.hpp"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

struct Material {
    Vec3 albedo;
    float metallic;
    float roughness;
    float ior;
    int transmissive;
};

struct GPUMaterial {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ior;
    int transmissive;
    float _pad2 = 0.0;
};

struct Sphere {
    Vec3 center;
    float radius;
    int matId;
}; 

struct GPUSphere {
    glm::vec3 center;
    float radius;
    int matId;

    float _pad0 = 0.0;
    float _pad1 = 0.0;
    float _pad2 = 0.0;
};

struct Vertex {
    Vec3 position;
    Vec3 normal = {0.0f};
};

struct GPUVertex {
    glm::vec3 position;
    float _pad0 = 0.0;
    glm::vec3 normal;
    float _pad1 = 0.0;
};

struct Mesh {
    int vertexOffset;
    int vertexCount;
    int indexOffset;
    int indexCount;
    int matId;
};

glm::vec3 vertexToGLM(const Vertex &vertex);

#endif
