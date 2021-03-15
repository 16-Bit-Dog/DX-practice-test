//texture2D InputTex : register(t0);
RWTexture2D<unorm float4> OutputBuffer : register(u0); //0 is always render targets

cbuffer PerTime : register(b3) //const buffer - for stuff needed once per program life time - group shared for many thread concurrent acsess
{
    matrix ConstantUnsortedTypes; //random DATA
    //[0][0] is time
}

[numthreads(32, 32, 1)] //32 * 32 means 1024 pixels --> add z if larger is used - but for now this is what I use - will later
void SimpleComputeShader(uint3 DispatchThreadID : SV_DispatchThreadID)
{

    float r = OutputBuffer[DispatchThreadID.xy].r - float(sin(ConstantUnsortedTypes[0][0]) / 300);
    float g = OutputBuffer[DispatchThreadID.xy].g + float(sin(ConstantUnsortedTypes[0][0]) / 300);
    float b = OutputBuffer[DispatchThreadID.xy].b;
    float w = OutputBuffer[DispatchThreadID.xy].w;

    OutputBuffer[DispatchThreadID.xy] = float4(r, g, b, w);
}
