# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/12 23:37:41 by aelmeski          #+#    #+#              #
#    Updated: 2026/09/12 23:37:53 by aelmeski         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

.PHONY: fclean clean re all

CC = cc

CFLAGS = -Wall -Wextra -Werror -pthread

NAME = codexion

SRCS = coder.c dongle.c heap.c init.c main.c monitor.c parse.c utils.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)
$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

%.o: %.c codexion.h
	$(CC) $(CFLAGS) -c $< -o $@


re:
	$(MAKE) fclean
	$(MAKE) all

clean:
	rm -f $(OBJS)

fclean:
	rm -f $(OBJS) $(NAME)
