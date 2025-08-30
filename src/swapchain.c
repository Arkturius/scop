/**
 * swapchain.c
 */

#include <scop.h>

VkSurfaceFormatKHR
*app_vk_swap_formats(u32 *formats_count)
{
	App					*app = App_getinstance();
	VkSurfaceFormatKHR	*formats;

	vkGetPhysicalDeviceSurfaceFormatsKHR(app->physical_device, app->surface, formats_count, NULL);
	if (*formats_count == 0)
		app_panic("no surface formats found.");

	formats = malloc(*formats_count * sizeof(*formats));
	if (!formats)
		app_panic("%s: malloc failed.", __func__);

	vkGetPhysicalDeviceSurfaceFormatsKHR(app->physical_device, app->surface, formats_count, formats);
	return (formats);
}

static VkFormat
app_vk_swap_format_choose(void)
{
	VkFormat			format;
	VkSurfaceFormatKHR	*formats;
	VkSurfaceFormatKHR	*iter;
	u32					formats_count;

	formats = app_vk_swap_formats(&formats_count);
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
app_vk_swap_extent_choose(VkSurfaceCapabilitiesKHR *caps)
{
	App	*app = App_getinstance();

	if (caps->currentExtent.width != UINT32_MAX)
		return (caps->currentExtent);

	u32	width;
	u32	height;

	glfwGetFramebufferSize(app->window, (int *)&width, (int *)&height);
	return (VkExtent2D)
	{
		.width = CLAMP(width, caps->minImageExtent.width, caps->maxImageExtent.width),
		.height = CLAMP(height, caps->minImageExtent.height, caps->maxImageExtent.height),
	};
}

static VkPresentModeKHR
*app_vk_swap_present_modes(u32 *mode_count)
{
	App					*app = App_getinstance();
	VkPresentModeKHR	*modes;

	vkGetPhysicalDeviceSurfacePresentModesKHR(app->physical_device, app->surface, mode_count, NULL);
	if (*mode_count == 0)
		app_panic("no present modes found.");

	modes = malloc(*mode_count * sizeof(*modes));
	if (!modes)
		app_panic("%s: malloc failed.", __func__);

	vkGetPhysicalDeviceSurfacePresentModesKHR(app->physical_device, app->surface, mode_count, modes);
	return (modes);
}

static VkPresentModeKHR
app_vk_swap_present_mode_choose(void)
{
	VkPresentModeKHR	mode;
	VkPresentModeKHR	*modes;
	VkPresentModeKHR	*iter;
	u32					mode_count;

	mode = VK_PRESENT_MODE_FIFO_KHR;
	modes = app_vk_swap_present_modes(&mode_count);
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
app_vk_swapchain_images(void)
{
	App			*app = App_getinstance();
	u32			image_count;
	VkImages	images;

	vkGetSwapchainImagesKHR(app->logical_device, app->swapchain, &image_count, NULL);
	if (image_count == 0)
		app_panic("no images in swapchain.");

	images.items = malloc(image_count * sizeof(*images.items));
	if (!images.items)
		app_panic("%s: malloc failed.", __func__);

	vkGetSwapchainImagesKHR(app->logical_device, app->swapchain, &image_count, images.items);

	images.count = image_count;
	images.capacity = image_count;
	return (images);
}

void
app_vk_swapchain_views(void)
{
	App			*app = App_getinstance();
	VkImageView	view;

	VkImageViewCreateInfo	create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    	.viewType			= VK_IMAGE_VIEW_TYPE_2D,
		.format				= app->swap_format,
		.subresourceRange	= (VkImageSubresourceRange)
		{
			.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT,
			.baseMipLevel	= 0,
			.levelCount		= 1,
			.baseArrayLayer = 0,
			.layerCount		= 1,
		},
	};

	vec_foreach(VkImage, img, app->swap_images)
	{
		create_info.image = *img;
		vkCreateImageView(app->logical_device, &create_info, NULL, &view);
		vec_append(app->swap_views, view);
	}
	app->state = VK_IMAGE_VIEWS;
}

void
app_vk_swapchain(void)
{
	App							*app = App_getinstance();
	VkSurfaceCapabilitiesKHR	surface_caps;
	VkPresentModeKHR			present_mode;
	u32							min_image_count;

	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(app->physical_device, app->surface, &surface_caps);
	app->swap_format = app_vk_swap_format_choose();
	app->swap_extent = app_vk_swap_extent_choose(&surface_caps);

	min_image_count = MAX(3, surface_caps.minImageCount);
	if (surface_caps.maxImageCount > 0 && min_image_count > surface_caps.maxImageCount)
		min_image_count = surface_caps.maxImageCount;
	
	present_mode = app_vk_swap_present_mode_choose();

	const VkSwapchainCreateInfoKHR	create_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		.surface			= app->surface,
		.minImageCount		= min_image_count,
		.imageFormat		= app->swap_format,
		.imageExtent		= app->swap_extent,
		.imageColorSpace	= VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
		.imageArrayLayers	= 1,
		.imageUsage			= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		.imageSharingMode	= VK_SHARING_MODE_EXCLUSIVE,
		.preTransform		= surface_caps.currentTransform,
		.compositeAlpha		= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
		.presentMode		= present_mode,
		.clipped			= VK_TRUE,
	};

	if (vkCreateSwapchainKHR(app->logical_device, &create_info, NULL, &app->swapchain) != VK_SUCCESS)
		app_panic("failed to create swapchain.");

	app->swap_images = app_vk_swapchain_images();
	app->state = VK_SWAPCHAIN;
}
