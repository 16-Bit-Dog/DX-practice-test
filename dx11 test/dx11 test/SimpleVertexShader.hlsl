

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

struct AppData //vars in here are passed from application to vertex shader
{
    float3 position : POSITION; //vertex attribute var 
    float3 normal: NORMAL; // normal attribute var
    float2 tex : TEXCOORD; // texture coordinate
};

struct VertexShaderOutput //vertex shader data to be passed to pixel shader
{
    float4 position : SV_POSITION; //assign color var first to apply color before vertex shifting
    //float4 position : SV_POSITION; //assign vertex pos 
    float2 tex : TEXCOORD; // I do not need normals to send to the SimplePixelShader for now, so... yeah
};

VertexShaderOutput SimpleVertexShader(AppData IN) //entry point for vertex shader program - returns VertexShaderOutput struct for pixel shader, takes in appData struct
{
    VertexShaderOutput OUT;

    matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix)); //-model to view to projection- is computed by doing projectionMatrix*(viewMatrix*worldMatrix)
    OUT.position = mul(mvp, float4(IN.position, 1.0f)); //OUT.position stores vertex pos made into float4 to be passed to pixel shader
    //OUT.color = float4(IN.color, 1.0f); //float4 is what we use for a pixel shader variable - so we declare a OUT.var for color in this sense (1.0 means we send the whole value, un modified - 0.9 would send faactor of 9 value
    OUT.tex = IN.tex;

    return OUT; //output will be going to pixel shader
}