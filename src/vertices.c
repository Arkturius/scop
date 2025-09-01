/**
 * vertices.c
 */

#include <string.h>

#include <scop.h>
#include <geometry.h>

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

u32
app_vk_vertex_memory_type(u32 filter, VkMemoryPropertyFlags properties)
{
	App									*app = App_getinstance();
	VkPhysicalDeviceMemoryProperties	mem_properties;

	vkGetPhysicalDeviceMemoryProperties(app->physical_device, &mem_properties);
	for (u32 i = 0; i < mem_properties.memoryTypeCount; ++i)
	{
		if (!(filter & (1 << i)))
			continue ;
		if ((mem_properties.memoryTypes[i].propertyFlags & properties) != properties)
			continue ;
		return (i);
	}
	app_panic("failed to find suitable memory type.");
}

static void
app_vk_buffer
(
	VkDeviceSize		size,
	VkBufferUsageFlags	usage,
	VkBuffer			*buffer,
	VkDeviceMemory		*buffer_mem,
	u32					properties
)
{
	App	*app = App_getinstance();

	const VkBufferCreateInfo buffer_info =
	{
		.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size			= size,
		.usage			= usage,
		.sharingMode	= VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateBuffer(app->device, &buffer_info, NULL, buffer) != VK_SUCCESS)
		app_panic("failed to create vertex buffer.");

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(app->device, *buffer, &requirements);

	const VkMemoryAllocateInfo alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = app_vk_vertex_memory_type(requirements.memoryTypeBits, properties),
	};

	if (vkAllocateMemory(app->device, &alloc_info, NULL, buffer_mem) != VK_SUCCESS)
		app_panic("failed to allocate vertex buffer memory.");

	vkBindBufferMemory(app->device, *buffer, *buffer_mem, 0);
}

static void
app_vk_buffer_copy(VkBuffer dst, VkBuffer src, VkDeviceSize size)
{
	App				*app = App_getinstance();
	VkCommandBuffer	copy_buffer;

	const VkCommandBufferAllocateInfo alloc_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool		= app->cmd_pool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount	= 1,
	};	
	
	if (vkAllocateCommandBuffers(app->device, &alloc_info, &copy_buffer) != VK_SUCCESS)
		app_panic("failed to allocate command buffers.");

	const VkCommandBufferBeginInfo begin_info = 
	{
		.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
	};

	vkBeginCommandBuffer(copy_buffer, &begin_info);
	vkCmdCopyBuffer(copy_buffer, src, dst, 1, &(VkBufferCopy){0, 0, size});
	vkEndCommandBuffer(copy_buffer);

	const VkSubmitInfo sub_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_SUBMIT_INFO,
		.pWaitDstStageMask		= &(VkPipelineStageFlags){VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
		.commandBufferCount		= 1,
		.pCommandBuffers		= &copy_buffer,
	};

	vkQueueSubmit(app->graphics_queue, 1, &sub_info, NULL);
	vkQueueWaitIdle(app->graphics_queue);
}

void
app_vk_vertex_buffer()
{
	App				*app = App_getinstance();
	VkDeviceSize	size = sizeof(vertices);
	VkBuffer		staging_buf;
	VkDeviceMemory	staging_mem;

	app_vk_buffer
	(
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_buf, &staging_mem,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	void	*staging_data;
	
	vkMapMemory(app->device, staging_mem, 0, size, 0, &staging_data);
	memcpy(staging_data, vertices, sizeof(vertices));
	vkUnmapMemory(app->device, staging_mem);
	
	app_vk_buffer
	(
		size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		&app->vertex_buffer, &app->vertex_buffer_mem,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	app_vk_buffer_copy(app->vertex_buffer, staging_buf, size);

	if (app->state != VK_RUNNING)
		app->state = VK_VERTEX_BUFFER;

	vkFreeMemory(app->device, staging_mem, NULL);
	vkDestroyBuffer(app->device, staging_buf, NULL);
}

void
app_vk_index_buffer()
{
	App				*app = App_getinstance();
	VkDeviceSize	size = sizeof(indices);
	VkBuffer		staging_buf;
	VkDeviceMemory	staging_mem;

	app_vk_buffer
	(
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_buf, &staging_mem,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	void	*staging_data;
	
	vkMapMemory(app->device, staging_mem, 0, size, 0, &staging_data);
	memcpy(staging_data, indices, sizeof(indices));
	vkUnmapMemory(app->device, staging_mem);
	
	app_vk_buffer
	(
		size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		&app->index_buffer, &app->index_buffer_mem,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	app_vk_buffer_copy(app->index_buffer, staging_buf, size);

	if (app->state != VK_RUNNING)
		app->state = VK_INDEX_BUFFER;

	vkFreeMemory(app->device, staging_mem, NULL);
	vkDestroyBuffer(app->device, staging_buf, NULL);
}

