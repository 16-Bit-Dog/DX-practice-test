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
}

[numthreads(1024, 1, 1)] //threads to launch inside compute shader
void SmoothMotionCompute(uint ids : SV_DispatchThreadID)
{
    uint id = ids.x;

    float3 posModData = OutputBuffer[id].position;

    /*
    * 
    *move object - static all x, y, then z
    * 
    float posModx = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[0]));
    float posMody = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[1]));
    float posModz = ((sin(ConstantUnsortedTypes[0][0]) / 100 + posModData[2]));
    */
    float time = ConstantUnsortedTypes[0][0];

    float posModx = (posModData[0]);
    float posMody = (posModData[1]);
    float posModz = (posModData[2]);
    
    uint rem = id % 3;

    posModx += (sin(2 * time) * (sin(time) / log(time))) / (100 * (rem + 1));
    posMody += (sin(2 * time) * (sin(time) / log(time))) / (100 * (rem + 1));
    posModz += (sin(2 * time) * (sin(time) / log(time))) / (100 * (rem + 1));

    //time to move every other vertex to simulate water - AN IMPORTANT THING TO KNOW IS THAT THE MODEL IS EVEN, so reminder is fine
    //if (id % 10 == 0) {
    //    //posModx += sin();
    //    posModx += (sin(2 * time) * (sin(time) / log(time))) / 100;
    //    posMody += (sin(2 * time) * (sin(time) / log(time))) / 100;
    //    posMody += (sin(2 * time) * (sin(time) / log(time))) / 100;

    //}
    //else if (id % 10 == 1) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 1900;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 1900;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 2100;
    //}

    //else if (id % 10 == 2) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 2100;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 2100;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 2200;
    //}

    //else if (id % 10 == 3) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 2500;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 2500;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 3000;
    //}

    //else if (id % 10 == 4) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 3000;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 3000;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 3500;
    //}

    //else if (id % 10 == 5) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 3500;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 3500;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 4000;
    //}

    //else if (id % 10 == 6) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 4000;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 4000;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 5300;
    //}

    //else if (id % 10 == 7) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 4500;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 4800;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 5500;
    //}

    //else if (id % 10 == 8) {
    //    //posModx += sin();
    //    posMody += sin(ConstantUnsortedTypes[0][0]) / 5700;
    //    posModx += sin(ConstantUnsortedTypes[0][0]) / 5700;
    //    posModz += cos(ConstantUnsortedTypes[0][0]) / 5800;
    //}


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
