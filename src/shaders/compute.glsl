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
const int NUM_TRIANGLES = 12;

#include "structs.glsl"

layout(std430, binding = 1) buffer SphereBuffer { Sphere spheres[]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[]; };

layout(std430, binding = 4) buffer VertexBuffer { Vertex vertice[]; };
layout(std430, binding = 5) buffer IndexBuffer { int indices[]; };

layout(std430, binding = 6) buffer TriMats { int triMatIds[]; };

void buildTB(vec3 n, inout vec3 T, inout vec3 B) {
    vec3 nUp = abs(dot(n, vec3(0, 1, 0))) < 0.99 ? vec3(0, 1, 0) : vec3(1, 0, 0);
    T = normalize(cross(nUp, n));
    B = cross(n, T);
    return;
}

#include "rand.glsl"
#include "sampling.glsl"
#include "pbr.glsl"

bool isInsideInterval(Interval interval, float t) {
    return t <= interval.tmax && t >= interval.tmin;
}

#include "hit.glsl"

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
