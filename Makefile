# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: aelmeski <aelmeski@student.42.fr>          +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2026/09/16 14:15:40 by aelmeski          #+#    #+#              #
#    Updated: 2026/09/20 14:16:33 by aelmeski         ###   ########.fr        #
#                                                                              #
# **************************************************************************** #


NAME		= codexion

CC			= cc
CFLAGS		= -Wall -Wextra -Werror -pthread

SRCS_DIR	= srcs

SRCS		= $(SRCS_DIR)/main.c \
			  $(SRCS_DIR)/parse.c \
			  $(SRCS_DIR)/parse_utils.c \
			  $(SRCS_DIR)/init.c \
			  $(SRCS_DIR)/init_utils.c \
			  $(SRCS_DIR)/utils.c \
			  $(SRCS_DIR)/heap.c \
			  $(SRCS_DIR)/heap_utils.c \
			  $(SRCS_DIR)/dongle.c \
			  $(SRCS_DIR)/dongle_utils.c \
			  $(SRCS_DIR)/coder.c \
			  $(SRCS_DIR)/monitor.c

OBJS		= $(SRCS:.c=.o)

HEADER		= $(SRCS_DIR)/codexion.h

all: $(NAME)

$(NAME): $(OBJS)
	$(CC) $(CFLAGS) -o $(NAME) $(OBJS)

$(SRCS_DIR)/%.o: $(SRCS_DIR)/%.c $(HEADER)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS)

fclean: clean
	rm -f $(NAME)

re: fclean all

.PHONY: all clean fclean re
