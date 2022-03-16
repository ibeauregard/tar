CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror -g3
LINKERFLAG = -lm
SRCS = $(wildcard src/*.c) $(wildcard src/*/*.c)
OBJS = $(SRCS:.c=.o)
MAIN = my_tar

.PHONY = all clean fclean re

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $(LINKERFLAG) $^

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(MAIN)

re: fclean all
