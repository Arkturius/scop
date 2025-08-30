/**
 * devices.c
 */

#include <string.h>

#include <scop.h>

static const String device_extensions[] =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME,
	VK_KHR_SPIRV_1_4_EXTENSION_NAME,
	VK_KHR_SYNCHRONIZATION_2_EXTENSION_NAME,
	VK_KHR_CREATE_RENDERPASS_2_EXTENSION_NAME,
};

static VkPhysicalDevice
*app_vk_physical_devices(u32 *device_count)
{
	App					*app = App_getinstance();

	VkPhysicalDevice	*devices;

	vkEnumeratePhysicalDevices(app->instance, device_count, NULL);
	if (device_count == 0)
		app_panic("no suitable GPU found.");

	devices = malloc(*device_count * sizeof(VkPhysicalDevice));
	if (!devices)
		app_panic("%s: malloc failed.", __func__);

	vkEnumeratePhysicalDevices(app->instance, device_count, devices);
	return (devices);
}

static Strings
app_vk_device_extensions(VkPhysicalDevice device)
{
	VkExtensionProperties	*extension_properties	= NULL;
	Strings					extensions				= {0};
	u32						extension_count			= 0;

	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, NULL);
	if (extension_count == 0)
		app_panic("no device extensions.");

	extension_properties = malloc(extension_count * sizeof(VkExtensionProperties));
	if (!extension_properties)
		app_panic("malloc");

	vkEnumerateDeviceExtensionProperties(device, NULL, &extension_count, extension_properties);
	for (u32 i = 0; i < extension_count; ++i)
		vec_append(extensions, strdup(extension_properties[i].extensionName));

	free(extension_properties);
	return (extensions);
}

static String
*app_vk_ext_iter(String device_ext, Strings extensions)
{
	vec_foreach(String, ext, extensions)
	{
		if (!strcmp(*ext, device_ext))
			return (ext);
	}
	return (extensions.items + extensions.count);
}

static VkPhysicalDevice
*app_vk_physical_device_pick(VkPhysicalDevice *devices, u32 device_count)
{
	u32							qfp_count = 0;
	VkPhysicalDeviceProperties	device_properties;

	for (u32 i = 0; i < device_count; ++i)
	{
		VkPhysicalDevice	device = devices[i];

		vkGetPhysicalDeviceProperties(device, &device_properties);
		
		bool	is_suitable = device_properties.apiVersion >= VK_API_VERSION_1_3;
		
		VkQueueFamilyProperties	*qfps		= app_vk_qfps(device, &qfp_count);
		VkQueueFamilyProperties	*qfp_iter	= app_vk_qfps_find(qfps, qfp_count, app_vk_qfps_graphics);

		free(qfps);
		is_suitable = is_suitable && (qfp_iter);

		Strings			extensions	= app_vk_device_extensions(device);
		bool			found		= true;
		const String	*dext		= device_extensions;

		for (; dext < device_extensions + ARRAY_LEN(device_extensions); ++dext)
		{
			String	*ext = app_vk_ext_iter(*dext, extensions);

			found = found && (ext != extensions.items + extensions.count);
		}
		vec_map(String, extensions, free);
		vec_destroy(extensions);

		is_suitable = is_suitable && found;
		if (is_suitable)
			return (&devices[i]);
	}
	return (NULL);
}

void
app_vk_physical_device(void)
{
	App					*app			= App_getinstance();
	VkPhysicalDevice	*devices		= NULL;
	u32					device_count	= 0;

	devices = app_vk_physical_devices(&device_count);

	app->physical_device = *app_vk_physical_device_pick(devices, device_count);
	if (!app->physical_device)
		app_panic("no physical device selected.");
	free(devices);
}

void
app_vk_logical_device(void)
{
	App	*app		= App_getinstance();
	u32	qfp_count	= 0;

	VkQueueFamilyProperties	*qfps		= app_vk_qfps(app->physical_device, &qfp_count);
	VkQueueFamilyProperties	*qfp_iter	= app_vk_qfps_find(qfps, qfp_count, app_vk_qfps_present);

	free(qfps); // TODO : big arena to manage those temporary vecs and mallocs...

	if (!qfp_iter)
		app_panic("no graphics queue family found.");

	u32	graphics_index = qfp_iter - qfps;

	VkPhysicalDeviceExtendedDynamicStateFeaturesEXT	dynamic = 
	{
		.sType					= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_EXTENDED_DYNAMIC_STATE_FEATURES_EXT,
		.extendedDynamicState	= VK_TRUE,
		.pNext					= NULL,
	};
	VkPhysicalDeviceVulkan13Features				vk13 = 
	{
		.sType					= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES,
		.dynamicRendering		= VK_TRUE,
		.pNext					= &dynamic,
	};
	VkPhysicalDeviceFeatures2						features2 =
	{
		.sType					= VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2,
		.pNext 					= &vk13,
	};

	float	priority = 0.0f;
	VkDeviceQueueCreateInfo	dq_create_info = 
	{
		.sType					= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
		.queueCount				= 1,
		.queueFamilyIndex		= graphics_index,
		.pQueuePriorities		= &priority,
	};
	VkDeviceCreateInfo	create_info = 
	{
		.sType						= VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
		.queueCreateInfoCount		= 1,
		.pQueueCreateInfos			= &dq_create_info,
		.pNext						= &features2,
		.enabledExtensionCount		= ARRAY_LEN(device_extensions),
		.ppEnabledExtensionNames	= (const char **) device_extensions,
	};

	if (vkCreateDevice(app->physical_device, &create_info, NULL, &app->logical_device) != VK_SUCCESS)
		app_panic("failed to create logical device.");

	vkGetDeviceQueue(app->logical_device, 0, graphics_index, &app->graphics_queue);
	if (!app->graphics_queue)
		app_panic("failed to retrieve graphics + present queue.");

	app->state = VK_DEVICE;
}
