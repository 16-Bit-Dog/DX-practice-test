Texture2D shaderTexture : register(t0); //some to do's with registers to select textures...
SamplerState SampleType : register(s0);

struct PixelShaderInput //pixel shader input struct
{
    float4 position  : SV_POSITION;//take in color
    float3 normal: NORMAL;
    float2 tex : TEXCOORD;
};

float4 SimplePixelShader(PixelShaderInput IN) : SV_TARGET
{
    //PixelShaderInput OUT;
//    IN.color = float4(100,50,30,0);
    //OUT.color = float4(0, 1.0f, 0, 1.0f);

    float3 lightDirection = normalize(float3(1, -1, 0));

    float4 textureColor;

    textureColor = shaderTexture.Sample(SampleType, IN.tex); //later I should make a funny meme worthy filter; and some texture interpolator for fun stuff

    float lightMagnitude = 0.8f * saturate(dot(IN.normal, -lightDirection)) - 0.2f;

    return textureColor; //return unchanged color since I am not doing the light shader yet*
}
