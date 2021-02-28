//need shader 4.0 NON dx9 is a requirment since dynamic indexing is used here
Texture2D shaderTexture[100] : register(t0); //some to do's with registers to select textures...
SamplerState SampleType : register(s0);

struct PixelShaderInput //pixel shader input struct
{
    float4 position  : SV_POSITION;//take in color
    float3 normal: NORMAL;
    float2 tex : TEXCOORD;
    uint texlink : TEXLINK;
};

float4 SimplePixelShader(PixelShaderInput IN) : SV_TARGET
{
    //PixelShaderInput OUT;
//    IN.color = float4(100,50,30,0);
    //OUT.color = float4(0, 1.0f, 0, 1.0f);

    float3 lightDirection = normalize(float3(1, -1, 0));

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



    float lightMagnitude = 0.8f * saturate(dot(IN.normal, -lightDirection)) - 0.2f;

    return textureColor; //return unchanged color since I am not doing the light shader yet*
}
