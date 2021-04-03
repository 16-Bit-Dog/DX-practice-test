//Hull input
struct HSI //vertex shader data to be passed to pixel shader
{
    float4 position : SV_POSITION; //assign color var first to apply color before vertex shifting
    //float4 position : SV_POSITION; //assign vertex pos 
    float3 normal: NORMAL;
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texlink : TEXLINK;
    float4 PositionWS : TEXCOORD1;
};

/*
cbuffer TessellationBuffer
{
    float tessellationAmount;
    float3 padding;
};*/

struct ConstantOutputType
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};



ConstantOutputType ColorPatchConstantFunction(InputPatch<HSI, 3> inputPatch, uint patchId : SV_PrimitiveID) //patch func
{
    ConstantOutputType output;

    float tessellationAmount = 1;

    // Set the tessellation factors for the three edges of the triangle.
    output.edges[0] = tessellationAmount;
    output.edges[1] = tessellationAmount;
    output.edges[2] = tessellationAmount;

    // Set the tessellation factor for tessallating inside the triangle.
    output.inside = tessellationAmount;

    return output;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)] 
[patchconstantfunc("ColorPatchConstantFunction")] //patch func
//the hull shader
HSI BasicHullShader(InputPatch<HSI, 3> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    HSI output;


    // Set the position for this control point as the output position. - same for rest of elements
    output.position = patch[pointId].position;
    output.normal = patch[pointId].normal;
    output.tex = patch[pointId].tex;
    output.texlink = patch[pointId].texlink;
    output.PositionWS = patch[pointId].PositionWS;
    
    
    return output;
}
