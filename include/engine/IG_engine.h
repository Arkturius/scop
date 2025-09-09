/**
 * scop.h
 */

#if !defined(_SCOP_H)
# define _SCOP_H

# include <vulkan/vulkan.h>

# include <types.h>
# include <geometry.h>

# include <IG_window.h>
# include <IG_input.h>

# define	MAX_FRAMES	2

typedef enum	_engine_state
{
	IG_NULL			,
	IG_WINDOW		,
	IG_INSTANCE 	,
	IG_SURFACE		,
	IG_DEVICE		,
	IG_SWAPCHAIN	,
	IG_IMAGE_VIEWS	,
	IG_PIPELINE		,
	IG_CMD_POOL		,
	IG_VERTEX_BUFFER,
	IG_INDEX_BUFFER ,
	IG_CMD_BUFFER	,
	IG_SYNC			,
	IG_RUNNING		,
}	IGEngineState;

typedef struct _vulkan_ctx		VulkanCtx;
typedef struct _renderer		Renderer;
typedef struct _buffer			Buffer;

typedef struct	_engine
{
	IGEngineState	state;

	Window			window;
	InputManager	inputs;
	VulkanCtx		*vulkan;
	Buffer			*buffer;
	Renderer		*renderer;
	
	u32				current_frame;
	bool			fb_resized;
}	Engine;

extern Engine	IG;

extern const Vertex		vertices[4];
extern const u32		indices[6];

arr_decl(String, Strings);

# define	IG_panic(_m, ...)												\
	{																		\
		error(_m, ##__VA_ARGS__);											\
		IG_cleanup();														\
		exit(1);															\
	}

void
IG_run(void);

void
IG_cleanup(void);

#endif
