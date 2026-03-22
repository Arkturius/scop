/**
 * vertices.c
 */

#include <IG_engine.h>

Vertex	*vertices = NULL;
u32		*indices = NULL;

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
	static VkVertexInputAttributeDescription	attrs[3] =
	{
		{
			.binding	= 0,
			.location	= 0,
			.format		= VK_FORMAT_R32G32B32_SFLOAT,
			.offset		= struct_offset(Vertex, pos),
		},
		{
			.binding	= 0,
			.location	= 1,
			.format		= VK_FORMAT_R32G32B32_SFLOAT,
			.offset		= struct_offset(Vertex, color),
		},
		{
			.binding	= 0,
			.location	= 2,
			.format		= VK_FORMAT_R32G32_SFLOAT,
			.offset		= struct_offset(Vertex, tex),
		},
	};
	return (attrs);
}
