Texture2D shaderTexture; //some to do's with registers to select textures...
SamplerState SampleType;

struct PixelShaderInput //pixel shader input struct
{
    float4 position  : SV_POSITION;//take in color
    float2 tex : TEXCOORD;
};

float4 SimplePixelShader(PixelShaderInput IN) : SV_TARGET
{
    //PixelShaderInput OUT;
//    IN.color = float4(100,50,30,0);
    //OUT.color = float4(0, 1.0f, 0, 1.0f);

    float4 textureColor;

    textureColor = shaderTexture.Sample(SampleType, IN.tex); //later I should make a funny meme worthy filter; and some texture interpolator for fun stuff

    return textureColor; //return unchanged color
}