#
#	Makefile
#

NAME		:=	libengine.so

SRC_DIR		:=	src
INC_DIR		:=	include
OBJ_DIR		:=	build

include			sources.mk
SRCS		:=	$(addprefix $(SRC_DIR)/, $(SRCS))

COPTS_DIRS	:=	$(INC_DIR) $(INC_DIR)/engine
COPTS		+=	$(foreach dir, $(COPTS_DIRS), -I$(dir))

OBJS 		:=	$(addprefix $(OBJ_DIR)/, $(SRCS:%.c=%.o))

CC			:=	clang

CFLAGS		:=	 -fPIC -O3 -Wall -Wextra -MMD -MP -g
LDFLAGS		:=	-lglfw -lvulkan -ldl -pthread -lX11 -lXxf86vm -lXrandr -lXi

SLANG		:=	slangc
SLANG_FLAGS	:=	-target spirv -profile spirv_1_4 -emit-spirv-directly -fvk-use-entrypoint-name -entry vert_main -entry frag_main

SHADER_FILE	:=	$(SRC_DIR)/shaders/shader.slang
SHADER_COMP	:=	$(SRC_DIR)/shaders/shader.spv


ifeq ($(TEMP), 1)
	CFLAGS	+=	--save-temps=obj
endif

VERBOSE		?=	0
ifeq ($(VERBOSE), 1)
	CFLAGS	+=	-DVERBOSE=1
endif

ifeq ($(NDEBUG), 1)
	CFLAGS	+=	-DNDEBUG
endif

RM			:=	rm -rf
MKDIR		:=	mkdir -p

MAKE		+=	--no-print-directory

#
# Rules
#

all:					$(SHADER_COMP) $(NAME)

$(NAME):				$(OBJS)
	@echo " $(GREEN)$(BOLD)$(ITALIC)■$(RESET)  linking	$(GREEN)$(BOLD)$(ITALIC)$(NAME)$(RESET)"
	@$(CC) -shared $(CFLAGS) $(COPTS) -o $@ $^ $(LDFLAGS)

$(OBJ_DIR)/src/%.o: 	src/%.c
	@$(MKDIR) $(@D)
	@echo " $(CYAN)$(BOLD)$(ITALIC)■$(RESET)  compiling	$(GRAY)$(BOLD)$(ITALIC)$(notdir $@) from $(GRAY)$(BOLD)$(ITALIC)$(notdir $^)$(RESET)"
	@$(CC) $(CFLAGS) -o $@ -c $(COPTS) $<

$(SHADER_COMP):	$(SHADER_FILE)
	@echo " $(CYAN)$(BOLD)$(ITALIC)■$(RESET)  compiling shader $(GRAY)$(BOLD)$(ITALIC)$(notdir $@) from $(GRAY)$(BOLD)$(ITALIC)$(notdir $^)$(RESET)"
	@$(SLANG) $< $(SLANG_FLAGS) -o $@

clean:
	@if [ -f "$(SHADER_COMP)" ]; then \
		echo " $(RED)$(BOLD)$(ITALIC)■$(RESET)  deleted	$(RED)$(BOLD)$(ITALIC)$(SHADER_COMP)$(RESET)"; \
		$(RM) $(SHADER_COMP); \
	fi
	@if [ -d $(OBJ_DIR) ]; then \
		echo " $(RED)$(BOLD)$(ITALIC)■$(RESET)  deleted	$(RED)$(BOLD)$(ITALIC)$(NAME)/$(OBJ_DIR)$(RESET)"; \
		$(RM) $(OBJ_DIR); \
	fi

fclean:					clean
	@if [ -f "$(NAME)" ]; then \
		echo " $(RED)$(BOLD)$(ITALIC)■$(RESET)  deleted	$(RED)$(BOLD)$(ITALIC)$(NAME)$(RESET)"; \
		$(RM) $(NAME); \
	fi;

re:					fclean all

-include	$(OBJS:.o=.d)

.PHONY:		all clean fclean re
.SILENT:	all clean fclean re

#
# COLORS
# 

BOLD			=	\033[1m
ITALIC			=	\033[3m

RED				=	\033[31m
GREEN			=	\033[32m
YELLOW			=	\033[33m
CYAN			=	\033[36m
GRAY			=	\033[90m

RESET			=	\033[0m

LINE_CLR		=	\33[2K\r


