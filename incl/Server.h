/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.h                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasori <sasori@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/25 02:09:31 by aaljaber          #+#    #+#             */
/*   Updated: 2023/06/07 01:00:51 by sasori           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_H
#define SERVER_H

#include <string.h> // strerror
#include <errno.h> 
#include <unistd.h> // close
#include <arpa/inet.h> // inet_ntoa
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#define PORT 8771
#define	MAXCLNT 10
#define	WLCMSG "Welcome to nano server \r\n"
# define BBLK "\e[1;30m"
# define BRED "\e[1;31m"
# define BGRN "\e[1;32m"
# define BYEL "\e[1;33m"
# define BBLU "\e[1;34m"
# define BMAG "\e[1;35m"
# define BCYN "\e[1;36m"
# define BWHT "\e[1;37m"
# define BPUR "\e[0;35m"
# define DEFCOLO "\033[0m"

typedef struct	s_client
{
	int			fd;
	int 		id;
	char		msgStorage[4096];
	struct	s_client	*next;
}				t_client;

int							_masterSocket;
int 						_opt;
struct sockaddr_in			_address;
int							_addrlen;
fd_set						_readfds; // fds set
int							_maxSocketfd; // will be used to define the range of the fds
int							_totalBitSet;
int							_readbyte;
char						_msgBuffer[4096];
t_client					*_head;

void	resetClients();
int		connectClients();
void	getClientMsg();
void initServer();
void sendToAll (int fd);
void fatalError();

#endif