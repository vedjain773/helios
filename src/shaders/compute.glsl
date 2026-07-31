#version 450 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;
layout(rgba32f, binding = 3) uniform image2D accumBuffer;

const float PI = 3.14159265359;

uniform vec3 center;
uniform vec3 pixel00Loc;
uniform vec3 pixelDeltaU;
uniform vec3 pixelDeltaV;

uniform int frameCounter;

vec3 lightPos = vec3(1, 1, 1);
const int MAX_BOUNCES = 5;
const int NUM_TRIANGLES = 1;

struct Ray {
    vec3 source;
    vec3 direction;
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

layout(std430, binding = 1) buffer SphereBuffer { Sphere spheres[]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[]; };

layout(std430, binding = 4) buffer VertexBuffer { Vertex vertice[]; };
layout(std430, binding = 5) buffer IndexBuffer { int indices[]; };

void buildTB(vec3 n, inout vec3 T, inout vec3 B) {
    vec3 nUp = abs(dot(n, vec3(0, 1, 0))) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    T = normalize(cross(nUp, n));
    B = cross(n, T);
    return;
}

uint pcg_hash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand(inout uint seed) {
    seed = pcg_hash(seed);
    return float(seed) / 4294967295.0;
}

float randTex(ivec2 texelCoord, int offset) {
    uint seed = pcg_hash(uint(texelCoord.x)) 
              ^ pcg_hash(uint(texelCoord.y) * 9781u)
              ^ pcg_hash(uint(frameCounter) * 6271u)
              ^ pcg_hash(uint(offset) * 26699u);
    return rand(seed);
}

vec3 SamplingSpecGGX(float alpha, vec3 n, vec3 w_o, ivec2 texelCoord, inout vec3 h) { 
    float u1 = randTex(texelCoord, 0);
    float u2 = randTex(texelCoord, 8795);

    float theta = atan(alpha * sqrt(u1) / sqrt(1 - u1));
    float phi = 2 * PI * u2;
    
    vec3 hlocal = vec3(sin(theta) * cos(phi), sin(theta) * sin(phi), cos(theta));
    vec3 T, B;
    buildTB(n, T, B);

    h = hlocal.x * T + hlocal.y * B + hlocal.z * n;

    vec3 w_i = 2.0 * dot(w_o, h) * h - w_o;
    return dot(w_i, n) > 0.0 ? w_i : -w_i;
}

vec3 SamplingDiffGGX(vec3 n, vec3 w_o, ivec2 texelCoord) {
    float u1 = randTex(texelCoord, 0);
    float u2 = randTex(texelCoord, 2795);

    float phi = 2 * PI * u2;
    
    vec3 w_ilocal = vec3(sqrt(u1) * cos(phi), sqrt(u1) * sin(phi), sqrt(1 - u1)); 

    vec3 T, B;
    buildTB(n, T, B);

    vec3 w_i = w_ilocal.x * T + w_ilocal.y * B + w_ilocal.z * n;
    return w_i;
}

float pdfSpec(float D, vec3 h, vec3 w_o, vec3 n) {
    return D * dot(n, h) / max(4 * dot(w_o, h), 1e-3);
}

float pdfDiff(vec3 n, vec3 w_i) {
    return dot(n, w_i) / PI;
}

// PBR Utils
float DistributionGGX(vec3 n, vec3 h, float alphaSq) {
    //alpha -> roughness ^ 2
    //h -> normalize(w_o + w_i)

    float nDoth = dot(n, h);
    float nDoth_sq = nDoth * nDoth;
    float denominator = nDoth_sq * (alphaSq - 1) + 1;
    
    denominator = PI * denominator * denominator;

    return alphaSq / denominator;
}

float GeometrySchlickGGX(float nDotv, float alphaSq) {
    float numerator = 2 * nDotv;
    float denominator = nDotv + sqrt(alphaSq + (1 - alphaSq) * nDotv * nDotv);

    return numerator / denominator;
}

float GeometrySmith(float nDotw_o, float nDotw_i, float alphaSq) {
    float ggx1 = GeometrySchlickGGX(nDotw_o, alphaSq);
    float ggx2 = GeometrySchlickGGX(nDotw_i, alphaSq);

    return ggx1 * ggx2;
}

vec3 FresnelSchlick(vec3 h, vec3 v, vec3 F0) {
    float hDotv = max(dot(h, v), 0.0);
    return F0 + (1 - F0) * pow(clamp(1 - hDotv, 0.0, 1.0), 5.0);
}

float FresnelSchlickT(float cosTheta, float ior) {
    float r0 = (1 - ior) / (1 + ior);
    r0 = r0 * r0;

    return r0 + (1 - r0) * pow(1 - cosTheta, 5);
}

vec3 fSpecular(vec3 h, vec3 w_o, vec3 w_i, vec3 n, float alphaSq, vec3 F) {
    float nDotw_o = max(dot(n, w_o), 0.0);
    float nDotw_i = max(dot(n, w_i), 0.0);
    
    float D = DistributionGGX(n, h, alphaSq);
    float G = GeometrySmith(nDotw_o, nDotw_i, alphaSq);

    return (D * G * F) / (max(4 * nDotw_o * nDotw_i, 1e-3));
}

vec3 fDiffuse(vec3 albedo, vec3 F, float metallic) {
    vec3 kD = 1 - F;
    kD *= 1 - metallic;
    
    return albedo * kD / PI; 
}

vec3 fTotal(vec3 h, vec3 w_o, vec3 w_i, vec3 n, vec3 F, Material mat) {
    vec3 albedo = mat.albedo;
    float metallic = mat.metallic;
    float alpha = max(mat.roughness * mat.roughness, 1e-3);
    float alphaSq = alpha * alpha;

    vec3 fspec = fSpecular(h, w_o, w_i, n, alphaSq, F);
    vec3 fdiff = fDiffuse(albedo, F, metallic);

    return fspec + fdiff;
}

BSDFSample SampleBSDF(Material matr, ivec2 texelCoord, vec3 F, vec3 n, vec3 w_o) {
    float alpha = matr.roughness * matr.roughness;
    float p_spec = F.x; 

    float u1 = randTex(texelCoord, 12658);

    vec3 nh;
    vec3 w_ispec = SamplingSpecGGX(alpha, n, w_o, texelCoord, nh);
    float pdf_spec = p_spec * pdfSpec(
            DistributionGGX(n, nh, alpha * alpha),
            nh, w_o, n);

    vec3 w_idiff = SamplingDiffGGX(n, w_o, texelCoord);
    float pdf_diff = (1 - p_spec) * pdfDiff(n, w_idiff);

    vec3 w_i = u1 < p_spec ? w_ispec : w_idiff; 
    return BSDFSample(w_i, pdf_diff, pdf_spec);
}

bool isInsideInterval(Interval interval, float t) {
    return t <= interval.tmax && t >= interval.tmin;
}

bool hitSphere(Sphere sphere, Ray ray, Interval interval, inout HitRecord hitr) {
    vec3 center = sphere.center;
    float radius = sphere.radius;

    vec3 oc = center - ray.source;
    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;

    if (discriminant < 0) return false;

    float sqrtDisc = sqrt(discriminant); 
    float root = (-b - sqrtDisc) / (2 * a);

    if (!isInsideInterval(interval, root)) {
        root = (-b + sqrtDisc) / (2 * a);

        if (!isInsideInterval(interval, root))
            return false;
    } 

    hitr.t = root;
    hitr.point = ray.source + root * ray.direction;
    vec3 outNormal = (hitr.point - center) / radius;

    if (dot(outNormal, ray.direction) > 0.0) {
        hitr.normal = -outNormal;
        hitr.frontFace = false;
    } else {
        hitr.normal = outNormal;
        hitr.frontFace = true; 
    }

    hitr.matId = sphere.matId;

    return true;
}

bool hitTriangle(int triId, Ray ray, Interval interval, inout HitRecord hitr) {
    
    vec3 a = vertice[indices[triId * 3]].position;
    vec3 b = vertice[indices[triId * 3 + 1]].position;
    vec3 c = vertice[indices[triId * 3 + 2]].position;

    Triangle tri = Triangle(a, b, c, 1);

    vec3 rayDir = ray.direction;
    vec3 raySrc = ray.source;

    vec3 edge1 = tri.c - tri.a;
    vec3 edge2 = tri.b - tri.a;
    vec3 outNormal = normalize(cross(edge1, edge2));

    vec3 normal;
    bool isFront;

    if (dot(outNormal, rayDir) > 0) {
        normal = -1 * outNormal;
        isFront = false;
    } else {
        normal = outNormal;
        isFront = true;
    }

    if (abs(dot(normal, rayDir)) < 1e-3)
        return false;

    //moller trumbore
    vec3 T = raySrc - tri.a;
    vec3 P = cross(rayDir, edge2);
    vec3 Q = cross(T, edge1);

    float denom = dot(P, edge1);

    if (abs(denom) < 1e-3) 
        return false;

    float t = dot(Q, edge2) / denom;

    if (!isInsideInterval(interval, t))
        return false;

    float u = dot(P, T) / denom;
    float v = dot(Q, rayDir) / denom;

    if (u >= 0 && v >= 0 && u + v <= 1) {
        hitr.point = raySrc + t * rayDir;
        hitr.t = t;
        hitr.normal = normal;
        hitr.frontFace = isFront;
        hitr.matId = tri.matId;
        return true;
    }

    return false;
}

Ray handleDielectric(Ray ray, Material matr, HitRecord hitr, ivec2 texelCoord) {
    vec3 n = normalize(hitr.normal);
    float ri = hitr.frontFace ? 1.0 / matr.ior : matr.ior;

    vec3 unitDir = normalize(ray.direction);
    float cosTheta = min(dot(-unitDir, n), 1.0);
    float sinTheta = sqrt(1.0 - cosTheta * cosTheta);

    bool cannotRefract = ri * sinTheta > 1.0;
    bool didReflect = cannotRefract ||
        (randTex(texelCoord, 8645) < FresnelSchlickT(cosTheta, ri));

    vec3 direction = didReflect ? reflect(unitDir, n) : refract(unitDir, n, ri);
    vec3 offsetN = didReflect ? n : -n;

    return Ray(hitr.point + 1e-3 * offsetN, direction);
}

bool traceRay(Ray ray, Interval interval, inout HitRecord hitr) {
    bool hasHit = false;

    for (int i = 0; i < spheres.length(); i++) {
        if (hitSphere(spheres[i], ray, interval, hitr)) {
            hasHit = true;
            interval.tmax = hitr.t;
        }
    }

    for (int i = 0; i < NUM_TRIANGLES; i++) {
        if (hitTriangle(i, ray, interval, hitr)) {
            hasHit = true;
            interval.tmax = hitr.t;
        }
    }

    return hasHit;
}

bool anyHit(Ray ray) {
    HitRecord hitr;
    Interval interval = Interval(0.001, 1.0);

    for (int i = 0; i < spheres.length(); i++) {
        if (hitSphere(spheres[i], ray, interval, hitr)) {
            Material matr = materials[hitr.matId];
            if (matr.transmissive != 1) return true; 
        } 
    }
    
    for (int i = 0; i < NUM_TRIANGLES; i++) {
        if (hitTriangle(i, ray, interval, hitr)) {
            Material matr = materials[hitr.matId];
            if (matr.transmissive != 1) return true;
        }
    }

    return false;
}

vec3 closestHit(Ray ray, ivec2 texelCoord) {
    HitRecord hitr;
    Ray initRay = ray;
    vec3 radiance = vec3(0.0);
    vec3 throughput = vec3(1.0);

    vec3 missColor = vec3(0.0); 

    for (int i = 0; i < MAX_BOUNCES; i++) {

        if (traceRay(initRay, Interval(0.0, 1000.0), hitr)) {
            Material matr = materials[hitr.matId];
            vec3 n = hitr.normal;

            if (matr.transmissive == 1) {
                initRay = handleDielectric(initRay, matr, hitr, texelCoord);
                throughput *= 1.0;
                continue; 
            }

            vec3 F0 = vec3(0.04);
            F0 = mix(F0, matr.albedo, matr.metallic);
    
            vec3 w_o = -normalize(initRay.direction);
            vec3 w_i = normalize(lightPos - hitr.point);

            float nDotw_i = max(dot(n, w_i), 0.0);
            vec3 h = normalize(w_o + w_i);
          
            vec3 F = FresnelSchlick(h, w_o, F0);
            vec3 direct = fTotal(h, w_o, w_i, n, F, matr) * nDotw_i;
            
            Ray sray = Ray(hitr.point + 0.01 * n, lightPos - hitr.point);
            if (!anyHit(sray)) {
                radiance += throughput * direct;
            }
           
            BSDFSample bsdf_sample = SampleBSDF(matr, texelCoord, F, n, w_o);
            w_i = bsdf_sample.w_i;
            float pdf_spec = bsdf_sample.pdf_spec;
            float pdf_diff = bsdf_sample.pdf_diff;
            float pdf_total = pdf_spec + pdf_diff;

            h = normalize(w_o + w_i);
            F = FresnelSchlick(h, w_o, F0);
            nDotw_i = max(dot(n, w_i), 0.0);

            throughput *= fTotal(h, w_o, w_i, n, F, matr) * nDotw_i / pdf_total;
            initRay = Ray(hitr.point + 1e-3 * n, w_i); 

        } else {
            radiance += throughput * missColor;
            break;
        } 
    }

    return radiance;
}

Ray genRay(ivec2 pixel) {
    vec3 pixelCenter = pixel00Loc 
                      + (float(pixel.x) * pixelDeltaU) 
                      + (float(pixel.y) * pixelDeltaV);
    
    vec3 rayOrigin = center;
    vec3 rayDir = pixelCenter - rayOrigin;
    return Ray(rayOrigin, rayDir);
}

void main() {
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
   
    Ray ray = genRay(texelCoord);
    vec3 sampleColor = closestHit(ray, texelCoord);

    vec3 prevSum = (frameCounter == 0) 
        ? vec3(0.0) 
        : imageLoad(accumBuffer, texelCoord).rgb;

    vec3 newSum = prevSum + sampleColor;
    imageStore(accumBuffer, texelCoord, vec4(newSum, 1.0));

    vec3 averaged = newSum / float(frameCounter + 1);
    averaged = averaged / (averaged + vec3(1.0));
    averaged = pow(averaged, vec3(1.0/2.2)); 
    
    imageStore(imgOutput, texelCoord, vec4(averaged, 1.0));
}
