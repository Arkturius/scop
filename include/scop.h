/**
 * scop.h
 */

#if !defined(_SCOP_H)
# define _SCOP_H

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>
# include <vulkan/vulkan.h>

# include <types.h>
# include <singleton.h>

# define	WIDTH	1366
# define	HEIGHT	768

typedef void	(*_run)(void);

typedef enum	_appstate
{
	VK_NULL			,
	VK_INSTANCE 	,
	VK_SURFACE		,
	VK_DEVICE		,
	VK_SWAPCHAIN	,
	VK_IMAGE_VIEWS	,
	VK_PIPELINE		,
}	AppState;

vec_decl(VkImage);
vec_decl(VkImageView);

typedef struct	_shaderfile
{
	String	content;
	u32		size;
}	ShaderFile;

typedef struct	_app
{
	AppState	state;
	bool		running;
	_run		run;

	GLFWwindow					*window;
	VkInstance					instance;
	VkPhysicalDevice			physical_device;
	VkDevice					logical_device;
	VkQueue						graphics_queue;

	VkSurfaceKHR				surface;
	VkFormat					swap_format;
	VkExtent2D					swap_extent;
	VkImages					swap_images;
	VkImageViews				swap_views;
	VkSwapchainKHR				swapchain;

	VkShaderModule				shader;

	VkDebugUtilsMessengerEXT	debug_messenger;
}	App;

extern volatile bool	enable_validation_layers;

singleton_decl(App);

vec_decl(String);

# define	app_panic(_m, ...)												\
	{																		\
		error(_m, ##__VA_ARGS__);											\
		app_cleanup();														\
		exit(1);															\
	}

void
app_run(void);

void
app_cleanup(void);

void
app_window_init(void);

void
app_vk_init(void);

void
app_vk_instance();

void
app_vk_debug_messenger(void);

void
app_vk_debug_destroy(void);

void
app_vk_surface(void);

void
app_vk_physical_device(void);

typedef bool	(*qfps_finder)(VkQueueFamilyProperties *props, u32 idx);

bool
app_vk_qfps_graphics(VkQueueFamilyProperties *props, u32 idx);

bool
app_vk_qfps_present(VkQueueFamilyProperties *props, u32 idx);

VkQueueFamilyProperties
*app_vk_qfps_find(VkQueueFamilyProperties *properties, u32 properties_count, qfps_finder finder);

VkQueueFamilyProperties
*app_vk_qfps(VkPhysicalDevice device, u32 *qfp_count);

void
app_vk_logical_device(void);

void
app_vk_swapchain();

void
app_vk_swapchain_views(void);

void
app_vk_pipeline(void);

ShaderFile
app_shader_read(String filepath);

void
app_shader_cleanup(ShaderFile shader);

#endif
