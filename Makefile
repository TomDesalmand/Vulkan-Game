UNAME_S := $(shell uname -s)

CC 		= 	g++

CFLAGS	= 	-std=c++17 -Wall -Wextra -g3

INCLUDES= 	-I./include \

ifeq ($(UNAME_S), Darwin)
	LDFLAGS	= 	-lvulkan -lglfw -lm -L/opt/homebrew/lib
	INCLUDES += -I/opt/homebrew/include
else
	LDFLAGS	= 	-lvulkan -lglfw -lm
endif

RM		=	rm -f

NAME	=	project

SRC		=	$(wildcard *.cpp)	\
			$(wildcard source/*.cpp) \
			$(wildcard source/window/*.cpp) \
			$(wildcard source/pipeline/*.cpp) \
			$(wildcard source/devices/*.cpp) \

OBJ		= 	$(SRC:.cpp=.o)

SHADERS_SRC  = 	$(wildcard shaders/*.vert) \
				$(wildcard shaders/*.frag)

SHADERS_BIN  = 	$(SHADERS_SRC:.vert=.vert.spv)
SHADERS_BIN += 	$(SHADERS_SRC:.frag=.frag.spv)


all		:	$(NAME) shaders

$(NAME)	:	$(OBJ)
		$(CC) $(OBJ) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

shaders	: 	$(SHADERS_BIN)

%.vert.spv: %.vert
	glslc $< -o $@````

%.frag.spv: %.frag
	glslc $< -o $@

clean	:
		$(RM) $(OBJ)


fclean	:	clean
		$(RM) $(NAME)
		$(RM) $(wildcard shaders/*.spv)

re		:	fclean all

.PHONY: all clean fclean re
