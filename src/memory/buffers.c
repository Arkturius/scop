/**
 * buffers.c
 */

#include <string.h>

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>

static void
IG_vk_buffer
(
	VkDeviceSize		size,
	VkBufferUsageFlags	usage,
	VkBuffer			*buffer,
	VkDeviceMemory		*buffer_mem,
	u32					properties
)
{
	const VkBufferCreateInfo buffer_info =
	{
		.sType			= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		.size			= size,
		.usage			= usage,
		.sharingMode	= VK_SHARING_MODE_EXCLUSIVE,
	};

	if (vkCreateBuffer(IG.vulkan->device, &buffer_info, NULL, buffer) != VK_SUCCESS)
		IG_panic("failed to create vertex buffer.");

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(IG.vulkan->device, *buffer, &requirements);

	const VkMemoryAllocateInfo alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = IG_vk_buffer_memory_type(requirements.memoryTypeBits, properties),
	};

	if (vkAllocateMemory(IG.vulkan->device, &alloc_info, NULL, buffer_mem) != VK_SUCCESS)
		IG_panic("failed to allocate vertex buffer memory.");

	vkBindBufferMemory(IG.vulkan->device, *buffer, *buffer_mem, 0);
}

u32
IG_vk_buffer_memory_type(u32 filter, VkMemoryPropertyFlags properties)
{
	VkPhysicalDeviceMemoryProperties	mem_properties;

	vkGetPhysicalDeviceMemoryProperties(IG.vulkan->physical_device, &mem_properties);
	for (u32 i = 0; i < mem_properties.memoryTypeCount; ++i)
	{
		if (!(filter & (1 << i)))
			continue ;
		if ((mem_properties.memoryTypes[i].propertyFlags & properties) != properties)
			continue ;
		return (i);
	}
	IG_panic("failed to find suitable memory type.");
}

static void
IG_vk_buffer_copy(VkBuffer dst, VkBuffer src, VkDeviceSize size)
{
	VkCommandBuffer	copy_buffer;

	const VkCommandBufferAllocateInfo alloc_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		.commandPool		= IG.renderer->cmd_pool,
		.level				= VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		.commandBufferCount	= 1,
	};	
	
	if (vkAllocateCommandBuffers(IG.vulkan->device, &alloc_info, &copy_buffer) != VK_SUCCESS)
		IG_panic("failed to allocate command buffers.");

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

	vkQueueSubmit(IG.vulkan->graphics_queue, 1, &sub_info, NULL);
	vkQueueWaitIdle(IG.vulkan->graphics_queue);
}

void
IG_vk_vertex_buffer()
{
	VkDeviceSize	size = sizeof(vertices);
	VkBuffer		staging_buf;
	VkDeviceMemory	staging_mem;

	IG_vk_buffer
	(
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_buf, &staging_mem,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	void	*staging_data;
	
	vkMapMemory(IG.vulkan->device, staging_mem, 0, size, 0, &staging_data);
	memcpy(staging_data, vertices, sizeof(vertices));
	vkUnmapMemory(IG.vulkan->device, staging_mem);
	
	IG_vk_buffer
	(
		size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		&IG.buffer->vertex, &IG.buffer->vertex_mem,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	IG_vk_buffer_copy(IG.buffer->vertex, staging_buf, size);

	if (IG.state != IG_RUNNING)
		IG.state = IG_VERTEX_BUFFER;

	vkFreeMemory(IG.vulkan->device, staging_mem, NULL);
	vkDestroyBuffer(IG.vulkan->device, staging_buf, NULL);
}

void
IG_vk_index_buffer()
{
	VkDeviceSize	size = sizeof(indices);
	VkBuffer		staging_buf;
	VkDeviceMemory	staging_mem;

	IG_vk_buffer
	(
		size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		&staging_buf, &staging_mem,
		VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	);
	void	*staging_data;
	
	vkMapMemory(IG.vulkan->device, staging_mem, 0, size, 0, &staging_data);
	memcpy(staging_data, indices, sizeof(indices));
	vkUnmapMemory(IG.vulkan->device, staging_mem);
	
	IG_vk_buffer
	(
		size, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
		&IG.buffer->index, &IG.buffer->index_mem,
		VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
	);
	IG_vk_buffer_copy(IG.buffer->index, staging_buf, size);

	if (IG.state != IG_RUNNING)
		IG.state = IG_INDEX_BUFFER;

	vkFreeMemory(IG.vulkan->device, staging_mem, NULL);
	vkDestroyBuffer(IG.vulkan->device, staging_buf, NULL);
}
