#version 450 core

layout (local_size_x = 1, local_size_y = 1, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

uniform vec3 center;      //= vec3(0.0, 0.0, 0.0);
uniform vec3 pixel00Loc;  //= vec3(-0.551594, -0.413523, -1.0);
uniform vec3 pixelDeltaU; //= vec3(0.00138071, 0.0, 0.0);
uniform vec3 pixelDeltaV; //= vec3(0.0, 0.00138071, 0.0);

vec3 lightPos = vec3(-0.577, -0.577, -0.577);

struct Ray {
    vec3 source;
    vec3 direction;
};

struct HitRecord {
    vec3 point;
    vec3 normal;
    float t;
};

struct Interval {
    float tmin;
    float tmax;
};

struct Sphere {
    vec3 center;
    float radius;
};

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
    float root = (-b + sqrtDisc) / (2 * a);

    if (!isInsideInterval(interval, root)) {
        root = (-b - sqrtDisc) / (2 * a);

        if (!isInsideInterval(interval, root))
            return false;
    } 

    hitr.t = root;
    hitr.point = ray.source + root * ray.direction;
    hitr.normal = (hitr.point - center) / radius;

    return true;
}

Sphere sph1 = Sphere(vec3(0.0, 0.0, -1.0), 0.5);

vec3 getColor(Ray ray) {
   HitRecord hitr;

    if (hitSphere(sph1, ray, Interval(0.0, 1000.0), hitr)) {
       return vec3(1.0, 0.0, 0.0); 
    }

    return vec3(0.0, 0.0, 0.0); 
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
