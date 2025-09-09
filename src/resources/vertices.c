/**
 * vertices.c
 */

#include <IG_engine.h>

const Vertex	vertices[4] =
{
    { .pos = {-0.5f, -0.5f}, .color = {1.0f, 0.0f, 0.0f} },
    { .pos = { 0.5f, -0.5f}, .color = {0.0f, 1.0f, 0.0f} },
    { .pos = { 0.5f,  0.5f}, .color = {0.0f, 0.0f, 1.0f} },
    { .pos = {-0.5f,  0.5f}, .color = {1.0f, 1.0f, 1.0f} }
};

const u32	indices[6] =
{
	0, 1, 2, 2, 3, 0
};

VkVertexInputBindingDescription
IG_vk_vertex_get_binding(void)
{
	return (const VkVertexInputBindingDescription)
	{
		.binding	= 0,
		.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX,
		.stride		= sizeof(Vertex),
	};
}

VkVertexInputAttributeDescription
*IG_vk_vertex_get_attributes(void)
{
	static VkVertexInputAttributeDescription	attrs[2] =
	{
		{
			.binding	= 0,
			.location	= 0,
			.format		= VK_FORMAT_R32G32_SFLOAT,
			.offset		= struct_offset(Vertex, pos),
		},
		{
			.binding	= 0,
			.location	= 1,
			.format		= VK_FORMAT_R32G32B32_SFLOAT,
			.offset		= struct_offset(Vertex, color),
		},
	};
	return (attrs);
}
