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

struct GPUMaterial {
    glm::vec3 albedo;
    float metallic;
    float roughness;
    float ior;
    int transmissive;
    float _pad2 = 0.0;
};

#endif
