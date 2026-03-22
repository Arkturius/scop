/**
 * vkcore.h
 */

#if !defined (_VKCORE_H)
# define _VKCORE_H

# include <vulkan/vulkan.h>

# include <types.h>

typedef struct _vulkan_ctx			VulkanCtx;
typedef VkDebugUtilsMessengerEXT	VkDebug;
typedef bool						(*qfps_finder)(VkQueueFamilyProperties *, u32);

struct _vulkan_ctx
{
	VkInstance			instance;
	VkPhysicalDevice	physical_device;
	VkDevice			device;
	VkQueue				graphics_queue;
	VkSurfaceKHR		surface;
	u32					queue_index;
	VkDebug				debug_messenger;
};

extern volatile bool	enable_validation_layers;

void
IG_vk_instance();

void
IG_vk_debug_messenger(void);

void
IG_vk_debug_destroy(void);

void
IG_vk_surface(void);

void
IG_vk_physical_device(void);


bool
IG_vk_qfps_graphics(VkQueueFamilyProperties *props, u32 idx);

bool
IG_vk_qfps_present(VkQueueFamilyProperties *props, u32 idx);

VkQueueFamilyProperties
*IG_vk_qfps_find(VkQueueFamilyProperties *properties, u32 properties_count, qfps_finder finder);

VkQueueFamilyProperties
*IG_vk_qfps(VkPhysicalDevice device, u32 *qfp_count);

void
IG_vk_logical_device(void);

#endif // _VKCORE_H

