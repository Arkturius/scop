/**
 * app.c
 */

#include <scop.h>

void
app_window_init(void)
{
	App	*app = App_getinstance();

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

	app->window = glfwCreateWindow(WIDTH, HEIGHT, "app", NULL, NULL);
	if (app->window)
		app->running = true;
}


void
app_vk_init(void)
{
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
}

void
app_loop(void)
{
	App	*app = App_getinstance();

	while (!glfwWindowShouldClose(app->window))
		glfwPollEvents();
}

void
app_cleanup(void)
{
	App	*app = App_getinstance();

	switch (app->state)
	{
		case VK_PIPELINE:
			info("destroying VkPipelineKHR.");
			vkDestroyShaderModule(app->logical_device, app->shader, NULL);

		case VK_IMAGE_VIEWS:
			info("destroying VkImageViews");
			vec_foreach(VkImageView, view, app->swap_views)
				vkDestroyImageView(app->logical_device, *view, NULL);
			vec_destroy(app->swap_images);
			vec_destroy(app->swap_views);
			
		case VK_SWAPCHAIN:
			info("destroying VkSwapchainKHR");
			vkDestroySwapchainKHR(app->logical_device, app->swapchain, NULL);

		case VK_DEVICE:
			info("destroying VkDevice.");
			vkDestroyDevice(app->logical_device, NULL);

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

		default:
			break ;
	}
	glfwDestroyWindow(app->window);
	glfwTerminate();
}

void
app_run(void)
{
	App	*app = App_getinstance();

	app_window_init();
	if (!app->running)
		return ;

	app_vk_init();
	app_loop();
	app_cleanup();
}
