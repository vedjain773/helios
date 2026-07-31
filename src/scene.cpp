#include "scene.hpp"
#include "compute.hpp"

void Scene::addSphere(Sphere &sphere) {
    GPUSphere gpuSphere;
    gpuSphere.center = glm::vec3(sphere.center.x, sphere.center.y, sphere.center.z);
    gpuSphere.radius = sphere.radius;
    gpuSphere.matId = sphere.matId;

    spheres.push_back(gpuSphere);
}

void Scene::addMaterial(Material &material) {
    GPUMaterial gpuMaterial;
    gpuMaterial.albedo = glm::vec3(material.albedo.x, material.albedo.y, material.albedo.z);
    gpuMaterial.metallic = material.metallic;
    gpuMaterial.roughness = material.roughness;
    gpuMaterial.ior = material.ior;
    gpuMaterial.transmissive = material.transmissive;

    materials.push_back(gpuMaterial);
    cpuMaterials.emplace_back(material);
}

void Scene::addVertices(Vertex &vertex) {
    GPUVertex gpuvertex;
    gpuvertex.position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
    gpuvertex.normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z);

    vertices.push_back(gpuvertex);
}

void Scene::addIndices(int index) {
    indices.push_back(index);
}

void Scene::update(int index) {
    GPUMaterial &target = materials[index];
    Material material = cpuMaterials[index];

    target.albedo = glm::vec3(material.albedo.x, material.albedo.y, material.albedo.z);
    target.metallic = material.metallic;
    target.roughness = material.roughness;
    target.ior = material.ior;
    target.transmissive = material.transmissive;
} 
