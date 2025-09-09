/**
 * input.c
 */

#include <IG_engine.h>
#include <IG_input.h>

void
IG_key_on(GLFWwindow *window, i32 key, i32 scan, i32 mods)
{
	unused(window);
	unused(scan);
	unused(mods);
	key_on(key);
}

void
IG_key_off(GLFWwindow *window, i32 key, i32 scan, i32 mods)
{
	unused(window);
	unused(scan);
	unused(mods);
	key_off(key);
}
