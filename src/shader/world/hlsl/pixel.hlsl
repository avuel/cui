struct output_t {
    float4 color : SV_Target;
};

output_t main(float4 color : TEXCOORD0)
{
    output_t output;

    output.color = color;

    return output;
}
