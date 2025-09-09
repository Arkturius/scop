/**
 * queues.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>

bool
IG_vk_qfps_graphics(VkQueueFamilyProperties *props, u32 idx)
{
	unused(idx);
	return ((props->queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0);
}

bool
IG_vk_qfps_present(VkQueueFamilyProperties *props, u32 idx)
{
	VkBool32	support = false;

	if (IG_vk_qfps_graphics(props, idx))
		vkGetPhysicalDeviceSurfaceSupportKHR(IG.vulkan->physical_device, idx, IG.vulkan->surface, &support);
	return ((bool)support);
}

VkQueueFamilyProperties
*IG_vk_qfps_find(VkQueueFamilyProperties *properties, u32 properties_count, qfps_finder finder)
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
*IG_vk_qfps(VkPhysicalDevice device, u32 *qfp_count)
{
	VkQueueFamilyProperties		*qfps = NULL;

	vkGetPhysicalDeviceQueueFamilyProperties(device, qfp_count, NULL);

	if (qfp_count == 0)
		IG_panic("no physical device queue family properties.");

	qfps = malloc(*qfp_count * sizeof(VkQueueFamilyProperties));
	if (!qfps)
		IG_panic("malloc failed.");

	vkGetPhysicalDeviceQueueFamilyProperties(device, qfp_count, qfps);

	return (qfps);
}
