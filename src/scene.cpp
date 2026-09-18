#define GLM_ENABLE_EXPERIMENTAL

#include "scene.hpp"
#include "compute.hpp"
#include "objloader.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include <iostream>
#include <algorithm>
#include <cassert>

void Scene::addSphere(Sphere &sphere) {
    GPUSphere gpuSphere {
        .center = glm::vec3(sphere.center.x, sphere.center.y, sphere.center.z),
        .radius = sphere.radius,
        .matId = sphere.matId
    }; 

    spheres.push_back(gpuSphere);
}

void Scene::addSpheres(std::initializer_list<Sphere> sphereList) {
    for (Sphere sphere: sphereList) {
        addSphere(sphere);
    }
}

void Scene::addMaterial(Material &material) {
    GPUMaterial gpuMaterial = {
        .albedo = glm::vec3(material.albedo.x, material.albedo.y, material.albedo.z),
        .metallic = material.metallic,
        .roughness = material.roughness,
        .ior = material.ior,
        .transmissive = material.transmissive
    }; 

    materials.push_back(gpuMaterial);
    cpuMaterials.emplace_back(material);
}

void Scene::addMaterials(std::initializer_list<Material> materialList) {
    for (Material material: materialList) {
        addMaterial(material);
    }
}

void Scene::addVertices(std::initializer_list<Vertex> vertexList) {
    cpuVertices.insert(cpuVertices.end(), vertexList);

    for (const Vertex &vertex: vertexList) {
        GPUVertex gpuvertex {
            .position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z),
            .normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z) 
        }; 

        vertices.push_back(gpuvertex);
    } 
}

void Scene::addVertices(std::vector<Vertex> &vertexList) {
    cpuVertices.insert(cpuVertices.end(), vertexList.begin(), vertexList.end());

    for (const Vertex &vertex: vertexList) {
        GPUVertex gpuvertex {
            .position = glm::vec3(vertex.position.x, vertex.position.y, vertex.position.z),
            .normal = glm::vec3(vertex.normal.x, vertex.normal.y, vertex.normal.z) 
        }; 

        vertices.push_back(gpuvertex);
    }
} 

void Scene::addIndices(std::initializer_list<int> indexList) {
    indices.insert(indices.end(), indexList); 
}

void Scene::addIndices(const std::array<int, 36> &indexList) {
    indices.insert(indices.end(), indexList.begin(), indexList.end());
}

void Scene::addIndices(std::vector<int> &indexList) {
    indices.insert(indices.end(), indexList.begin(), indexList.end());
}

void Scene::addTriMatIds(std::initializer_list<int> triMatList) {
    triMatIds.insert(triMatIds.end(), triMatList);
}

void Scene::addTriMatIds(std::vector<int> &triMatList) {
    triMatIds.insert(triMatIds.end(), triMatList.begin(), triMatList.end());
}

void Scene::buildBVH() {
    BVHBuilder builder(cpuVertices, indices, triMatIds);
    builder.buildTree(0, 0);

    std::vector<int> indicesN = builder.getIndices();
    std::vector<int> triMatIdsN = builder.getMatIDs();
    nodes = builder.getNodes();

    assert(indicesN.size() == indices.size());
    assert(triMatIdsN.size() == triMatIds.size());

    assert(std::is_permutation(indices.begin(), indices.end(), indicesN.begin()));

    indices = indicesN;
    triMatIds = triMatIdsN;
}

void Scene::updateLight(const Vec3 &corner, const Vec3 &edge1, const Vec3 &edge2) {
    light.corner = glm::vec3(corner.x, corner.y, corner.z);
    light.edge1 = glm::vec3(edge1.x, edge1.y, edge1.z);
    light.edge2 = glm::vec3(edge2.x, edge2.y, edge2.z);
}

void Scene::uploadLight(unsigned int ID) {
    int loc;

    loc = glGetUniformLocation(ID, "quadLight.corner");
    glUniform3fv(loc, 1, glm::value_ptr(light.corner));
    
    loc = glGetUniformLocation(ID, "quadLight.edge1");
    glUniform3fv(loc, 1, glm::value_ptr(light.edge1));
    
    loc = glGetUniformLocation(ID, "quadLight.edge2");
    glUniform3fv(loc, 1, glm::value_ptr(light.edge2));

    loc = glGetUniformLocation(ID, "quadLight.normal");
    glUniform3fv(loc, 1, glm::value_ptr(light.normal));

    loc = glGetUniformLocation(ID, "quadLight.emission");
    glUniform3fv(loc, 1, glm::value_ptr(light.emission));
}

void Scene::uploadBounces(unsigned int ID, int numberOfBounces) {
    int loc;
    
    loc = glGetUniformLocation(ID, "MAX_BOUNCES");
    glUniform1i(loc, numberOfBounces);
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

    scene.addSpheres({sp_gl, sp_metal, sp_diff, sp_gr});
    scene.addMaterials({mat_gl, mat_metal, mat_diff, mat_gr});

    scene.updateLight(
        {-0.15, 0.49, -0.15},
        {0.3, 0.0, 0.0},
        {0.0, 0.0, 0.3}
    );
        
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

    scene.addMaterials({mat_red, mat_green, mat_white});

    scene.addVertices({v0, v1, v2, v3, v4, v5, v6, v7});
    int base = 0;
    scene.addIndices({
            base + 0, base + 1, base + 2,   base + 0, base + 2, base + 3,
            base + 4, base + 0, base + 3,   base + 4, base + 3, base + 7,
            base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
            base + 4, base + 5, base + 1,   base + 4, base + 1, base + 0,
            base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
    });
    
    scene.addTriMatIds({2, 2, 0, 0, 1, 1, 2, 2, 2, 2});
    
    Material small = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};
    scene.addMaterial(small);
    
    Mesh smallMesh = createCubeMesh(scene, {0.06, -0.5, -0.05}, {0.3, 0.3, 0.3}, 3); 
    rotateMesh(scene, smallMesh, -18.0f);

    Material tall = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};
    scene.addMaterial(tall);
    
    Mesh tallMesh = createCubeMesh(scene, {-0.25, -0.5, -0.3}, {0.25, 0.6, 0.25}, 4);
    rotateMesh(scene, tallMesh, 16.0f); 
    
    scene.buildBVH();

    scene.updateLight(
        {-0.15, 0.49, -0.15},
        {0.3, 0.0, 0.0},
        {0.0, 0.0, 0.3}
    );

    return scene;
}

Scene buildCubeAlt() {
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

    scene.addMaterials({mat_red, mat_green, mat_white});

    scene.addVertices({v0, v1, v2, v3, v4, v5, v6, v7});
    int base = 0;
    scene.addIndices({
            base + 0, base + 1, base + 2,   base + 0, base + 2, base + 3,
            base + 4, base + 0, base + 3,   base + 4, base + 3, base + 7,
            base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
            base + 4, base + 5, base + 1,   base + 4, base + 1, base + 0,
            base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
    });
    
    scene.addTriMatIds({2, 2, 0, 0, 1, 1, 2, 2, 2, 2});
     
    Material tall = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};
    scene.addMaterial(tall);
    
    Mesh tallMesh = createCubeMesh(scene, {-0.25, -0.5, -0.3}, {0.25, 0.6, 0.25}, 4);
    rotateMesh(scene, tallMesh, 16.0f);

    Material mat_gl = {{0.1, 0.1, 0.1}, 0.25, 0.25, 1.5, 1};
    Sphere sp_gl = {{0.15, -0.35, 0.15}, 0.15, 4};

    scene.addMaterial(mat_gl);
    scene.addSphere(sp_gl);

    scene.buildBVH();

    scene.updateLight(
        {-0.15, 0.49, -0.15},
        {0.3, 0.0, 0.0},
        {0.0, 0.0, 0.3}
    );

    return scene;
}

Scene buildObj() {
    ObjLoader objLoader("../obj/suzanne.obj");
    Scene scene;

    Material mat_red = {{1.0, 0.0, 0.0}, 0.00, 1.00, 0.00, 0};
    Material mat_green = {{0.0, 1.0, 0.0}, 0.00, 1.00, 0.00, 0};
    Material mat_white = {{1.0, 1.0, 1.0}, 0.00, 1.00, 0.00, 0};

    Vertex v0 = {{-4.0, -0.25, 3.0}};
    Vertex v1 = {{-1.0, -0.25, 3.0}};
    Vertex v2 = {{-1.0,  2.5,  3.0}};
    Vertex v3 = {{-4.0,  2.5,  3.0}};
    Vertex v4 = {{-4.0, -0.0,  6.0}};
    Vertex v5 = {{-1.0, -0.0,  6.0}};
    Vertex v6 = {{-1.0,  2.5,  6.0}};
    Vertex v7 = {{-4.0,  2.5,  6.0}};

    scene.addMaterials({mat_red, mat_green, mat_white});

    scene.addVertices({v0, v1, v2, v3, v4, v5, v6, v7});
    int base = 0;
    scene.addIndices({
            base + 0, base + 1, base + 2,   base + 0, base + 2, base + 3,
            base + 4, base + 0, base + 3,   base + 4, base + 3, base + 7,
            base + 1, base + 5, base + 6,   base + 1, base + 6, base + 2,
            base + 4, base + 5, base + 1,   base + 4, base + 1, base + 0,
            base + 3, base + 2, base + 6,   base + 3, base + 6, base + 7
    });
    
    scene.addTriMatIds({2, 2, 0, 0, 1, 1, 2, 2, 2, 2});

    Material mat_w = {{0.901, 0.880, 0.811}, 0.00, 1.00, 0.00, 0};
    scene.addMaterial(mat_w);

    Mesh objmesh = objLoader.createObjMesh(scene, 3);
    std::cout << "Done: " << objmesh.vertexOffset << "\n";
    
    scene.buildBVH();

    scene.updateLight(
        {-2.5, 2.49, 4.5},
        {0.5, 0.0, 0.0},
        {0.0, 0.0, 0.5}
    );

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

Mesh createCubeMesh(Scene &scene, const Vec3 &corner, const Vec3 &dim, int matId) {
    int vertexOffset = scene.vertices.size();
    int indexOffset = scene.indices.size(); 

    scene.addVertices({
        {corner + Vec3{0, 0, 0}},   
        {corner + Vec3{dim.x, 0, 0}},
        {corner + Vec3{dim.x, dim.y, 0}}, 
        {corner + Vec3{0, dim.y, 0}},
        {corner + Vec3{0, 0, dim.z}}, 
        {corner + Vec3{dim.x, 0, dim.z}},
        {corner + Vec3{dim.x, dim.y, dim.z}}, 
        {corner + Vec3{0, dim.y, dim.z}}
    });

    int base = vertexOffset; 
    scene.addIndices(getCubeIndices(base));

    int indexCount = scene.indices.size() - indexOffset;
    int vertexCount = scene.vertices.size() - vertexOffset;

    std::vector<int> triMatList(indexCount / 3, matId);
    scene.addTriMatIds(triMatList);

    return Mesh{vertexOffset, vertexCount, indexOffset, indexCount, matId};
}

void rotateMesh(Scene &scene, Mesh &mesh, float angle) {
    int vertexOffset = mesh.vertexOffset;
    int vertexCount = mesh.vertexCount;

    for (int i = vertexOffset; i < vertexOffset + vertexCount; i++) {
        glm::vec3 pos = scene.vertices[i].position;

        glm::vec3 rotated = glm::rotate(pos, glm::radians(angle),
                glm::vec3(0.0f, 1.0f, 0.0f));

        scene.vertices[i].position = rotated; 
        
        scene.cpuVertices[i].position = {
            rotated.x, rotated.y, rotated.z
        };
    }
}
