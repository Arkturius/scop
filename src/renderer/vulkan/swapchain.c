/**
 * swapchain.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>
#include <IG_renderer.h>
#include <IG_memory.h>

VkSurfaceFormatKHR
*IG_vk_swap_formats(u32 *formats_count)
{
	VkSurfaceFormatKHR	*formats;

	vkGetPhysicalDeviceSurfaceFormatsKHR(IG.vulkan->physical_device, IG.vulkan->surface, formats_count, NULL);
	if (*formats_count == 0)
		IG_panic("no surface formats found.");

	formats = malloc(*formats_count * sizeof(*formats));
	if (!formats)
		IG_panic("%s: malloc failed.", __func__);

	vkGetPhysicalDeviceSurfaceFormatsKHR(IG.vulkan->physical_device, IG.vulkan->surface, formats_count, formats);
	return (formats);
}

static VkFormat
IG_vk_swap_format_choose(void)
{
	VkFormat			format;
	VkSurfaceFormatKHR	*formats;
	VkSurfaceFormatKHR	*iter;
	u32					formats_count;

	formats = IG_vk_swap_formats(&formats_count);
	format = formats->format;
	for (iter = formats; iter < formats + formats_count; ++iter)
	{
		if (iter->format != VK_FORMAT_B8G8R8A8_SRGB)
			continue ;
		if (iter->colorSpace != VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			continue ;
		format = iter->format;
	}
	free(formats);
	return (format);
}

static VkExtent2D
IG_vk_swap_extent_choose(VkSurfaceCapabilitiesKHR *caps)
{
	if (caps->currentExtent.width != UINT32_MAX)
		return (caps->currentExtent);

	u32	width;
	u32	height;

	glfwGetFramebufferSize(IG.window.handle, (int *)&width, (int *)&height);
	return (VkExtent2D)
	{
		.width = clamp(width, caps->minImageExtent.width, caps->maxImageExtent.width),
		.height = clamp(height, caps->minImageExtent.height, caps->maxImageExtent.height),
	};
}

static VkPresentModeKHR
*IG_vk_swap_present_modes(u32 *mode_count)
{
	VkPresentModeKHR	*modes;

	vkGetPhysicalDeviceSurfacePresentModesKHR(IG.vulkan->physical_device, IG.vulkan->surface, mode_count, NULL);
	if (*mode_count == 0)
		IG_panic("no present modes found.");

	modes = malloc(*mode_count * sizeof(*modes));
	if (!modes)
		IG_panic("%s: malloc failed.", __func__);

	vkGetPhysicalDeviceSurfacePresentModesKHR(IG.vulkan->physical_device, IG.vulkan->surface, mode_count, modes);
	return (modes);
}

static VkPresentModeKHR
IG_vk_swap_present_mode_choose(void)
{
	VkPresentModeKHR	mode;
	VkPresentModeKHR	*modes;
	VkPresentModeKHR	*iter;
	u32					mode_count;

	mode = VK_PRESENT_MODE_FIFO_KHR;
	modes = IG_vk_swap_present_modes(&mode_count);
	for (iter = modes; iter < modes + mode_count; ++iter)
	{
		if (*iter == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			mode = *iter;
			break ;
		}
	}
	free(modes);
	return (mode);
}

static VkImages
IG_vk_swapchain_images(void)
{
	u32			image_count;
	VkImages	images;

	vkGetSwapchainImagesKHR(IG.vulkan->device, IG.renderer->swapchain, &image_count, NULL);
	if (image_count == 0)
		IG_panic("no images in swapchain.");

	images.items = malloc(image_count * sizeof(*images.items));
	if (!images.items)
		IG_panic("%s: malloc failed.", __func__);

	vkGetSwapchainImagesKHR(IG.vulkan->device, IG.renderer->swapchain, &image_count, images.items);

	images.count = image_count;
	images.capacity = image_count;
	return (images);
}

//     vk::raii::ImageView createImageView(vk::raii::Image& image, vk::Format format, vk::ImageAspectFlags aspectFlags) {
//         vk::ImageViewCreateInfo viewInfo{
//             .image = image,
//             .viewType = vk::ImageViewType::e2D,
//             .format = format,
//             .subresourceRange = { aspectFlags, 0, 1, 0, 1 }
//         };
//         return vk::raii::ImageView(device, viewInfo);
//     }
void
IG_vk_swapchain_views(void)
{
	arr_foreach(VkImage, img, IG.renderer->swap_images)
	{
		VkImageView	v = IG_vk_image_view(*img, IG.renderer->swap_format, VK_IMAGE_ASPECT_COLOR_BIT);
		arr_append(IG.renderer->swap_views, v);
	}
	if (IG.state != IG_RUNNING)
		IG.state = IG_IMAGE_VIEWS;
}

void
IG_vk_swapchain(void)
{
	VkSurfaceCapabilitiesKHR	surface_caps;
	VkPresentModeKHR			present_mode;
	u32							min_image_count;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(IG.vulkan->physical_device, IG.vulkan->surface, &surface_caps);

	IG.renderer->swap_format	= IG_vk_swap_format_choose();
	IG.renderer->swap_extent	= IG_vk_swap_extent_choose(&surface_caps);
	present_mode				= IG_vk_swap_present_mode_choose();
	min_image_count				= max(3, surface_caps.minImageCount);

	if (surface_caps.maxImageCount > 0 && min_image_count > surface_caps.maxImageCount)
		min_image_count = surface_caps.maxImageCount;

	const VkSwapchainCreateInfoKHR	create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface			= IG.vulkan->surface,
		.minImageCount		= min_image_count,
		.imageFormat		= IG.renderer->swap_format,
		.imageExtent		= IG.renderer->swap_extent,
		.imageColorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageArrayLayers	= 1,
		.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode	= VK_SHARING_MODE_EXCLUSIVE,
		.preTransform		= surface_caps.currentTransform,
		.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode		= present_mode,
		.clipped			= VK_TRUE,
		.oldSwapchain		= IG.renderer->swapchain ? IG.renderer->swapchain : VK_NULL_HANDLE,
	};

	VkSwapchainKHR	oldSwapchain = IG.renderer->swapchain;

	if (vkCreateSwapchainKHR(IG.vulkan->device, &create_info, NULL, &IG.renderer->swapchain) != VK_SUCCESS)
		IG_panic("failed to create swapchain.");

	if (oldSwapchain != VK_NULL_HANDLE)
		vkDestroySwapchainKHR(IG.vulkan->device, oldSwapchain, NULL);

	arr_destroy(IG.renderer->swap_images);
	IG.renderer->swap_images = IG_vk_swapchain_images();
	if (IG.state != IG_RUNNING)
		IG.state = IG_SWAPCHAIN;
}

void
IG_vk_swapchain_cleanup(void)
{
	arr_map_custom
	(
		VkImageView, view, IG.renderer->swap_views,
		vkDestroyImageView, (IG.vulkan->device, *view, NULL)
	);
	arr_count(IG.renderer->swap_views) = 0;
	arr_map_custom
	(
		VkSemaphore, sem, IG.renderer->render_semaphores,
		vkDestroySemaphore, (IG.vulkan->device, *sem, NULL)
	);
	arr_count(IG.renderer->render_semaphores) = 0;
	vkDestroyImageView(IG.vulkan->device, IG.buffer->depth_view, NULL);
}

void
IG_vk_swapchain_recreate(void)
{
    int width = 0;
	int	height = 0;

    while (true)
	{
        glfwGetFramebufferSize(IG.window.handle, &width, &height);
		if (width != 0 || height != 0)
			break ;
        glfwWaitEvents();
    }

	vkDeviceWaitIdle(IG.vulkan->device);
	IG_vk_swapchain_cleanup();

	IG_vk_swapchain();
	IG_vk_swapchain_views();
	IG_vk_depth_resources();
	IG_vk_sync_render_semaphores();
}
