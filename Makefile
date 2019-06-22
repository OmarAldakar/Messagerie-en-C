NAME = main
FILES = main.c $(wildcard interface/*.c network/*.c library/*.c threads/*.c)
LIB = -lpthread -lreadline -lm -g `pkg-config gtk+-3.0 --libs`

HEADERS = $(wildcard interface/headers/*.h network/headers/*.h library/headers/*.h threads/headers/*.h)
OBJ = $(FILES:%.c=%.o)

CC = gcc
FLAGS = -Wall -Wextra `pkg-config gtk+-3.0 --cflags`

.PHONY: all
all: $(NAME)


$(NAME): $(OBJ)
	$(CC) $(FLAGS) -o $@ $^ $(LIB)


%.o: %.c $(HEADERS)
	$(CC) $(FLAGS) -o $@ -c $<

.PHONY: clean
clean:
	rm -rf $(OBJ)

.PHONY: fclean
fclean: clean
	rm -rf $(NAME)

.PHONY: re
re: fclean all
