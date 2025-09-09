/**
 * surface.c
 */

#include <IG_engine.h>
#include <IG_vkcore.h>

void
IG_vk_surface(void)
{
	if (glfwCreateWindowSurface(IG.vulkan->instance, IG.window.handle, NULL, &IG.vulkan->surface) != 0)
		IG_panic("failed to create surface.");

	IG.state = IG_SURFACE;
}
