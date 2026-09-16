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

void sort2(inout float arr[2]) {
    if (arr[0] > arr[1]) {
        float tmp = arr[0];
        arr[0] = arr[1];
        arr[1] = tmp;
    }
}

bool hitNode(int nodeId, Ray ray) {
    vec3 boundsMin = nodes[nodeId].boundsMin;
    vec3 boundsMax = nodes[nodeId].boundsMax;

    vec3 raySrc = ray.source;
    vec3 rayDir = ray.direction;

    float xInter[2] = float[2]( 
        (raySrc.x - boundsMin.x) /rayDir.x, (raySrc.x - boundsMax.x) /rayDir.x
    );

    float yInter[2] = float[2](
        (raySrc.y - boundsMin.y) /rayDir.y, (raySrc.y - boundsMax.y) /rayDir.y
    );

    float zInter[2] = float[2](
        (raySrc.z - boundsMin.z) /rayDir.z, (raySrc.z - boundsMax.z) /rayDir.z
    );

    sort2(xInter);
    sort2(yInter);
    sort2(zInter);

    float near = max(max(xInter[0], yInter[0]), zInter[0]);
    float far = min(min(xInter[1], yInter[1]), zInter[1]);
    
    if (isnan(near) || isnan(far)) return false;

    return near <= far; 
}

bool hitTriangle(int triId, Ray ray, Interval interval, inout HitRecord hitr) {
    
    vec3 a = vertice[indices[triId * 3]].position;
    vec3 b = vertice[indices[triId * 3 + 1]].position;
    vec3 c = vertice[indices[triId * 3 + 2]].position;

    Triangle tri = Triangle(a, b, c, triMatIds[triId]);

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

