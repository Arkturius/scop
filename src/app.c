/**
 * app.c
 */

#include <GLFW/glfw3.h>
#include <scop.h>

static void
app_window_resize_callback(GLFWwindow *window, int width, int height)
{
	App	*app = glfwGetWindowUserPointer(window);

	UNUSED(width);
	UNUSED(height);
	app->fb_resized = true;
}

# define	KEY_MAX			GLFW_KEY_LAST
# define	KEY_TABLE_MAX	((KEY_MAX / 64) + 1)

# define	key_long_idx(_k)	((_k) >> 6)
# define	key_bool_idx(_k)	((_k) & 63)
# define	key_on(_k)			key_table[key_long_idx(_k)] |= 1 << key_bool_idx(_k)
# define	key_off(_k)			key_table[key_long_idx(_k)] &= ~(1 << key_bool_idx(_k))
# define	key_is_on(_k)		(key_table[key_long_idx(_k)] & (1 << key_bool_idx(_k)))
# define	key_is_off(_k)		!(key_is_on(_k))

u64	key_table[KEY_TABLE_MAX] = {0};

static void
app_key_on(GLFWwindow *window, int key, int scan, int mods)
{
	UNUSED(window);
	UNUSED(scan);
	UNUSED(mods);
	key_on(key);
}

static void
app_key_off(GLFWwindow *window, int key, int scan, int mods)
{
	UNUSED(window);
	UNUSED(scan);
	UNUSED(mods);
	key_off(key);
}

static void
app_key_callback(GLFWwindow *window, int key, int scan, int action, int mods)
{
	warning("\tACTION = %d", action);
	switch (action)
	{
		case GLFW_PRESS:
			app_key_on(window, key, scan, mods);
			break ;
		case GLFW_RELEASE:
			app_key_off(window, key, scan, mods);
			break ;
		case GLFW_REPEAT:
		default:
			break ;
	}	
}

void
app_window_init(void)
{
	App	*app = App_getinstance();

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	app->window = glfwCreateWindow(WIDTH, HEIGHT, "app", NULL, NULL);
	
	glfwSetWindowUserPointer(app->window, app);
	glfwSetFramebufferSizeCallback(app->window, app_window_resize_callback);
	glfwSetKeyCallback(app->window, app_key_callback);

	if (app->window)
		app->state = VK_WINDOW;
}


void
app_vk_init(void)
{
	App	*app = App_getinstance();

	info("creating VkInstance.");
	app_vk_instance();

	info("creating VkDebugMessenger.");
	app_vk_debug_messenger();

	info("creating VkSurfaceKHR");
	app_vk_surface();

	info("creating VkDevice.");
	app_vk_physical_device();
	app_vk_logical_device();

	info("creating VkSwapchainKHR.");
	app_vk_swapchain();

	info("creating VkImageViews.");
	app_vk_swapchain_views();

	info("creating VkPipelineKHR.");
	app_vk_pipeline();

 	info("creating VkCommandPool.")
 	app_vk_command_pool();

	info("creating VkBuffer and VkDeviceMemory. (vertex)");
	app_vk_vertex_buffer();

	info("creating VkBuffer and VkDeviceMemory. (index)");
	app_vk_index_buffer();

 	info("creating VkCommandBuffers")
 	app_vk_command_buffers();

 	info("creating VkSemaphores and VkFences.")
 	app_vk_sync_objects();

	app->state = VK_RUNNING;
}

void
app_loop(void)
{
	App	*app = App_getinstance();

	while (!glfwWindowShouldClose(app->window))
	{
		glfwPollEvents();
		app_vk_draw_frame();

		if (key_is_on(GLFW_KEY_ESCAPE))
			break ;
	}
	vkDeviceWaitIdle(app->device);
}

void
app_cleanup(void)
{
	App	*app = App_getinstance();

	switch (app->state)
	{
		case VK_RUNNING:
		case VK_SYNC:
			info("destroying VkSemaphores and VkFences.");
			vec_map_custom
			(
				VkFence, fence, app->draw_fences,
				vkDestroyFence, (app->device, *fence, NULL)
			);
			vec_destroy(app->draw_fences);
			vec_map_custom
			(
				VkSemaphore, semaphore, app->present_semaphores,
				vkDestroySemaphore, (app->device, *semaphore, NULL)
			);
			vec_destroy(app->present_semaphores);

		case VK_CMD_BUFFER:
			info("destroying VkCommandBuffers.");
			vkFreeCommandBuffers
			(
				app->device, app->cmd_pool,
				vec_count(app->cmd_buffers), vec_first(app->cmd_buffers)
			);
			vec_destroy(app->cmd_buffers);

		case VK_INDEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (index)");
			vkFreeMemory(app->device, app->index_buffer_mem, NULL);
			vkDestroyBuffer(app->device, app->index_buffer, NULL);

		case VK_VERTEX_BUFFER:
			info("destroying VkDeviceMemory and VkBuffer. (vertex)");
			vkFreeMemory(app->device, app->vertex_buffer_mem, NULL);
			vkDestroyBuffer(app->device, app->vertex_buffer, NULL);

		case VK_CMD_POOL:
			info("destroying VkCommandPool.");
			vkDestroyCommandPool(app->device, app->cmd_pool, NULL);

		case VK_PIPELINE:
			info("destroying VkPipelineKHR.");
			vkDestroyPipelineLayout(app->device, app->pp_layout, NULL);
			vkDestroyPipeline(app->device, app->pipeline, NULL);

		case VK_IMAGE_VIEWS:
			info("destroying VkImageViews");
			vec_map_custom 
			(
				VkImageView, view, app->swap_views,
				vkDestroyImageView, (app->device, *view, NULL)
			);
			vec_destroy(app->swap_images);
			vec_destroy(app->swap_views);
			
		case VK_SWAPCHAIN:
			info("destroying VkSwapchainKHR");
			app_vk_swapchain_cleanup();
			vkDestroySwapchainKHR(app->device, app->swapchain, NULL);
			vec_destroy(app->render_semaphores);

		case VK_DEVICE:
			info("destroying VkDevice.");
			vkDestroyDevice(app->device, NULL);

		case VK_SURFACE:
			info("destroying VkSurfaceKHR");
			vkDestroySurfaceKHR(app->instance, app->surface, NULL);

		case VK_INSTANCE:
			info("destroying VkInstance.");
			if (enable_validation_layers)
			{
				info("destroying VkDebugMessenger.");
				app_vk_debug_destroy();
			}
			vkDestroyInstance(app->instance, NULL);

		case VK_WINDOW:
			info("destroying GLFWWindow.");
			glfwDestroyWindow(app->window);

		default:
			break ;
	}
	glfwTerminate();
}

void
app_run(void)
{
	App	*app = App_getinstance();

	app_window_init();
	if (app->state == VK_NULL)
		return ;

	app_vk_init();
	app_loop();
	app_cleanup();
}

