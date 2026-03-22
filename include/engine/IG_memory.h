/**
 * memory.h
 */

#if !defined (_MEMORY_H)
# define _MEMORY_H

# include <geometry.h>
# include <vulkan/vulkan.h>

typedef struct _buffer_allocator	BufferAllocator;

struct _buffer_allocator
{
	// TODO : build the allocator.
};

typedef struct _uniform_buffer_object
{
	alignas(16) Mat4	model;
	alignas(16) Mat4	view;
	alignas(16) Mat4	proj;
}	UBO;

arr_decl(VkBuffer,			VkBuffers);
arr_decl(VkDeviceMemory,	VkDeviceMemories);
arr_decl(void *,			pVoids);

typedef struct _buffer
{
	VkBuffer			vertex;
	VkDeviceMemory		vertex_mem;

	VkBuffer			index;
	VkDeviceMemory		index_mem;

	VkBuffers			uniform;
	VkDeviceMemories	uniform_mem;
	pVoids				uniform_mapped;

	VkImage				texture;
	VkImageView			texture_view;
	VkDeviceMemory		texture_mem;
	VkSampler			texture_sampler;

	VkImage				depth;
	VkImageView			depth_view;
	VkDeviceMemory		depth_mem;
}	Buffer;

u32
IG_vk_buffer_memory_type(u32 filter, VkMemoryPropertyFlags properties);

void
IG_vk_vertex_buffer(void);

void
IG_vk_index_buffer(void);

void
IG_vk_buffer
(VkDeviceSize size, VkBufferUsageFlags usage, VkBuffer *buffer, VkDeviceMemory *buffer_mem, u32 properties);

#endif // _MEMORY_H

