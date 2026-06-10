Texture2D<float4> Texture : register(t0, space2);
SamplerState Sampler      : register(s0, space2);

struct output_t {
    float4 color : SV_Target;
};

output_t main(float2 uv : TEXCOORD0, float4 color : TEXCOORD1)
{
    output_t output;

    output.color = color * Texture.Sample(Sampler, uv.rg);

    return output;
}
