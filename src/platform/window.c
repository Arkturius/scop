/**
 * window.c
 */

#include <IG_engine.h>
#include <IG_input.h>
#include <IG_window.h>

static void
IG_callback_window_resize(GLFWwindow *window, int width, int height)
{
	unused(window);
	unused(width);
	unused(height);
	IG.fb_resized = true;
}

static void
IG_callback_keyboard(GLFWwindow *window, int key, int scan, int action, int mods)
{
	warning("\tACTION = %d", action);
	switch (action)
	{
		case GLFW_PRESS:
			IG_key_on(window, key, scan, mods);
			break ;
		case GLFW_RELEASE:
			IG_key_off(window, key, scan, mods);
			break ;
		case GLFW_REPEAT:
		default:
			break ;
	}	
}

void
IG_window_init(void)
{
	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	IG.window.handle = glfwCreateWindow(WIN_WIDTH, WIN_HEIGHT, "ignis", NULL, NULL);
	
	glfwSetFramebufferSizeCallback(IG.window.handle, IG_callback_window_resize);
	glfwSetKeyCallback(IG.window.handle, IG_callback_keyboard);

	if (IG.window.handle)
		IG.state = IG_WINDOW;
}
