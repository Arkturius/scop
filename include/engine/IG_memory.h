/**
 * memory.h
 */

#if !defined (_MEMORY_H)
# define _MEMORY_H

# include <vulkan/vulkan.h>

typedef struct _buffer_allocator	BufferAllocator;

struct _buffer_allocator
{
	// TODO : build the allocator.
};

typedef struct _buffer
{
	VkBuffer			vertex;
	VkDeviceMemory		vertex_mem;
	VkBuffer			index;
	VkDeviceMemory		index_mem;
}	Buffer;

#endif // _MEMORY_H

