struct vs_input
{
	float2 pos;
	float3 color;
}

struct vs_output
{
    float4 pos		: SV_Position;
	float3 color;
};

[shader("vertex")]
vs_output
vert_main(vs_input input)
{
    vs_output	output;

    output.pos		= float4(input.pos, 0.0, 1.0);
    output.color	= float3(input.pos, input.color.z);

    return (output);
}

[shader("fragment")]
float4
frag_main(vs_output input) : SV_Target
{
    return float4(input.color, 1.0);
}
