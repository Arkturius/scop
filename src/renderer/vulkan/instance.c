/**
 * instance.c
 */

#include <stdlib.h>
#include <string.h>

#include <IG_engine.h>
#include <IG_vkcore.h>

const char	*validation_layers[] =
{
    "VK_LAYER_KHRONOS_validation",
};

#ifdef NDEBUG
volatile bool enable_validation_layers = false;
# else
volatile bool enable_validation_layers = true;
#endif

static Strings
IG_vk_instance_extensions(void)
{
	VkExtensionProperties	*extension_properties	= NULL;
	Strings					extensions				= {0};
	u32						extension_count			= 0;

	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, NULL);
	if (extension_count == 0)
		IG_panic("no vulkan exts found.");

	extension_properties = malloc(extension_count * sizeof(VkExtensionProperties));
	if (!extension_properties)
		IG_panic("malloc failed.");

	vkEnumerateInstanceExtensionProperties(NULL, &extension_count, extension_properties);
	for (u32 i = 0; i < extension_count; ++i)
		arr_append(extensions, strdup(extension_properties[i].extensionName));

	free(extension_properties);
	return (extensions);
}

static Strings
IG_vk_layer_properties(void)
{
	VkLayerProperties	*layer_properties	= NULL;
	Strings				layers				= {0};
	u32					layer_count			= 0;

	vkEnumerateInstanceLayerProperties(&layer_count, NULL);
	if (layer_count == 0)
		IG_panic("no layer properties found.");

	layer_properties = malloc(layer_count * sizeof(VkLayerProperties));
	if (!layer_properties)
		IG_panic("malloc failed.");

	vkEnumerateInstanceLayerProperties(&layer_count, layer_properties);
	for (u32 i = 0; i < layer_count; ++i)
		arr_append(layers, strdup(layer_properties[i].layerName));

	free(layer_properties);
	return (layers);
}

static Strings
IG_glfw_extensions_required(void)
{
	Strings		required_extensions;
	const char	**extensions;
	u32			extension_count;

	extensions = glfwGetRequiredInstanceExtensions(&extension_count);
	
	required_extensions = (Strings)
	{
		.items = (String *) extensions,
		.capacity = extension_count,
		.count = extension_count,
	};
	if (enable_validation_layers)
		arr_append(required_extensions, VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return (required_extensions);
}

static void
IG_glfw_extensions_check(Strings glfw_extensions)
{
	Strings	vk_extensions;
	bool	found;

	vk_extensions = IG_vk_instance_extensions();

	arr_foreach(String, glfw_ext, glfw_extensions)
	{
		found = false;
		arr_foreach(String, vk_ext, vk_extensions)
		{
			if (!strcmp(*glfw_ext, *vk_ext))
			{
				found = true;
				break ;
			}
		}
		if (!found)
			warning("GLFW extension '%s' not supported.", *glfw_ext);
	}
	arr_map(String, vk_extensions, free);
	arr_destroy(vk_extensions);
}


static void
IG_vk_layers_check(Strings required)
{
	Strings				vk_layers = {0};
	bool				found;

	vk_layers = IG_vk_layer_properties();

	arr_foreach(String, layer, required)
	{
		found = false;
		arr_foreach(String, vk_layer, vk_layers)
		{
			if (!strcmp(*vk_layer, *layer))
			{
				found = true;
				break ;
			}
		}
		if (!found)
			IG_panic("validation layer '%s' not supported.", *layer);
	}
	arr_map(String, vk_layers, free);
	arr_destroy(vk_layers);
}

void
IG_vk_instance()
{
	Strings	glfw_extensions	= {0};
	Strings	vk_layers		= {0};

	if (enable_validation_layers)
	{
		arr_count(vk_layers) = array_len(validation_layers);
		vk_layers.items = (String *) validation_layers;
	}
	IG_vk_layers_check(vk_layers);

	glfw_extensions	= IG_glfw_extensions_required();
	IG_glfw_extensions_check(glfw_extensions);

	const VkApplicationInfo	IG_info = 
	{
		.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO,
		.pApplicationName	= "app",
		.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		.pEngineName		= "app",
		.engineVersion		= VK_MAKE_VERSION(1, 0, 0),
		.apiVersion			= VK_MAKE_API_VERSION(0, 1, 4, 0),
	};

	const VkInstanceCreateInfo	create_info =
	{
		.sType						= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
		.pApplicationInfo			= &IG_info,
		.enabledExtensionCount		= glfw_extensions.count,
		.ppEnabledExtensionNames	= (const char **) glfw_extensions.items,
		.enabledLayerCount			= vk_layers.count,
		.ppEnabledLayerNames		= (const char **) vk_layers.items,
	};

	if (vkCreateInstance(&create_info, NULL, &IG.vulkan->instance) != VK_SUCCESS)
		IG_panic("failed to create VkInstance.");
	IG.state = IG_INSTANCE;
}
