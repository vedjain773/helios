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

