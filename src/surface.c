/**
 * surface.c
 */

#include <scop.h>

void
app_vk_surface(void)
{
	App	*app = App_getinstance();

	if (glfwCreateWindowSurface(app->instance, app->window, NULL, &app->surface) != 0)
		app_panic("failed to create surface.");

	app->state = VK_SURFACE;
}
