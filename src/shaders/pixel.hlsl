cbuffer UBO : register(b0, space3)
{
    float plane_near;
    float plane_far;
};

struct output_ps
{
    float4 color : SV_Target0;
    float depth : SV_Depth;
};

float linearize_depth(float depth, float near, float far)
{
    float z = depth * 2.0 - 1.0;
    return ((2.0 * near * far) / (far + near - z * (far - near))) / far;
}

output_ps ps_main(float4 color : TEXCOORD0, float4 position : SV_Position)
{
    output_ps result;
    
    result.color = color;
    result.depth = linearize_depth(position.z, plane_near, plane_far);
    
    return result;
}
