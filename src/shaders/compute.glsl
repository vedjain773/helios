#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 center;      //= vec3(0.0, 0.0, 0.0);
uniform vec3 pixel00Loc;  //= vec3(-0.551594, -0.413523, -1.0);
uniform vec3 pixelDeltaU; //= vec3(0.00138071, 0.0, 0.0);
uniform vec3 pixelDeltaV; //= vec3(0.0, 0.00138071, 0.0);

struct Ray {
    vec3 source;
    vec3 direction;
};

bool hitSphere(vec3 center, float radius, Ray ray) {
    vec3 oc = center - ray.source;
    float a = dot(ray.direction, ray.direction);
    float b = -2.0 * dot(ray.direction, oc);
    float c = dot(oc, oc) - radius*radius;
    float discriminant = b*b - 4*a*c;
    return (discriminant >= 0);
}

vec3 getColor(Ray ray) {

    if (hitSphere(vec3(0.0, 0.0, -1.0), 0.5, ray)) {
        return vec3(1.0, 0.0, 0.0);
    }

    vec3 unit_dir = normalize(ray.direction);
    float t = (unit_dir.y + 1.0) * 0.5;
    vec3 result = mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t);
    return result;
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
    vec4 value = vec4(0.0, 0.0, 0.0, 1.0);
    ivec2 texelCoord = ivec2(gl_GlobalInvocationID.xy);
   
    Ray ray = genRay(texelCoord);
    value.xyz = getColor(ray); 

    imageStore(imgOutput, texelCoord, value);
}
