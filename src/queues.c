/**
 * queues.c
 */

#include <scop.h>
#include <stdbool.h>

bool
app_vk_qfps_graphics(VkQueueFamilyProperties *props, u32 idx)
{
	UNUSED(idx);
	return ((props->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0);
}

bool
app_vk_qfps_present(VkQueueFamilyProperties *props, u32 idx)
{
	App			*app = App_getinstance();
	VkBool32	support = false;

	if (app_vk_qfps_graphics(props, idx))
		vkGetPhysicalDeviceSurfaceSupportKHR(app->physical_device, idx, app->surface, &support);
	return ((bool)support);
}

VkQueueFamilyProperties
*app_vk_qfps_find(VkQueueFamilyProperties *properties, u32 properties_count, qfps_finder finder)
{
	VkQueueFamilyProperties	*iter;

	for (iter = properties; iter < properties + properties_count; ++iter)
	{
		if (finder(iter, iter - properties))
			break ;
	}
	if (iter == properties + properties_count)
		return (NULL);
	return (iter);
}

VkQueueFamilyProperties
*app_vk_qfps(VkPhysicalDevice device, u32 *qfp_count)
{
	VkQueueFamilyProperties		*qfps = NULL;

	vkGetPhysicalDeviceQueueFamilyProperties(device, qfp_count, NULL);

	if (qfp_count == 0)
		app_panic("no physical device queue family properties.");

	qfps = malloc(*qfp_count * sizeof(VkQueueFamilyProperties));
	if (!qfps)
		app_panic("malloc failed.");

	vkGetPhysicalDeviceQueueFamilyProperties(device, qfp_count, qfps);

	return (qfps);
}
