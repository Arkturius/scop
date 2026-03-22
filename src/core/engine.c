/**
 * IG.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>
#include <vulkan/vulkan_core.h>

Engine	IG = {0};

void
IG_vulkan_init(void)
{
	info("creating VkInstance.");
	IG_vk_instance();

	info("creating VkDebugMessenger.");
	IG_vk_debug_messenger();

	info("creating VkSurfaceKHR");
	IG_vk_surface();

	info("creating VkDevice.");
	IG_vk_physical_device();
	IG_vk_logical_device();

	info("creating VkSwapchainKHR.");
	IG_vk_swapchain();

	info("creating VkImageViews.");
	IG_vk_swapchain_views();

	info("creating VkDescriptorSetLayout.");
	IG_vk_descriptor_set_layout();

	info("creating VkPipelineKHR.");
	IG_vk_pipeline();

 	info("creating VkCommandPool.")
 	IG_vk_command_pool();

	info("creating VkImage, VkDeviceMemory and VkImageView. (depth)");
	IG_vk_depth_resources();

	info("creating VkImage and VkDeviceMemory. (texture)");
	IG_vk_texture_image();

	info("creating VkImageView. (texture)");
	IG_vk_texture_image_view();

	info("creating VkSampler.");
	IG_vk_texture_sampler();


	info("creating VkBuffer and VkDeviceMemory. (vertex)");
	IG_vk_vertex_buffer();

	info("creating VkBuffer and VkDeviceMemory. (index)");
	IG_vk_index_buffer();

	info("creating VkBuffer and VkDeviceMemory. (index)");
	IG_vk_uniform_buffers();

	info("creating VkDescriptorPool.");
	IG_vk_descriptor_pool();

	info("creating VkDescriptorSets.");
	IG_vk_descriptor_sets();


 	info("creating VkCommandBuffers")
 	IG_vk_command_buffers();

 	info("creating VkSemaphores and VkFences.")
 	IG_vk_sync_objects();

	IG.state = IG_RUNNING;
}

void
IG_loop(void)
{
	IG.renderer->pos = vec3(0, -10, 0);
	IG.renderer->rot = vec3(0, 0, 0);
	while (!glfwWindowShouldClose(IG.window.handle))
	{
		glfwPollEvents();
		IG_vk_draw_frame();

		if (key_is_on(GLFW_KEY_ESCAPE))
			break ;

		if (key_is_on(GLFW_KEY_W))
			IG.renderer->pos.y += 0.1f;
		if (key_is_on(GLFW_KEY_A))
			IG.renderer->pos.x -= 0.1f;
		if (key_is_on(GLFW_KEY_S))
			IG.renderer->pos.y -= 0.1f;
		if (key_is_on(GLFW_KEY_D))
			IG.renderer->pos.x += 0.1f;
		if (key_is_on(GLFW_KEY_SPACE))
			IG.renderer->pos.z += 0.1f;
		if (key_is_on(GLFW_KEY_LEFT_SHIFT))
			IG.renderer->pos.z -= 0.1f;
	}
	vkDeviceWaitIdle(IG.vulkan->device);
}

void
IG_cleanup(void)
{
	switch (IG.state)
	{
		case IG_RUNNING:
		case IG_SYNC:
			info("destroying VkSemaphores and VkFences.");
			arr_map_custom
			(
				VkFence, fence, IG.renderer->draw_fences,
				vkDestroyFence, (IG.vulkan->device, *fence, NULL)
			);
			arr_destroy(IG.renderer->draw_fences);
			arr_map_custom
			(
				VkSemaphore, semaphore, IG.renderer->present_semaphores,
				vkDestroySemaphore, (IG.vulkan->device, *semaphore, NULL)
			);
			arr_destroy(IG.renderer->present_semaphores);

		case IG_CMD_BUFFER:
			info("destroying VkCommandBuffers.");
			vkFreeCommandBuffers
			(
				IG.vulkan->device, IG.renderer->cmd_pool,
				arr_count(IG.renderer->cmd_buffers), arr_first(IG.renderer->cmd_buffers)
			);
			arr_destroy(IG.renderer->cmd_buffers);

		case IG_DESCRIPTOR_SETS:
			info("destroying VkDescriptorSets");
			vkFreeDescriptorSets
			(
				IG.vulkan->device, IG.renderer->descriptor_pool,
				arr_count(IG.renderer->descriptor_sets),
				IG.renderer->descriptor_sets.items
			);

		case IG_DESCRIPTOR_POOL:
			info("destroying VkDescriptorPool");
			vkDestroyDescriptorPool(IG.vulkan->device, IG.renderer->descriptor_pool, NULL);

		case IG_UNIFORM_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (uniform)");
			arr_map_custom
			(
				VkDeviceMemory, memory, IG.buffer->uniform_mem,
				vkFreeMemory, (IG.vulkan->device, *memory, NULL)
			);
			arr_map_custom
			(
				VkBuffer, buffer, IG.buffer->uniform,
				vkDestroyBuffer, (IG.vulkan->device, *buffer, NULL)
			);

		case IG_INDEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (index)");
			vkFreeMemory(IG.vulkan->device, IG.buffer->index_mem, NULL);
			vkDestroyBuffer(IG.vulkan->device, IG.buffer->index, NULL);

		case IG_VERTEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (vertex)");
			vkFreeMemory(IG.vulkan->device, IG.buffer->vertex_mem, NULL);
			vkDestroyBuffer(IG.vulkan->device, IG.buffer->vertex, NULL);

		case IG_TEXTURE:
			info("destroying VkDeviceMemory and VkImage. (texture)");
//			vkFreeMemory(IG.vulkan->device, IG.buffer->texture_mem, NULL);

		case IG_CMD_POOL:
			info("destroying VkCommandPool.");
			vkDestroyCommandPool(IG.vulkan->device, IG.renderer->cmd_pool, NULL);

		case IG_PIPELINE:
			info("destroying VkPipelineKHR.");
			vkDestroyPipelineLayout(IG.vulkan->device, IG.renderer->pp_layout, NULL);
			vkDestroyPipeline(IG.vulkan->device, IG.renderer->pipeline, NULL);

		case IG_DS_LAYOUT:
			info("destroying VkDescriptorSetLayout.");
			vkDestroyDescriptorSetLayout(IG.vulkan->device, IG.renderer->ds_layout, NULL);

		case IG_IMAGE_VIEWS:
			info("destroying VkImageViews");
			arr_map_custom 
			(
				VkImageView, view, IG.renderer->swap_views,
				vkDestroyImageView, (IG.vulkan->device, *view, NULL)
			);
			arr_destroy(IG.renderer->swap_images);
			arr_destroy(IG.renderer->swap_views);
			
		case IG_SWAPCHAIN:
			info("destroying VkSwapchainKHR");
			IG_vk_swapchain_cleanup();
			vkDestroySwapchainKHR(IG.vulkan->device, IG.renderer->swapchain, NULL);
			arr_destroy(IG.renderer->render_semaphores);

		case IG_DEVICE:
			info("destroying VkDevice.");
			vkDestroyDevice(IG.vulkan->device, NULL);

		case IG_SURFACE:
			info("destroying VkSurfaceKHR");
			vkDestroySurfaceKHR(IG.vulkan->instance, IG.vulkan->surface, NULL);

		case IG_INSTANCE:
			info("destroying VkInstance.");
			if (enable_validation_layers)
			{
				info("destroying VkDebugMessenger.");
				IG_vk_debug_destroy();
			}
			vkDestroyInstance(IG.vulkan->instance, NULL);

		case IG_WINDOW:
			info("destroying GLFWWindow.");
			glfwDestroyWindow(IG.window.handle);

		default:
			break ;
	}
	glfwTerminate();
	free(IG.vulkan);
	free(IG.renderer);
	free(IG.buffer);
}

#include <string.h>

void
IG_run(void)
{
	IG.vulkan	= malloc(sizeof(VulkanCtx));
	IG.renderer	= malloc(sizeof(Renderer));
	IG.buffer	= malloc(sizeof(Buffer));

	memset(IG.vulkan, 0, sizeof(VulkanCtx));
	memset(IG.renderer, 0, sizeof(Renderer));
	memset(IG.buffer, 0, sizeof(Buffer));

	IG_window_init();
	IG_vulkan_init();
	if (IG.state == IG_NULL)
		return ;
	IG_loop();
	IG_cleanup();
}

