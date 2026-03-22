/**
 * buffers.c
 */

#include "geometry.h"
#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>
#include <string.h>

void
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
		IG_panic("failed to create buffer.");

	VkMemoryRequirements requirements;
	vkGetBufferMemoryRequirements(IG.vulkan->device, *buffer, &requirements);

	const VkMemoryAllocateInfo alloc_info =
	{
		.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
		.allocationSize = requirements.size,
		.memoryTypeIndex = IG_vk_buffer_memory_type(requirements.memoryTypeBits, properties),
	};

	if (vkAllocateMemory(IG.vulkan->device, &alloc_info, NULL, buffer_mem) != VK_SUCCESS)
		IG_panic("failed to allocate buffer memory.");

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
	VkCommandBuffer	copy_buffer = IG_vk_command_buffer_single();
	vkCmdCopyBuffer(copy_buffer, src, dst, 1, &(VkBufferCopy){0, 0, size});
	IG_vk_command_buffer_single_end(copy_buffer);
}

# define JUSTOBJ_IMPLEMENTATION
# include <job.h>

arr_decl(Vertex, Vertices);

void
IG_vk_vertex_buffer()
{
	IG.model_file = job_open_model("viking_room.obj");
	if (!IG.model_file.content)
		IG_panic("failed to open model.");

	if (job_parse_model(&IG.model_file, &IG.model_data) == false)
		IG_panic("failed to parse model.");

	VkDeviceSize	size = arr_count(IG.model_data.v) * sizeof(Vertex);
//	VkDeviceSize	size = 8 * sizeof(Vertex);

	info("parsed model: %d vertices, %d faces\n", arr_count(IG.model_data.v), arr_count(IG.model_data.f));

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

#if 1

	Vertices	vertices = {0};

	arr_foreach(JOBv, v, IG.model_data.v)
	{
		Vertex	new_vertex;

		new_vertex.pos.x = v->x;
		new_vertex.pos.y = v->y;
		new_vertex.pos.z = v->z;
		new_vertex.color = vec3(v->x, v->y, v->z);
		new_vertex.tex = vec2(v->x, v->z);
//		info("vertex : {%f %f %f}\n", new_vertex.pos.x, new_vertex.pos.y, new_vertex.pos.z);
		arr_append(vertices, new_vertex);
	}
	memcpy(staging_data, vertices.items, size);
#else

	Vertex	vertices[8] = 
	{
		{{-0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, 0.0f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}},

		{{-0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
		{{0.5f, -0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
		{{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{-0.5f, 0.5f, -0.5f}, {1.0f, 1.0f, 1.0f}, {0.0f, 1.0f}}
	};
	memcpy(staging_data, vertices, size);

#endif

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

arr_decl(u32, Indices);

void
IG_vk_index_buffer()
{
	VkDeviceSize	size = arr_count(IG.model_data.f) * 3 * sizeof(u32);
//	VkDeviceSize	size = sizeof(u32) * 12;
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

#if 1
	
	Indices	indices = {0};

	arr_foreach(JOBf, f, IG.model_data.f)
	{
		arr_append(indices, f->v.x - 1);
		arr_append(indices, f->v.y - 1);
		arr_append(indices, f->v.z - 1);
	}
	memcpy(staging_data, indices.items, arr_count(indices) * sizeof(indices.items[0]));
#else

	u32	indices[12] = 
	{
		0, 1, 2, 2, 3, 0,
		4, 5, 6, 6, 7, 4,
	};
	memcpy(staging_data, indices, size);

#endif

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

