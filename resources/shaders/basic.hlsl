#include "common.hlsli"

//float4x4 WorldViewProjection;

float4 vertex_main(float4 inputPosition
                   : POSITION) : POSITION {
    //return mul(inputPosition, WorldViewProjection);
    return inputPosition;
}

float4 pixel_main(float4 inputPosition
                  : POSITION) : COLOR0 {
    return float4(inputPosition.xyz, 1);
}
