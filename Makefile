# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/16 14:15:40 by aelmeski          #+#    #+#              #
#    Updated: 2026/09/20 14:26:13 by aelmeski         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME = codexion

SRCS = srcs/main.c srcs/parse.c srcs/parse_utils.c srcs/init.c \
       srcs/init_utils.c srcs/utils.c srcs/heap.c srcs/heap_utils.c \
       srcs/dongle.c srcs/dongle_utils.c srcs/coder.c srcs/monitor.c

OBJS = $(SRCS:.c=.o)

all: $(NAME)

$(NAME): $(OBJS)
	cc -Wall -Wextra -Werror -pthread -o $(NAME) $(OBJS)

srcs/%.o: srcs/%.c srcs/codexion.h
	cc -Wall -Wextra -Werror -pthread -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
