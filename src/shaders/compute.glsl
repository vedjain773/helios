#version 450 core

layout (local_size_x = 16, local_size_y = 16, local_size_z = 1) in;
layout(rgba32f, binding = 0) uniform image2D imgOutput;

const float PI = 3.14159265359;

uniform vec3 center;      //= vec3(0.0, 0.0, 0.0);
uniform vec3 pixel00Loc;  //= vec3(-0.551594, -0.413523, -1.0);
uniform vec3 pixelDeltaU; //= vec3(0.00138071, 0.0, 0.0);
uniform vec3 pixelDeltaV; //= vec3(0.0, 0.00138071, 0.0);

uniform float metallic;
uniform float roughness;

vec3 lightPos = vec3(1, 1, 1);

struct Ray {
    vec3 source;
    vec3 direction;
};

struct Material {
    vec3 albedo;
    float metallic;
    float roughness;
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

vec3 fSpecular(vec3 h, vec3 w_o, vec3 w_i, vec3 n, float alphaSq, vec3 F) {
    float nDotw_o = max(dot(n, w_o), 0.0);
    float nDotw_i = max(dot(n, w_i), 0.0);
    
    float D = DistributionGGX(n, h, alphaSq);
    float G = GeometrySmith(nDotw_o, nDotw_i, alphaSq);

    return (D * G * F) / (4 * nDotw_o * nDotw_i + 1e-3);
}

vec3 fDiffuse(vec3 albedo, vec3 F, float metallic) {
    vec3 kD = 1 - F;
    kD *= 1 - metallic;
    
    return albedo * kD / PI; 
}

vec3 fTotal(vec3 h, vec3 w_o, vec3 w_i, vec3 n, vec3 F0, Material mat) {
    vec3 albedo = mat.albedo;
    float metallic = mat.metallic;
    float alpha = mat.roughness * mat.roughness;
    float alphaSq = alpha * alpha;

    vec3 F = FresnelSchlick(h, w_o, F0);

    vec3 fspec = fSpecular(h, w_o, w_i, n, alphaSq, F);
    vec3 fdiff = fDiffuse(albedo, F, metallic);

    return fspec + fdiff;
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
    hitr.normal = (hitr.point - center) / radius;

    return true;
}

Sphere sph1 = Sphere(vec3(0.0, 0.0, -1.0), 0.5);

vec3 getColor(Ray ray) {
    HitRecord hitr;
    Material mat1 = Material(vec3(1.0, 0.0, 0.0), metallic, roughness); 
    
    vec3 F0 = vec3(0.04);
    F0 = mix(F0, mat1.albedo, mat1.metallic);
    vec3 radiance = vec3(5.0);
    
    if (hitSphere(sph1, ray, Interval(0.0, 1000.0), hitr)) {
        vec3 n = hitr.normal;
        vec3 w_o = normalize(center - hitr.point);
        vec3 w_i = normalize(lightPos - hitr.point);
        
        float nDotw_i = max(dot(n, w_i), 0.0);
        
        vec3 h = normalize(w_o + w_i);

        vec3 color = fTotal(h, w_o, w_i, n, F0, mat1) * radiance * nDotw_i;
        
        color = color / (color + vec3(1.0));
        color = pow(color, vec3(1.0 / 2.2));

        return color;
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
