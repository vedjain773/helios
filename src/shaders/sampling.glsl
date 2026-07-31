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

