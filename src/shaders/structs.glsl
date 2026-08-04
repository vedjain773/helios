struct Ray {
    vec3 source;
    vec3 direction;
};

struct QuadLight {
    vec3 corner;
    vec3 edge1;
    vec3 edge2;
    vec3 normal;
    vec3 emission;
};

struct LightSample {
    vec3 point;
    vec3 normal;
    vec3 emission;
    float pdfArea;
};

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
    float ior;
    int transmissive;
};

struct HitRecord {
    vec3 point;
    vec3 normal;
    float t;
    bool frontFace;
    int matId;
};

struct Interval {
    float tmin;
    float tmax;
};

struct Sphere {
    vec3 center;
    float radius;
    int matId;
};

struct Vertex {
    vec3 position;
    vec3 normal;
};

struct Triangle {
    vec3 a, b, c;
    int matId;
};

struct BSDFSample {
    vec3 w_i;
    float pdf_diff;
    float pdf_spec;
};

