cbuffer UBO : register(b0, space1) {
    float4x4 transform : packoffset(c0);
};

struct input_t {
    float2 position  : TEXCOORD0;
    float2 uv        : TEXCOORD1;
    float4 color     : TEXCOORD2;
};

struct output_t {
    float2 uv       : TEXCOORD0;
    float4 color    : TEXCOORD1;
    float4 position : SV_Position;
};

output_t main(input_t input)
{
    output_t output;

    output.position = mul(transform, float4(input.position, 0.0f, 1.0f));
    output.uv       = input.uv;
    output.color    = input.color;

    return output;
}
