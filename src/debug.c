/**
 * debug.c
 */

#include <scop.h>

static VKAPI_ATTR VkBool32 VKAPI_CALL
app_vk_debug_callback
(
	VkDebugUtilsMessageSeverityFlagBitsEXT		severity,
	VkDebugUtilsMessageTypeFlagsEXT				type,
	const VkDebugUtilsMessengerCallbackDataEXT	*pcallback_data,
	void										*puser_data
)
{
	UNUSED(type);
	UNUSED(puser_data);

	if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		error("%s", pcallback_data->pMessage);
	if (severity == VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
		warning("%s", pcallback_data->pMessage);
	return VK_FALSE;
}

VkResult
app_vk_debug_create(const VkDebugUtilsMessengerCreateInfoEXT *create_info)
{
	App									*app = App_getinstance();
	PFN_vkCreateDebugUtilsMessengerEXT	fn;

	fn = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr
	(
		app->instance,
		"vkCreateDebugUtilsMessengerEXT"
	);

	if (fn)
		return (fn(app->instance, create_info, NULL, &app->debug_messenger));
	return (VK_ERROR_EXTENSION_NOT_PRESENT);
}

void
app_vk_debug_destroy(void)
{
	App									*app = App_getinstance();
	PFN_vkDestroyDebugUtilsMessengerEXT	fn;

	fn = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr
	(
		app->instance,
		"vkDestroyDebugUtilsMessengerEXT"
	);
	if (fn)
		fn(app->instance, app->debug_messenger, NULL);
}

void
app_vk_debug_messenger(void)
{
	VkDebugUtilsMessageSeverityFlagsEXT	severity;
	VkDebugUtilsMessageTypeFlagsEXT		msg_type;

	severity =  VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT;
	severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
	severity |= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;

	msg_type =  VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT;
	msg_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
	msg_type |= VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;

	const VkDebugUtilsMessengerCreateInfoEXT debug_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
		.messageSeverity	= severity,
		.messageType		= msg_type,
		.pfnUserCallback	= app_vk_debug_callback,
	};
	
	app_vk_debug_create(&debug_info);
}
