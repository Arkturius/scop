static float2	positions[3] = float2[]
(
    float2( 0.0, -0.5),
    float2( 0.5,  0.5),
    float2(-0.5,  0.5)
);

static float3	colors[3] = float3[]
(
    float3(1.0, 0.0, 0.0),
    float3(0.0, 1.0, 0.0),
    float3(0.0, 0.0, 1.0),
);

struct vertex_output
{
    float3 color;
    float4 sv_position : SV_Position;
};

[shader("vertex")]
vertex_output
vert_main(uint id : SV_VertexID)
{
    vertex_output	output;

    output.sv_position	= float4(positions[id], 0.0, 1.0);
    output.color		= colors[id];

    return (output);
}

[shader("fragment")]
float4
frag_main(vertex_output inVert) : SV_Target
{
    float3	color;

	color = inVert.color;
    return float4(color, 1.0);
}
