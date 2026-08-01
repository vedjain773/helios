float DistributionGGX(vec3 n, vec3 h, float alphaSq) {
    //alpha -> roughness ^ 2
    //h -> normalize(w_o + w_i)

    float nDoth = dot(n, h);
    float nDoth_sq = nDoth * nDoth;
    float denominator = nDoth_sq * (alphaSq - 1) + 1;
    
    denominator = PI * denominator * denominator;

    return alphaSq / denominator;
}

float GeometrySmithGGX(float nDotv, float alphaSq) {
    float numerator = 2 * nDotv;
    float denominator = nDotv + sqrt(alphaSq + (1 - alphaSq) * nDotv * nDotv);

    return numerator / denominator;
}

float GeometrySmith(float nDotw_o, float nDotw_i, float alphaSq) {
    float ggx1 = GeometrySmithGGX(nDotw_o, alphaSq);
    float ggx2 = GeometrySmithGGX(nDotw_i, alphaSq);

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

