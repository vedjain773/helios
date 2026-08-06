#ifndef VECTOR_H
#define VECTOR_H

struct Vec3 {
    float x;
    float y;
    float z;
};

struct Vec4 {
    float x;
    float y;
    float z;
    float w;
};

inline Vec3 operator+(const Vec3 &a, const Vec3 &b) {
    return Vec3 {
        a.x + b.x,
        a.y + b.y,
        a.z + b.z
    };
}

#endif
