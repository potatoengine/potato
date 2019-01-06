float4x4 WorldViewProjection;

float4 vertex_main(float4 inputPosition : POSITION) : POSITION {
    return mul(inputPosition, WorldViewProjection);
}

float4 pixel_main() : COLOR0 {
    return float4(1, 0, 0, 1);
}
