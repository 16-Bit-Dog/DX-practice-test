////////NOT WORKING PLACE HOLDER

struct GeoShaderInput //pixel shader input struct
{
	float4 position  : SV_POSITION;//take in color

};

[maxvertexcount(6)]
void SimpleGeometryShader(
	line float4 inputP[2] : SV_POSITION, //take in triangle from the inputted vertex buffer - one by 1
//	triangle float4 inputN[3] : NORMAL,
//	triangle float4 inputC[3] : TEXCOORD,

	inout LineStream< GeoShaderInput > output //this writes to the vertex buffer
)
{
	//FIX VERTEX SETUP
	for (uint i = 0; i < 2; i++)
	{
		GeoShaderInput outputOGS;
		outputOGS.position = float4(1,1,1,1);
		
		output.Append(outputOGS); //write to vertex buffer - I can use this to double/add verticies attached to this object
	}
	output.RestartStrip();


}