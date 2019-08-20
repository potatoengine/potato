float linearizeDepth(float depth, float nearZ, float farZ) {
    return nearZ * farZ / (farZ + depth * (nearZ - farZ));
}
