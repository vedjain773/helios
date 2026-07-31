uint pcg_hash(uint seed) {
    uint state = seed * 747796405u + 2891336453u;
    uint word = ((state >> ((state >> 28u) + 4u)) ^ state) * 277803737u;
    return (word >> 22u) ^ word;
}

float rand(inout uint seed) {
    seed = pcg_hash(seed);
    return float(seed) / 4294967295.0;
}

float randTex(ivec2 texelCoord, int offset) {
    uint seed = pcg_hash(uint(texelCoord.x)) 
              ^ pcg_hash(uint(texelCoord.y) * 9781u)
              ^ pcg_hash(uint(frameCounter) * 6271u)
              ^ pcg_hash(uint(offset) * 26699u);
    return rand(seed);
}
