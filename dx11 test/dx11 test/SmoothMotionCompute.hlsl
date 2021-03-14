//texture2D InputTex : register(t0);

struct AppData //used as demonstration of data
{
    float3 position : POSITION; //vertex attribute var    //12 bytes
    //4
    //4
    //4

    float3 normal: NORMAL; // normal attribute var        //12 bytes
    float2 tex : TEXCOORD; // texture coordinate          //8 bytes
    int texlink : TEXLINK; //4 bytes
};

RWStructuredBuffer < AppData > OutputBuffer : register(u0); //1 is the vertex buffer 

cbuffer PerTime : register(b3) //const buffer - for stuff needed once per program life time - group shared for many thread concurrent acsess
{
    matrix ConstantUnsortedTypes; //random DATA
    //[0][0] is time
}

[numthreads(32, 32, 1)] //32 * 32 means 1024 pixels --> add z if larger is used - but for now this is what I use - will later
void SmoothMotionCompute(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint index = DispatchThreadID.x * DispatchThreadID.y;

    if (index < 100) {
        OutputBuffer[index].position = float3(sin(ConstantUnsortedTypes[0][0]), sin(ConstantUnsortedTypes[0][0]), sin(ConstantUnsortedTypes[0][0]));
    }
    OutputBuffer[index].normal = OutputBuffer[index].normal;
    OutputBuffer[index].tex = OutputBuffer[index].tex;
    OutputBuffer[index].texlink = OutputBuffer[index].texlink;

//    vertexC.position = float3(0.f, 0.f, 0.f);
//    vertexC.normal = OutputBuffer[index].normal;
//    vertexC.tex = OutputBuffer[index].tex;
//    vertexC.texlink = OutputBuffer[index].texlink;



  /*  uint i = (index * 36) - 36;

        float3 in_position = asfloat(OutputBuffer.Load3(i));
        float3 in_color = asfloat(OutputBuffer.Load3(i + 12));
        float2 in_tex = asfloat(OutputBuffer.Load2(i + 24));
        float in_texlink = asfloat(OutputBuffer.Load(i + 32));
        in_position[0] = 0.f;
        in_position[1] = 0.f;
        in_position[2] = 0.f;

        in_color[0] = 0.f;
        in_color[1] = 0.f;
        in_color[2] = 0.f;

        in_tex[0] = 0.f;
        in_tex[1] = 0.f;

        in_texlink = 0.f;

        OutputBuffer.Store3(i, asuint(in_position));
        OutputBuffer.Store3(i+12, asuint(in_color));
        OutputBuffer.Store2(i+24, asuint(in_tex));
        OutputBuffer.Store(i+32, asuint(in_texlink));

    
    /*
    tmp[0] = 0.0f;
    tmp[1] = 0.0f;
    tmp[2] = 0.0f;
    tmp[3] = 0.0f;
    */
    
    //float r = OutputBuffer[DispatchThreadID.xy].r + float(sin(ConstantUnsortedTypes[0][0]) / 100);
    //float g = OutputBuffer[DispatchThreadID.xy].g + float(sin(ConstantUnsortedTypes[0][0]) / 100);
    //float b = OutputBuffer[DispatchThreadID.xy].b;
    //float w = OutputBuffer[DispatchThreadID.xy].w;


}
