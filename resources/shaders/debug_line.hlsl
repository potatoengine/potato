#include "common.hlsli"
#include "depth.hlsli"

struct VS_Input {
    float3 position : POSITION;
    float4 color : COLOR;
};

struct VS_Output {
    float4 position : SV_Position;
    float4 color : COLOR;
};

VS_Output vertex_main(VS_Input input) {
    VS_Output output;
    output.position = float4(input.position, 1);
    output.position = mul(output.position, worldView);
    output.position = mul(output.position, viewProjection);
    output.color = input.color;
    return output;
}

float4 pixel_main(VS_Output input) : SV_Target {
    float depth = 1 - linearizeDepth(input.position.z / input.position.w, 2, 4000) / 4000;
    //depth = clamp(depth * 100, 0.1, 1);
    return input.color * float4(1, 1, 1, depth);
}
