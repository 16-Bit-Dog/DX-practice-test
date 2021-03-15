//texture2D InputTex : register(t0);

struct AppData //used as demonstration of data
{
    float3 position : POSITION; //vertex attribute var    //12 bytes
    //4 - x
    //4 - y
    //4 - z

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

[numthreads(1024, 1, 1)] //threads to launch inside compute shader
void SmoothMotionCompute(uint ids : SV_DispatchThreadID)
{
    uint id = ids.x;

    float3 posModData = OutputBuffer[id].position;

    float posModx = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[0]));
    float posMody = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[1]));
    float posModz = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[2]));

    OutputBuffer[id].position = float3(posModx, posMody, posModz);
    OutputBuffer[id].normal = OutputBuffer[id].normal;
    OutputBuffer[id].tex = OutputBuffer[id].tex;
    OutputBuffer[id].texlink = OutputBuffer[id].texlink;

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
