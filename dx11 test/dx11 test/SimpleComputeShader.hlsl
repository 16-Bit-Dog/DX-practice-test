
RWTexture2D<unorm float4> OutputBuffer : register(u0);

[numthreads(8, 8, 1)]
void SimpleComputeShader(uint3 groupId : SV_GroupID,
    uint3 groupThreadId : SV_GroupThreadID,
    uint3 dispatchThreadId : SV_DispatchThreadID,
    uint groupIndex : SV_GroupIndex)
{
    unorm float4 w = (0.1, 0.1, 0.1, 0.1);
    //Set the structured buffer to weird stuff
    OutputBuffer[dispatchThreadId.xy] = OutputBuffer.Load(int2(dispatchThreadId.x, dispatchThreadId.y)) + w; // load - loads a pixel


}
