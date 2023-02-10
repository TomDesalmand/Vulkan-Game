CFLAGS	= -Wall -Wextra -g3 -I ./include

LDFLAGS	= -lvulkan -lglfw

RM		=	rm -f

NAME	=	project

SRC		=	$(wildcard *.cpp)	\
			$(wildcard source/*.cpp) \

OBJ		= 	$(SRC:.cpp=.o)

all		:	$(NAME)

$(NAME)	:	$(OBJ)
		$(CC) $(OBJ) -o $(NAME) $(LDFLAGS) -lm

clean	:
		$(RM) $(OBJ)

fclean	:	clean
		$(RM) $(NAME)

re		:	fclean all

.PHONY: all clean fclean re