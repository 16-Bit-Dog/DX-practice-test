
RWTexture2D<unorm float4> OutputBuffer : register(u0); //0 is always render targets

[numthreads(32, 32, 1)]
void SimpleComputeShader(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    OutputBuffer[DispatchThreadID.xy] = float4(0.f, 1.f, 0.f, 0.f);
}
