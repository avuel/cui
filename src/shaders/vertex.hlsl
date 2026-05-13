cbuffer UBO : register(b0, space1)
{
    float4x4 transform : packoffset(c0);
};

struct input_vs
{
    float3 position : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct output_vs
{
    float4 color : TEXCOORD0;
    float4 position : SV_Position;
};

output_vs vs_main(input_vs input)
{
    output_vs output;

    output.color = input.color;
    output.position = mul(transform, float4(input.position, 1.0f));

    return output;
}
