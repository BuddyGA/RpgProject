#include "Common.hlsli"


// =============================================================================================== //
// VERTEX SHADER
// =============================================================================================== //
float4 VS_Main(float4 VertexPosition: POSITION) : SV_Position
{
	return mul(VertexPosition, ObjectTransforms[ObjectParameter.TransformIndex]);
}



// =============================================================================================== //
// GEOMETRY SHADER
// =============================================================================================== //
struct GeometryShaderOutput
{
	float4 SvPosition: SV_Position;
    uint RenderTargetIndex : SV_RenderTargetArrayIndex;
};



[maxvertexcount(18)] 
void GS_Main(triangle float4 ws_VertexPositions[3]: SV_Position, inout TriangleStream<GeometryShaderOutput> stream)
{
	for (uint rtIndex = 0; rtIndex < 6; ++rtIndex)
	{
        const RpgShaderView view = WorldData.Views[ObjectParameter.ViewIndex + rtIndex];
		
		for (int vtxIndex = 0; vtxIndex < 3; ++vtxIndex)
		{
            const float4 wsVertexPosition = ws_VertexPositions[vtxIndex];
			
			GeometryShaderOutput output;
            output.SvPosition = mul(wsVertexPosition, view.ViewProjectionMatrix);
            output.SvPosition.z = length(wsVertexPosition.xyz - view.WorldPosition.xyz) * output.SvPosition.w / view.FarClipZ;
            output.RenderTargetIndex = rtIndex;
			stream.Append(output);
		}
		
		stream.RestartStrip();
	}
}

