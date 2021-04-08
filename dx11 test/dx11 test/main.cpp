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

    float time = ConstantUnsortedTypes[0][0];
    float RatioRMS = ConstantUnsortedTypes[0][1];
    float RealSound = ConstantUnsortedTypes[0][2];

    float posModx = (posModData[0]);
    float posMody = (posModData[1]);
    float posModz = (posModData[2]);
    
    uint rem = id % 3;

    if (true) {

        posModx += RealSound/10;
        posMody += RealSound/10;
        posModz += RealSound/10;

    }
    else {
        posModx -= 0.01;
        posMody -= 0.01;
        posModz -= 0.01;


    }
    if (posModx < -2 || posMody < -2 || posModz < -2) {
        posModx += 2;
        posMody += 2;
        posModz += 2;
    }

    //posModx += sin(2 * time) / (10 * (rem + 1));
    //posMody += sin(2 * time) / (10 * (rem + 1));
    //posModz += sin(2 * time) / (10 * (rem + 1));

    OutputBuffer[id].position = float3(posModx, posMody, posModz);
    OutputBuffer[id].normal = OutputBuffer[id].normal;
    OutputBuffer[id].tex = OutputBuffer[id].tex;
    OutputBuffer[id].texlink = OutputBuffer[id].texlink;

}
