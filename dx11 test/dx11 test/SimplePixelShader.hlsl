

//need shader 4.0 NON dx9 is a requirment since dynamic indexing is used here
Texture2D shaderTexture[100] : register(t0); //some to do's with registers to select textures...
SamplerState SampleType : register(s0);

cbuffer PerLight1 : register(b4) //const buffer - for stuff needed once per program life time
{
    matrix lightS1;
};


struct PixelShaderInput //pixel shader input struct
{
    float4 position : SV_POSITION; //assign color var first to apply color before vertex shifting
    //float4 position : SV_POSITION; //assign vertex pos 
    float3 normal: NORMAL;
    float2 tex : TEXCOORD0; // I do not need normals to send to the SimplePixelShader for now, so... 
    uint texlink : TEXLINK;
    float4 PositionWS : TEXCOORD1;
};

float4 SimplePixelShader(PixelShaderInput IN) : SV_TARGET{
    //PixelShaderInput OUT;
//    IN.color = float4(100,50,30,0);
    //OUT.color = float4(0, 1.0f, 0, 1.0f);

    float3 normals = normalize(IN.normal);

    float4 textureColor;

    switch (IN.texlink) { // I am intentionally avoiding texture arrays, and rather setup a case-break statment for now --> NOT EFFICENT FOR OBVIOUS REASONS: technically a bindless texture array with dynamic indexing *may* work, but it requires dx 11.3 which may be a issue?

    case 0:

        textureColor = shaderTexture[0].Sample(SampleType, IN.tex); //later I should make a funny meme worthy filter; and some texture interpolator for fun stuff
        break;
    case 1:

        textureColor = shaderTexture[1].Sample(SampleType, IN.tex);
        break;
    case 2:

        textureColor = shaderTexture[2].Sample(SampleType, IN.tex);
        break;

    default:
        textureColor = shaderTexture[3].Sample(SampleType, IN.tex);
        break;

    };

    float3 finalColor;
    finalColor = textureColor * lightS1[1];
    //finalColor = textureColor * float4(lightS1[1][0], lightS1[1][1], lightS1[1][2], lightS1[1][3]);
    finalColor += saturate(dot(float3(lightS1[0][0], lightS1[0][1], lightS1[0][2]), normals) * lightS1[2] * textureColor);

    //return textureColor; //return lone color

    finalColor[0] /= 1.2;

    return float4(finalColor, textureColor[3]); //color with light

}
