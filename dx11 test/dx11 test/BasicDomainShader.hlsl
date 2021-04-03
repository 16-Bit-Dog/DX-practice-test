

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

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

struct HSI //hull shader data to be passed to pixel shader - only usful for NON GEO SHADER STUFF - GEO SHADER AND HULL ARE NOT COMPATIABLE
{
    float4 position : SV_POSITION; //assign color var first to apply color before vertex shifting
    float3 normal: NORMAL;
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texlink : TEXLINK;
    float4 PositionWS : TEXCOORD1;
};

[domain("tri")]

HSI BasicDomainShader(ConstantOutputType input, float3 uvwCoord : SV_DomainLocation, const OutputPatch<HSI, 3> patch)
{
    float3 vertexPosition;
    HSI output;


    // Determine the position of the new vertex.
    vertexPosition = uvwCoord.x * patch[0].position + uvwCoord.y * patch[1].position + uvwCoord.z * patch[2].position;

    // Calculate the position of the new vertex against the world, view, and projection matrices.
    output.position = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.position = mul(output.position, viewMatrix);
    output.position = mul(output.position, projectionMatrix);

    // Send the input color into the pixel shader.
    output.tex = patch[0].tex;

    //same as above for rest:

    output.normal = patch[0].normal;
    output.texlink = patch[0].texlink;
    output.PositionWS = patch[0].PositionWS;



    return output;
}