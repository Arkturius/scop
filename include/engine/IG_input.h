/**
 * input.h
 */

#if !defined (_INPUT_H)
# define _INPUT_H

# include <GLFW/glfw3.h>

# include <types.h>

# define	KEY_MAX				GLFW_KEY_LAST
# define	KEY_TABLE_MAX		((KEY_MAX / 64) + 1)

# define	key_long_idx(_k)	((_k) >> 6)
# define	key_bool_idx(_k)	((_k) & 63)

# define	key_on(_k)			IG.inputs.key_table[key_long_idx(_k)] |= 1ULL << key_bool_idx(_k)
# define	key_off(_k)			IG.inputs.key_table[key_long_idx(_k)] &= ~(1ULL << key_bool_idx(_k))

# define	key_is_on(_k)		(IG.inputs.key_table[key_long_idx(_k)] & (1ULL << key_bool_idx(_k)))
# define	key_is_off(_k)		!(key_is_on(_k))

extern u64	key_table[KEY_TABLE_MAX];

typedef struct _input_manager
{
	u64	key_table[KEY_TABLE_MAX];
}	InputManager;

void
IG_key_on(GLFWwindow *window, i32 key, i32 scan, i32 mods);

void
IG_key_off(GLFWwindow *window, i32 key, i32 scan, i32 mods);

#endif // _INPUT_H

