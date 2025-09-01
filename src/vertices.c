/**
 * vertices.c
 */

#include "types.h"
#include <scop.h>
#include <geometry.h>
#include <vulkan/vulkan_core.h>

const Vertex	vertices[3] =
{
	{ .pos = { 0.0f, -0.5f}, .color = {1, 0, 0} },
	{ .pos = { 0.5f,  0.5f}, .color = {0, 1, 0} },
	{ .pos = {-0.0f,  0.5f}, .color = {0, 0, 1} },
};

VkVertexInputBindingDescription
app_vk_vertex_get_binding(void)
{
	return (const VkVertexInputBindingDescription)
	{
		.binding	= 0,
		.inputRate	= VK_VERTEX_INPUT_RATE_VERTEX,
		.stride		= sizeof(Vertex),
	};
}

VkVertexInputAttributeDescription
*app_vk_vertex_get_attributes(void)
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

void
app_vk_vertex_memory_type(u32 filter, VkMemoryPropertyFlags properties)
{
	App									*app = App_getinstance();
	VkPhysicalDeviceMemoryProperties	mem_properties;

	vkGetPhysicalDeviceMemoryProperties(app->physical_device, &mem_properties);
	for (u32 i = 0; i < mem_properties.memoryTypeCount; ++i)
	{
		if (filter & (1 << i))
			return (i);
	}
	app_panic("failed to find suitable memory type.");
}

void
app_vk_vertex_buffer()
{
	App	*app = App_getinstance();

	const VkBufferCreateInfo	buffer_info =
	{
		.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size			= sizeof(vertices[0]) * ARRAY_LEN(vertices),
		.usage			= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
		.sharingMode	= VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateBuffer(app->device, &buffer_info, NULL, &app->vertex_buffer) != VK_SUCCESS)
		app_panic("failed to create vertex buffer.");
}
