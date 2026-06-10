cbuffer UBO : register(b0, space1) {
    float4x4 transform : packoffset(c0);
};

struct input_t {
    float3 position : TEXCOORD0;
    float4 color    : TEXCOORD1;
};

struct output_t {
    float4 color    : TEXCOORD0;
    float4 position : SV_Position;
};

output_t main(input_t input)
{
    output_t output;

    output.color = input.color;
    output.position = mul(transform, float4(input.position, 1.0f));

    return output;
}
