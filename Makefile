CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -Werror
SANITIZE = -fsanitize=address
LINKERFLAG = -lm

MAIN = my_tar
SRC_DIR = src
SRCS = $(SRC_DIR)/my_tar.c
OBJS = $(SRC_DIR)/_stdio.o         \
       $(SRC_DIR)/_string.o        \
       $(SRC_DIR)/params.o         \
       $(SRC_DIR)/path_node.o      \
       $(SRC_DIR)/c_mode.o         \
       $(SRC_DIR)/t_mode.o         \
       $(SRC_DIR)/r_mode.o         \
       $(SRC_DIR)/u_mode.o         \
       $(SRC_DIR)/x_mode.o         \

.PHONY = all clean fclean re

all: $(MAIN)

$(MAIN): $(SRCS) $(OBJS)
	$(CC) $(CFLAGS) $(SANITIZE) $(SRCS) -o $(MAIN) $(OBJS) $(LINKERFLAG)

clean:
	$(RM) $(OBJS)

fclean: clean
	$(RM) $(MAIN)

re: fclean all
