/**
 * renderer.h
 */

#if !defined (_RENDERER_H)
# define _RENDERER_H

# include <vulkan/vulkan.h>

# include <types.h>

typedef struct _renderer	Renderer;
typedef struct _shaderfile	ShaderFile;

arr_decl(VkImage,			VkImages);
arr_decl(VkImageView,		VkImageViews);
arr_decl(VkCommandBuffer,	VkCommandBuffers);
arr_decl(VkSemaphore,		VkSemaphores);
arr_decl(VkFence,			VkFences);

struct _shaderfile
{
	String	content;
	u32		size;
};

struct _renderer
{
	VkSwapchainKHR		swapchain;
	VkFormat			swap_format;
	VkExtent2D			swap_extent;
	VkImages			swap_images;
	VkImageViews		swap_views;

	VkPipelineLayout	pp_layout;
	VkPipeline			pipeline;

	VkCommandPool		cmd_pool;
	VkCommandBuffers	cmd_buffers;
	VkSemaphores		present_semaphores;
	VkSemaphores		render_semaphores;
	VkFences			draw_fences;
};

void
IG_vk_swapchain();

void
IG_vk_swapchain_cleanup(void);

void
IG_vk_swapchain_views(void);

void
IG_vk_swapchain_recreate(void);

ShaderFile
IG_shader_read(String filepath);

void
IG_shader_cleanup(ShaderFile shader);

VkShaderModule 
IG_vk_shader_module(void);

VkVertexInputBindingDescription
IG_vk_vertex_get_binding(void);

VkVertexInputAttributeDescription
*IG_vk_vertex_get_attributes(void);

void
IG_vk_pipeline(void);

void
IG_vk_command_pool(void);

u32
IG_vk_buffer_memory_type(u32 filter, VkMemoryPropertyFlags properties);

void
IG_vk_vertex_buffer();

void
IG_vk_index_buffer();

void
IG_vk_command_buffers(void);

void
IG_vk_sync_objects(void);

void
IG_vk_sync_render_semaphores();

void
IG_vk_draw_frame(void);

#endif // _RENDERER_H
