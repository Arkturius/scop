#
#	sources.mk
#

DIR_CORE		:=	core
DIR_MEMORY		:=	memory
DIR_PLATFORM	:=	platform
DIR_RESOURCES	:=	resources
DIR_RENDERER	:=	renderer

RENDERER		:=	vulkan

SRC_CORE		:=	engine.c	\
					debug.c

SRC_MEMORY		:=	allocators.c	\
					buffers.c

SRC_PLATFORM	:=	window.c 		\
					input.c

SRC_RESOURCES	:=	shaders.c		\
					vertices.c

SRC_RENDERER	:=	instance.c		\
					surface.c		\
					devices.c		\
					queues.c		\
					swapchain.c		\
					pipeline.c		\
					commands.c		\
					sync.c


SRC_CORE		:=	$(addprefix $(DIR_CORE)/, 		$(SRC_CORE))
SRC_MEMORY		:=	$(addprefix $(DIR_MEMORY)/, 	$(SRC_MEMORY))
SRC_PLATFORM	:=	$(addprefix $(DIR_PLATFORM)/,	$(SRC_PLATFORM))
SRC_RESOURCES	:=	$(addprefix $(DIR_RESOURCES)/,	$(SRC_RESOURCES))

SRC_RENDERER	:=	$(addprefix $(DIR_RENDERER)/$(RENDERER)/, $(SRC_RENDERER))

SRCS			:=	$(SRC_CORE)		\
                    $(SRC_MEMORY)	\
                    $(SRC_PLATFORM)	\
                    $(SRC_RENDERER)	\
                    $(SRC_RESOURCES)	
