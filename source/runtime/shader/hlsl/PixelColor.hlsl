
struct PixelShaderInput
{
    float4 SvPosition : SV_POSITION;
    float4 Color : COLOR0;
};


float4 PS_Main(PixelShaderInput input) : SV_Target
{
    return input.Color;
}
