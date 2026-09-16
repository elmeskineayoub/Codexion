# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/10 23:37:41 by aelmeski          #+#    #+#              #
#    Updated: 2026/09/16 11:26:06 by aelmeski         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: fclean clean re all

CC = cc
CFLAGS = -Wall -Wextra -Werror -pthread -Isrc
NAME = codexion

SRCS = src/coder.c \
       src/dongle.c \
       src/dongle_utils.c \
       src/heap.c \
       src/heap_utils.c \
       src/init.c \
       src/init_utils.c \
       src/main.c \
       src/monitor.c \
       src/parse.c \
       src/parse_utils.c \
       src/utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c src/codexion.h
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all
