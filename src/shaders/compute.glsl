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

//vec3 lightPos = vec3(1, 1, 1);
const int MAX_BOUNCES = 5;
const int NUM_TRIANGLES = 30;
const float FIREFLY_CLAMP = 5.0;

#include "structs.glsl"

QuadLight quadLight = QuadLight(
    vec3(-0.15, 0.49, -0.15),
    vec3(0.3, 0.0, 0.0),
    vec3(0.0, 0.0, 0.3),
    vec3(0.0, -1.0, 0.0),
    vec3(15.0, 15.0, 15.0)
);

layout(std430, binding = 1) buffer SphereBuffer { Sphere spheres[]; };
layout(std430, binding = 2) buffer MaterialBuffer { Material materials[]; };

layout(std430, binding = 4) buffer VertexBuffer { Vertex vertice[]; };
layout(std430, binding = 5) buffer IndexBuffer { int indices[]; };

layout(std430, binding = 6) buffer TriMats { int triMatIds[]; };
layout(std430, binding = 7) buffer NodesBuffer { BVHNode nodes[]; };

bool isLeaf(int nodeIndex) {
    BVHNode node = nodes[nodeIndex];

    return (node.childAIndex == 0 && node.childBIndex == 0);
}

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

    int nodeStack[32];
    int stackPtr = 0;

    nodeStack[stackPtr++] = 0;

    while (stackPtr > 0) {
        int currNodeIndex = nodeStack[--stackPtr];

        if(!hitNode(currNodeIndex, ray)) continue;

        if(isLeaf(currNodeIndex)) {
            int triangleIndex = nodes[currNodeIndex].triangleIndex;
            int triangleCount = nodes[currNodeIndex].triangleCount;

            for (int i = triangleIndex; i < triangleIndex + triangleCount; i++) {
                if (hitTriangle(i, ray, interval, hitr)) {
                    hasHit = true;
                    interval.tmax = hitr.t;
                } 
            } 
            
        } else {
            nodeStack[stackPtr++] = nodes[currNodeIndex].childAIndex;
            nodeStack[stackPtr++] = nodes[currNodeIndex].childBIndex;
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

            LightSample ls = sampleQuadLight(quadLight, texelCoord, 9976);

            vec3 toLight = ls.point - hitr.point;
            float distSq = dot(toLight, toLight);
            float dist = sqrt(distSq);

            vec3 w_i = toLight / dist;

            float cosThetaLight = max(dot(ls.normal, -w_i), 0.0);

            float nDotw_i = max(dot(n, w_i), 0.0);
            vec3 h = normalize(w_o + w_i);
          
            vec3 F = FresnelSchlick(h, w_o, F0);
            
            if (cosThetaLight > 0.0 && nDotw_i > 0.0) {
                float pdfSolidAng = ls.pdfArea * distSq / cosThetaLight;
                vec3 direct = fTotal(h, w_o, w_i, n, F, matr) * 
                    nDotw_i * ls.emission / pdfSolidAng;
                direct = min(direct, vec3(FIREFLY_CLAMP));

                Ray sray = Ray(hitr.point + 0.01 * n, toLight);
                if (!anyHit(sray)) {
                    radiance += throughput * direct;
                }
            }
           
            BSDFSample bsdf_sample = SampleBSDF(matr, texelCoord, F, n, w_o);
            w_i = bsdf_sample.w_i;
            float pdf_spec = bsdf_sample.pdf_spec;
            float pdf_diff = bsdf_sample.pdf_diff;
            float pdf_total = pdf_spec + pdf_diff;

            h = normalize(w_o + w_i);
            F = FresnelSchlick(h, w_o, F0);
            nDotw_i = max(dot(n, w_i), 0.0);
            
            vec3 indirect = fTotal(h, w_o, w_i, n, F, matr) * nDotw_i / pdf_total;
            indirect = min(indirect, vec3(FIREFLY_CLAMP));
            throughput *= indirect;

            initRay = Ray(hitr.point + 1e-3 * n, w_i);

            float continueProb = clamp(
                    max(throughput.r, max(throughput.g, throughput.b)), 0.05, 1.0);

            if (randTex(texelCoord, 5487) > continueProb) { break; }

            throughput /= continueProb;

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
    
    if (any(isnan(sampleColor)) || any(isinf(sampleColor)) )
        sampleColor = vec3(0.0);

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
