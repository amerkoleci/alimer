struct ComputeData
{
    float time;
    float width;
    float height;
    float padding;
};

ConstantBuffer<ComputeData> data : register(b0);

RWTexture2D<float4> Output : register(u0);

[numthreads(8, 8, 1)]
void computeMain(uint3 threadID : SV_DispatchThreadID)
{
    //float2 size = float2(data.width, data.height);
    //float2 uv = threadID.xy / size + 0.5 / size;
    //Output[threadID.xy] = float4(uv, 0.5 + 0.5 * sin(data.time), 1.0);
}
