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

void Scene::addVertices(std::initializer_list<Vertex> vertexList) {
    for (const Vertex &vertex: vertexList) {
        GPUVertex gpuvertex;
        gpuvertex.position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z);
        gpuvertex.normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z);

        vertices.push_back(gpuvertex);
    } 
}

void Scene::addIndices(std::initializer_list<int> indexList) {
    indices.insert(indices.end(), indexList); 
}

void Scene::addIndices(const std::array<int, 36> &indexList) {
    indices.insert(indices.end(), indexList.begin(), indexList.end());
}

void Scene::addTriMatIds(std::initializer_list<int> triMatList) {
    triMatIds.insert(triMatIds.end(), triMatList);
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

Scene buildThreeSpheres() {
    Sphere sp_gl = {{-1.05, 0, -1}, 0.5, 0};
    Sphere sp_metal = {{0, 0, -1}, 0.5, 1};
    Sphere sp_diff = {{1.05, 0, -1}, 0.5, 2};
    Sphere sp_gr = {{0, -100.5, -1}, 100.0, 3};

    Material mat_gl = {{0.1, 0.1, 0.1}, 0.25, 0.25, 1.5, 1};
    Material mat_metal = {{0.759, 0.759, 0.759}, 1.00, 0.2, 0.00, 0};
    Material mat_diff = {{1.0, 0.0, 0.0}, 0.00, 1.00, 0.00, 0};
    Material mat_gr = {{0.0, 1.0, 0.0}, 0.25, 0.25, 0.00, 0}; 

    Scene scene;
    scene.addSphere(sp_gl);
    scene.addSphere(sp_metal);
    scene.addSphere(sp_diff);
    scene.addSphere(sp_gr);

    scene.addMaterial(mat_gl);
    scene.addMaterial(mat_metal);
    scene.addMaterial(mat_diff);
    scene.addMaterial(mat_gr);
       
    return scene;
}

Scene buildCube() {
    Scene scene;

    Material mat_red = {{1.0, 0.0, 0.0}, 0.00, 1.00, 0.00, 0};
    Material mat_green = {{0.0, 1.0, 0.0}, 0.00, 1.00, 0.00, 0};
    Material mat_white = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};

    Vertex v0 = {{-0.5, -0.5, -0.5}};
    Vertex v1 = {{ 0.5, -0.5, -0.5}};
    Vertex v2 = {{ 0.5,  0.5, -0.5}};
    Vertex v3 = {{-0.5,  0.5, -0.5}};
    Vertex v4 = {{-0.5, -0.5,  0.5}};
    Vertex v5 = {{ 0.5, -0.5,  0.5}};
    Vertex v6 = {{ 0.5,  0.5,  0.5}};
    Vertex v7 = {{-0.5,  0.5,  0.5}};

    scene.addMaterial(mat_red);
    scene.addMaterial(mat_green);
    scene.addMaterial(mat_white);

    scene.addVertices({v0, v1, v2, v3, v4, v5, v6, v7});
    int base = 0;
    scene.addIndices({
            // back face  (z = -0.5)
            base, base + 1, base + 2,   base, base + 2, base + 3,
            // left face  (x = -0.5)
            base + 4, base, base + 3,   base + 4, base + 3, base + 7,
            // right face (x = 0.5)
            base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
            // bottom face (y = -0.5)
            base + 4, base + 5, base + 1,   base + 4, base + 1, base,
            // top face   (y = 0.5)
            base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
            });
    
    scene.addTriMatIds({2, 2, 0, 0, 1, 1, 2, 2, 2, 2});
    
    Material small = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};

    Vertex s0 = {{ 0.060, -0.50, -0.042}};
    Vertex s1 = {{ 0.342, -0.50,  0.060}};
    Vertex s2 = {{ 0.342, -0.20,  0.060}};
    Vertex s3 = {{ 0.060, -0.20, -0.042}};
    Vertex s4 = {{-0.042, -0.50,  0.240}};
    Vertex s5 = {{ 0.240, -0.50,  0.342}};
    Vertex s6 = {{ 0.240, -0.20,  0.342}};
    Vertex s7 = {{-0.042, -0.20,  0.240}};
    
    scene.addMaterial(small);
    scene.addVertices({s0, s1, s2, s3, s4, s5, s6, s7});

    base = 8;
    scene.addIndices(getCubeIndices(base));
    scene.addTriMatIds({3, 3, 3, 3, 3, 3, 3, 3, 3, 3});

    Material tall = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};

    Vertex t0 = {{-0.303, -0.50, -0.238}};
    Vertex t1 = {{-0.062, -0.50, -0.303}};
    Vertex t2 = {{-0.062,  0.10, -0.303}};
    Vertex t3 = {{-0.303,  0.10, -0.238}};
    Vertex t4 = {{-0.238, -0.50,  0.003}};
    Vertex t5 = {{ 0.003, -0.50, -0.062}};
    Vertex t6 = {{ 0.003,  0.10, -0.062}};
    Vertex t7 = {{-0.238,  0.10,  0.003}};
    
    scene.addMaterial(tall);
    scene.addVertices({t0, t1, t2, t3, t4, t5, t6, t7});

    base = 16;
    scene.addIndices(getCubeIndices(base));
    scene.addTriMatIds({4, 4, 4, 4, 4, 4, 4, 4, 4, 4});

    return scene;
}

std::array<int, 36> getCubeIndices(int base) {
    return {
        base + 0, base + 1, base + 2,   base + 0, base + 2, base + 3,
        base + 4, base + 6, base + 5,   base + 4, base + 7, base + 6,
        base + 4, base + 0, base + 3,   base + 4, base + 3, base + 7,
        base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
        base + 4, base + 5, base + 1,   base + 4, base + 1, base + 0,
        base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
    };
}
