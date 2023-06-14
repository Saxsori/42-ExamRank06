# **************************************************************************** #
#                                                                              #
#                                                         :::      ::::::::    #
#    Makefile                                           :+:      :+:    :+:    #
#                                                     +:+ +:+         +:+      #
#    By: sasori <sasori@student.42.fr>              +#+  +:+       +#+         #
#                                                 +#+#+#+#+#+   +#+            #
#    Created: 2022/11/08 10:58:57 by oal-tena          #+#    #+#              #
#    Updated: 2023/06/14 23:18:16 by sasori           ###   ########.fr        #
#                                                                              #
# **************************************************************************** #

SRC = ./src/mini_serv.c

OBJ = ${SRC:.c=.o}

NAME = mini_serv

CC = gcc

CFLAGS = -Wall -Werror -Wextra -g
	
RM = rm -rf

all : ${NAME}

${NAME} : ${OBJ}
		${CC} ${CFLAGS} ${OBJ} -o ${NAME}

clean : 
		${RM} ${OBJ}

fclean : clean
		${RM} ${NAME}
 
re : fclean all