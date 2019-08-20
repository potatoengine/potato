// Copyright (C) 2019 Sean Middleditch, all rights reserverd.

cbuffer frameData : register(b0) {
    uint frameNumber;
    float lastFrameTimeDelta;
    double timeStamp;
};

cbuffer cameraData : register(b1) {
    float4x4 worldViewProjection;
    float4x4 worldView;
    float4x4 viewProjection;
    float3 cameraPosition;
    float nearZ;
    float farZ;
};

cbuffer modelData : register(b2) {
    float4x4 modelWorld;
    float4x4 worldModel;
}

static const float PI = 3.14159265f;
