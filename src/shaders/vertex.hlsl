cbuffer UBO : register(b0, space1)
{
    float4x4 transform : packoffset(c0);
};

struct vs_input {
    float3 position : TEXCOORD0;
    float4 color : TEXCOORD1;
};

struct vs_output {
    float4 color : TEXCOORD0;
    float4 position : SV_Position;
};

vs_output vs_main(vs_input input)
{
    vs_output output;

    output.color = input.color;
    output.position = mul(transform, float4(input.position, 1.0f));

    return output;
}
