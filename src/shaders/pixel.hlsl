// cbuffer UBO : register(b0, space2)
// {
//     float plane_near;
//     float plane_far;
// };

// struct output_ps
// {
//     float4 color : SV_Target0;
//     float depth : SV_Depth;
// };
//
// float linearize_depth(float depth, float near, float far)
// {
//     float z = depth * 2.0 - 1.0;
//     return ((2.0 * near * far) / (far + near - z * (far - near))) / far;
// }

// output_ps ps_main(float4 color : TEXCOORD0, float4 position : SV_Position)
// float4 ps_main(float4 color : TEXCOORD0, float4 position : SV_Position) : SV_Target0
float4 ps_main(float4 color : TEXCOORD0) : SV_Target0
{
//     output_ps result;
//
//     result.color = color;
//     result.depth = linearize_depth(position.z, plane_near, plane_far);
//     result.color.r = 255.0f;
//     result.color.g = 0.0f;
//     result.color.b = 0.0f;
//     result.color.a = 255.0f;
//
//
//     return result;
    return color;
}
