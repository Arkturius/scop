/**
 * devices.c
 */

#include <string.h>

#include <IG_engine.h>
#include <IG_vkcore.h>

static const String device_extensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};

static VkPhysicalDevice
*IG_vk_physical_devices(u32 *device_count)
{
	VkPhysicalDevice	*devices;

	vkEnumeratePhysicalDevices(IG.vulkan->instance, device_count, NULL);
	if (device_count == 0)
		IG_panic("no suitable GPU found.");

	devices = malloc(*device_count * sizeof(VkPhysicalDevice));
	if (!devices)
		IG_panic("%s: malloc failed.", __func__);

	vkEnumeratePhysicalDevices(IG.vulkan->instance, device_count, devices);
	return (devices);
}

static Strings
IG_vk_device_extensions(VkPhysicalDevice device)
{
	VkExtensionProperties	*extension_properties	= NULL;
	Strings					extensions				= {0};
	u32						extension_count			= 0;

	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
	if (extension_count == 0)
		IG_panic("no device extensions.");

	extension_properties = malloc(extension_count * sizeof(VkExtensionProperties));
	if (!extension_properties)
		IG_panic("%s: malloc failed.", __func__);

	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_properties);
	for (u32 i = 0; i < extension_count; ++i)
		arr_append(extensions, strdup(extension_properties[i].extensionName));

	free(extension_properties);
	return (extensions);
}

static String
*IG_vk_ext_iter(String device_ext, Strings extensions)
{
	arr_foreach(String, ext, extensions)
	{
		if (!strcmp(*ext, device_ext))
			return (ext);
	}
	return (extensions.items + extensions.count);
}

static VkPhysicalDevice
*IG_vk_physical_device_pick(VkPhysicalDevice *devices, u32 device_count)
{
	u32							qfp_count = 0;
	VkPhysicalDeviceProperties	device_properties;

	for (u32 i = 0; i < device_count; ++i)
	{
		VkPhysicalDevice	device = devices[i];

		vkGetPhysicalDeviceProperties(device, &device_properties);
		
		bool	is_suitable = device_properties.apiVersion >= VK_API_VERSION_1_3;
		
		VkQueueFamilyProperties	*qfps		= IG_vk_qfps(device, &qfp_count);
		VkQueueFamilyProperties	*qfp_iter	= IG_vk_qfps_find(qfps, qfp_count, IG_vk_qfps_graphics);

		free(qfps);
		is_suitable = is_suitable && (qfp_iter);

		Strings			extensions	= IG_vk_device_extensions(device);
		bool			found		= true;
		const String	*dext		= device_extensions;

		for (; dext < device_extensions + array_len(device_extensions); ++dext)
		{
			String	*ext = IG_vk_ext_iter(*dext, extensions);

			found = found && (ext != extensions.items + extensions.count);
		}
		arr_map(String, extensions, free);
		arr_destroy(extensions);

		is_suitable = is_suitable && found;
		if (is_suitable)
			return (&devices[i]);
	}
	return (NULL);
}

void
IG_vk_physical_device(void)
{
	VkPhysicalDevice	*devices		= NULL;
	u32					device_count	= 0;

	devices = IG_vk_physical_devices(&device_count);

	IG.vulkan->physical_device = *IG_vk_physical_device_pick(devices, device_count);
	if (!IG.vulkan->physical_device)
		IG_panic("no physical device selected.");
	free(devices);
}

void
IG_vk_logical_device(void)
{
	u32		qfp_count	= 0;
	float	priority	= 0.0f;

	VkQueueFamilyProperties	*qfps		= IG_vk_qfps(IG.vulkan->physical_device, &qfp_count);
	VkQueueFamilyProperties	*qfp_iter	= IG_vk_qfps_find(qfps, qfp_count, IG_vk_qfps_present);

	free(qfps); // TODO : big arena to manage those temporary vecs and mallocs...

	if (!qfp_iter)
		IG_panic("no graphics queue family found.");

	IG.vulkan->queue_index = qfp_iter - qfps;

	const VkDeviceQueueCreateInfo dq_create_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount				= 1,
		.queueFamilyIndex		= IG.vulkan->queue_index,
		.pQueuePriorities		= &priority,
	};

	const VkDeviceCreateInfo create_info = 
	{
		.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount		= 1,
		.pQueueCreateInfos			= &dq_create_info,
		.pNext						= &(VkPhysicalDeviceFeatures2)
		{
			.sType					= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
			.pNext 					= &(VkPhysicalDeviceVulkan13Features)
			{
				.sType				= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
				.dynamicRendering	= VK_TRUE,
				.synchronization2	= VK_TRUE,
				.pNext				= &(VkPhysicalDeviceExtendedDynamicStateFeaturesEXT)
				{
					.sType					= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
					.extendedDynamicState	= VK_TRUE,
					.pNext					= NULL,
				},
			},
		},
		.enabledExtensionCount		= array_len(device_extensions),
		.ppEnabledExtensionNames	= (const char **)device_extensions,
	};

	if (vkCreateDevice(IG.vulkan->physical_device, &create_info, NULL, &IG.vulkan->device) != VK_SUCCESS)
		IG_panic("failed to create logical device.");

	vkGetDeviceQueue(IG.vulkan->device, 0, IG.vulkan->queue_index, &IG.vulkan->graphics_queue);
	if (!IG.vulkan->graphics_queue)
		IG_panic("failed to retrieve graphics + present queue.");

	IG.state = IG_DEVICE;
}
