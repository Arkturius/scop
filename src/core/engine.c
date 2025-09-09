/**
 * IG.c
 */

#include <GLFW/glfw3.h>

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>

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

	info("creating VkPipelineKHR.");
	IG_vk_pipeline();

 	info("creating VkCommandPool.")
 	IG_vk_command_pool();



	info("creating VkBuffer and VkDeviceMemory. (vertex)");
	IG_vk_vertex_buffer();

	info("creating VkBuffer and VkDeviceMemory. (index)");
	IG_vk_index_buffer();



 	info("creating VkCommandBuffers")
 	IG_vk_command_buffers();

 	info("creating VkSemaphores and VkFences.")
 	IG_vk_sync_objects();

	IG.state = IG_RUNNING;
}

void
IG_loop(void)
{
	while (!glfwWindowShouldClose(IG.window.handle))
	{
		glfwPollEvents();
		IG_vk_draw_frame();

		if (key_is_on(GLFW_KEY_ESCAPE))
			break ;
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

		case IG_INDEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (index)");
			vkFreeMemory(IG.vulkan->device, IG.buffer->index_mem, NULL);
			vkDestroyBuffer(IG.vulkan->device, IG.buffer->index, NULL);

		case IG_VERTEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (vertex)");
			vkFreeMemory(IG.vulkan->device, IG.buffer->vertex_mem, NULL);
			vkDestroyBuffer(IG.vulkan->device, IG.buffer->vertex, NULL);

		case IG_CMD_POOL:
			info("destroying VkCommandPool.");
			vkDestroyCommandPool(IG.vulkan->device, IG.renderer->cmd_pool, NULL);

		case IG_PIPELINE:
			info("destroying VkPipelineKHR.");
			vkDestroyPipelineLayout(IG.vulkan->device, IG.renderer->pp_layout, NULL);
			vkDestroyPipeline(IG.vulkan->device, IG.renderer->pipeline, NULL);

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

void
IG_run(void)
{
	IG.vulkan	= malloc(sizeof(VulkanCtx));
	IG.renderer	= malloc(sizeof(Renderer));
	IG.buffer	= malloc(sizeof(Buffer));

	IG_window_init();
	IG_vulkan_init();
	if (IG.state == IG_NULL)
		return ;
	IG_loop();
	IG_cleanup();
}

