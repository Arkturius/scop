struct vs_input
{
	float3 pos;
	float3 color;
	float2 tex;
}

struct uniform_buffer
{
	float4x4	model;
	float4x4	view;
	float4x4	proj;
};

ConstantBuffer<uniform_buffer>	ubo;

struct vs_output
{
    float4 pos		: SV_Position;
	float3 color;
	float2 tex;
};

[shader("vertex")]
vs_output
vert_main(vs_input input)
{
    vs_output	output;

	output.pos = mul(ubo.proj, mul(ubo.view, mul(ubo.model, float4(input.pos, 1.0))));
    output.color = input.color;
	output.tex = input.tex;

    return (output);
}

Sampler2D	texture;

[shader("fragment")]
float4
frag_main(vs_output input) : SV_Target
{
    return texture.Sample(input.tex);
}
