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

struct GSPS_INPUT
{
    float4 position : SV_POSITION;
    float3 normal: NORMAL; //mainly for particles, so I don't care for normals too much with this
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texlink : TEXLINK;
    float4 PositionWS : TEXCOORD1;
};

[maxvertexcount(12)]
void SO(triangle GSPS_INPUT input[3], inout TriangleStream<GSPS_INPUT> TriStream)
{
    GSPS_INPUT output;
    float Explode = 0.f;
    //
    // Calculate the face normal
    //
    float3 faceEdgeA = input[1].position - input[0].position;
    float3 faceEdgeB = input[2].position - input[0].position;
    float3 faceNormal = normalize(cross(faceEdgeA, faceEdgeB));
    float3 ExplodeAmt = faceNormal * Explode;

    //
    // Calculate the face center
    //
    float3 centerPos = (input[0].position.xyz + input[1].position.xyz + input[2].position.xyz) / 3.0;
    float2 centerTex = (input[0].tex + input[1].tex + input[2].tex) / 3.0;
    centerPos += faceNormal * Explode;

    //
    // Output the pyramid
    //
    output.texlink = 0;
    
    for (int i = 0; i < 3; i++)
    {
        output.position = input[i].position + float4(ExplodeAmt, 0);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        output.normal = input[i].normal;
        output.tex = input[i].tex;
        output.PositionWS = output.position;
        TriStream.Append(output);

        int iNext = (i + 1) % 3;
        output.position = input[iNext].position + float4(ExplodeAmt, 0);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        output.normal = input[iNext].normal;
        output.tex = input[iNext].tex;
        output.PositionWS = output.position;
        TriStream.Append(output);

        output.position = float4(centerPos, 1) + float4(ExplodeAmt, 0);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        output.normal = faceNormal;
        output.tex = centerTex;
        output.PositionWS = output.position;
        TriStream.Append(output);

        TriStream.RestartStrip();
    }

    for (int i = 2; i >= 0; i--)
    {
        output.position = input[i].position + float4(ExplodeAmt, 0);
        output.position = mul(output.position, viewMatrix);
        output.position = mul(output.position, projectionMatrix);
        output.normal = -input[i].normal;
        output.tex = input[i].tex;
        output.PositionWS = output.position;
        TriStream.Append(output);
    }
    TriStream.RestartStrip();
}