//D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ

cbuffer PerApplication : register(b0) //const buffer - for stuff needed once per program life time
{
    matrix projectionMatrix;
}

cbuffer PerFrame : register(b1) //const buffer object - for stuff updated every frame
{
    matrix viewMatrix;
}

cbuffer PerObject : register(b2) //const buffer - for stuff needed for every object
{
    matrix worldMatrix;
}

cbuffer PerTime : register(b3) //const buffer - for stuff needed once per program life time
{
    matrix time;
}

cbuffer PerLight1 : register(b4) //const buffer - for stuff needed once per program life time
{
    matrix lightS1;
}

struct GSOutput
{
    float4 pos : POSITION; //vertex attribute var 
};

[maxvertexcount(6)] // this geo shader takes 1 vertex from current vertex buffer and makes 6 vertex triangle stream out of them - makes a fuzzy multiplication of vertex effect
//by design all made output stream triangles follow the normal, color, and which texture to use symantic from the original input
void SimpleGeometryShader(point GSOutput input[1], inout TriangleStream<GSOutput> OutputStream)
{
    GSOutput gsout;

    gsout.pos = float4(input[0].pos.x + 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    OutputStream.Append(gsout);
    gsout.pos = float4(input[0].pos.x - 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    OutputStream.Append(gsout);
    gsout.pos = float4(input[0].pos.x - 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    OutputStream.Append(gsout);
    gsout.pos = float4(input[0].pos.x + 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    OutputStream.Append(gsout);

    
}
