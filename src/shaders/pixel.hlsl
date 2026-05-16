struct output_ps
{
    float4 color : SV_Target;
};

output_ps ps_main(float4 color : TEXCOORD0)
{
    output_ps result;

    result.color = color;

    return result;
}
