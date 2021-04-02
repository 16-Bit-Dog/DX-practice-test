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
    float4 pos : SV_POSITION; //assign color var first to apply color before vertex shifting
    //float4 position : SV_POSITION; //assign vertex pos 
    float3 norm: NORMAL;
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texl : TEXLINK;
    float4 posw : TEXCOORD1;
};

[maxvertexcount(6)] // this geo shader takes 1 vertex from current vertex buffer and makes 6 vertex triangle stream out of them - makes a fuzzy multiplication of vertex effect
//each tri stream has its own values based on the vertex submitted
void SimpleGeometryShader(point GSOutput input[1], inout TriangleStream<GSOutput> OutputStream)
{
    GSOutput gsout;

    gsout.pos = float4(input[0].pos.x + 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    gsout.posw = float4(input[0].pos.x + 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    gsout.norm = input[0].norm;
    gsout.tex = input[0].tex;
    gsout.texl = input[0].texl;
    OutputStream.Append(gsout);

    gsout.pos = float4(input[0].pos.x - 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    gsout.posw = float4(input[0].pos.x - 0.5, input[0].pos.y + 0.5, input[0].pos.z, input[0].pos.w);
    gsout.norm = input[0].norm;
    gsout.tex = input[0].tex;
    gsout.texl = input[0].texl;
    OutputStream.Append(gsout);

    gsout.pos = float4(input[0].pos.x - 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    gsout.posw = float4(input[0].pos.x - 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    gsout.norm = input[0].norm;
    gsout.tex = input[0].tex;
    gsout.texl = input[0].texl;
    OutputStream.Append(gsout);
    
    gsout.pos = float4(input[0].pos.x + 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    gsout.posw = float4(input[0].pos.x - 0.5, input[0].pos.y - 0.5, input[0].pos.z, input[0].pos.w);
    gsout.norm = input[0].norm;
    gsout.tex = input[0].tex;
    gsout.texl = input[0].texl;
    OutputStream.Append(gsout);

    
}
