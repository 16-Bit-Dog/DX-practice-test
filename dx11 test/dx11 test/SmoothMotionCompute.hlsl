// WORK IN PROGRESS

//texture2D InputTex : register(t0);

struct vertexLayout {

    float x;
    float y;
    float z;
    float w; 

};

RWBuffer<vertexLayout> OutputBuffer : register(u0); //1 is the vertex buffer 

cbuffer PerTime : register(b3) //const buffer - for stuff needed once per program life time - group shared for many thread concurrent acsess
{
    matrix ConstantUnsortedTypes; //random DATA
    //[0][0] is time
}

[numthreads(32, 32,1)] //32 * 32 means 1024 pixels --> add z if larger is used - but for now this is what I use - will later
void SmoothMotionCompute(uint3 DispatchThreadID : SV_DispatchThreadID)
{
    uint index = DispatchThreadID.x * DispatchThreadID.y;

    vertexLayout vertexC;

    vertexC.x = 0.f;
    vertexC.y = 0.f;
    vertexC.z = 0.f;
    vertexC.w = 0.f;


    OutputBuffer[index] = vertexC;

    
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
