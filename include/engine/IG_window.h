/**
 * window.h
 */

#if !defined (_WINDOW_H)
# define _WINDOW_H

# define GLFW_INCLUDE_VULKAN
# include <GLFW/glfw3.h>

# include <types.h>

# define	WIN_WIDTH	800
# define	WIN_HEIGHT	600

typedef struct _window
{
	GLFWwindow	*handle;
	String		title;
	u32			height;
	u32			width;
}	Window;

void
IG_window_init(void);

#endif // _WINDOW_H

