/**
 * renderer.h
 */

#include <vulkan/vulkan_core.h>
#if !defined (_RENDERER_H)
# define _RENDERER_H

# include <IG_memory.h>
# include <vulkan/vulkan.h>

# include <types.h>

typedef struct _renderer	Renderer;
typedef struct _shaderfile	ShaderFile;

arr_decl(VkImage,			VkImages);
arr_decl(VkImageView,		VkImageViews);

arr_decl(VkDescriptorSet,	VkDescriptorSets);

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

	VkDescriptorSetLayout	ds_layout;
	VkDescriptorPool		descriptor_pool;
	VkDescriptorSets		descriptor_sets;
	VkPipelineLayout		pp_layout;
	VkPipeline				pipeline;

	UBO					uniform_object;
	Vec3				pos;
	Vec3				rot;

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
IG_vk_descriptor_set_layout(void);

void
IG_vk_pipeline(void);

VkImageView
IG_vk_image_view(VkImage image, VkFormat format, VkImageAspectFlags flags);

void
IG_vk_command_pool(void);

VkFormat
IG_vk_depth_format();

void
IG_vk_depth_resources(void);

void
IG_vk_uniform_buffers(void);

void
IG_vk_update_uniforms(void);

void
IG_vk_descriptor_pool(void);

void
IG_vk_descriptor_sets(void);

void
IG_vk_command_buffers(void);

VkCommandBuffer
IG_vk_command_buffer_single(void);

void
IG_vk_command_buffer_single_end(VkCommandBuffer cmd_buf);

void
IG_vk_texture_image(void);

void
IG_vk_texture_image_view(void);

void
IG_vk_texture_sampler(void);

void
IG_vk_sync_objects(void);

void
IG_vk_sync_render_semaphores(void);

void
IG_vk_draw_frame(void);

#endif // _RENDERER_H
