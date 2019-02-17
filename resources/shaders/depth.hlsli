float linearizeDepth(float depth, float nearZ, float farZ) {
    return (2 * nearZ) / (farZ + nearZ - depth * (farZ - nearZ));
}
