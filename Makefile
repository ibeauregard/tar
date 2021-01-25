CC = gcc
CFLAGS = -Wall -Wextra -Werror -g3 -fsanitize=address
LINKERFLAG = -lm
SRCS = $(wildcard *.c)
OBJS = $(SRCS:.c=.o)
MAIN = my_tar
.PHONY = all clean

all: $(MAIN)

$(MAIN): $(OBJS)
	$(CC) $(CFLAGS) -o $(MAIN) $(LINKERFLAG) $(OBJS)
	$(RM) *.o

clean:
	$(RM) $(MAIN)