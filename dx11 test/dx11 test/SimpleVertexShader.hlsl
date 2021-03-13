

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

struct AppData //vars in here are passed from application to vertex shader
{
    float3 position : POSITION; //vertex attribute var 
    float3 normal: NORMAL; // normal attribute var
    float2 tex : TEXCOORD; // texture coordinate
    int texlink : TEXLINK;
};

struct VertexShaderOutput //vertex shader data to be passed to pixel shader
{
    float4 position : SV_POSITION; //assign color var first to apply color before vertex shifting
    //float4 position : SV_POSITION; //assign vertex pos 
    float3 normal: NORMAL;
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texlink : TEXLINK;
    float4 PositionWS : TEXCOORD1;
};

VertexShaderOutput SimpleVertexShader(AppData IN) //entry point for vertex shader program - returns VertexShaderOutput struct for pixel shader, takes in appData struct
{   
    VertexShaderOutput OUT;

    

    matrix mvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix)); //-model to view to projection- is computed by doing projectionMatrix*(viewMatrix*worldMatrix)
    
    OUT.position = mul(mvp, float4(IN.position, 1.0f)); //OUT.position stores vertex pos made into float4 to be passed to pixel shader
    OUT.PositionWS = mul(worldMatrix, float4(IN.position, 1.0f));
    OUT.tex = IN.tex;
    OUT.normal = mul(mvp, IN.normal);//mul(mvp, float4(IN.normal, 1.0f));
    OUT.normal = normalize(OUT.normal);

    OUT.texlink = IN.texlink;

    return OUT; //output will be going to pixel shader
}

