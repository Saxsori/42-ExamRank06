/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasori <sasori@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/25 02:11:15 by aaljaber          #+#    #+#             */
/*   Updated: 2023/06/07 03:27:34 by sasori           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>

typedef struct	s_client
{
	int			fd;
	int 		id;
	char		msgStorage[4096];
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
t_client					_clients[1024];
int							id;

void initServer(int port)
{
	_masterSocket = 0;
	_opt = 1;
	_address.sin_family = AF_INET;  
    _address.sin_addr.s_addr = INADDR_ANY;  
    _address.sin_port = htons(port);
    _addrlen = sizeof(_address);
	for (int i = 0; i < 1024; i++)
		_clients[i].fd = -1;
	id = 0;
}

void sendToAll (int fd)
{
	for (int i = 0; i < 1024; i++){
		if (_clients[i].fd != fd && _clients[i].fd != -1)
			send(_clients[i].fd, _msgBuffer, strlen(_msgBuffer), 0);
	}
	memset(_msgBuffer, 0, 4096);
}

void createClient(int fd) {
	for (int i = 0; i < 1024; i++){
		if (_clients[i].fd == -1)
		{
			_clients[i].fd = fd;
			_clients[i].id = id++;
			sprintf(_msgBuffer, "server: client %d just arrived\n", _clients[i].id);
			sendToAll(fd);
			break;
		}
	}
}

void fatalError()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit (1);
}

void	connectClients()
{
	if (select(_maxSocketfd + 1, &_readfds, NULL, NULL, NULL) < 0)
		fatalError();
	if (FD_ISSET(_masterSocket, &_readfds))
	{
		int newSocket;
		// ? extract the 1st connection request on the queue of the pending connection for the listening socket
		// ? create a new connected socket and return its fd
		if ((newSocket = accept(_masterSocket, NULL, NULL)) < 0)
			fatalError();
		if (newSocket > _maxSocketfd)
			_maxSocketfd = newSocket;
		createClient(newSocket);
	}
}

void	getClientMsg()
{
    for (int i = 0; i < 1024; i++)
	{
		if (_clients[i].fd != -1 && FD_ISSET(_clients[i].fd, &_readfds))  
		{
			if ((_readbyte = read(_clients[i].fd, _msgBuffer, 1024)) == 0)  
			{
				sprintf(_msgBuffer, "server: client %d just left\n", _clients[i].id);
				sendToAll(_clients[i].fd);
				FD_CLR(_clients[i].fd, &_readfds);
				close(_clients[i].fd);
				_clients[i].fd = -1;
			}
			else
			{
				// ? when the client receive partial data it doesn't end with new line
				// ? the _msgStorage used to save each data and join them till it get completed
				_msgBuffer[_readbyte] = '\0';
				if (strchr(_msgBuffer, '\n'))
				{
					strcat(_clients[i].msgStorage, _msgBuffer);
					memset(_msgBuffer, 0, 4096);
					sprintf(_msgBuffer, "client %d: %s", _clients[i].id, _clients[i].msgStorage);
					sendToAll(_clients[i].fd);
					memset(_clients[i].msgStorage, 0, 4096);
				}
				else
					strcat(_clients[i].msgStorage, _msgBuffer);
			}
		}
	}
}


int main(int argc, char **argv)  
{
	if (argc != 2)
	{
		write(2, "Wrong number of arguments\n", strlen("Wrong number of arguments\n"));
		exit (1);
	}
	initServer(atoi(argv[1]));
	if (!(_masterSocket = socket(AF_INET , SOCK_STREAM , 0)))  
		fatalError();
	_maxSocketfd = _masterSocket;
	if (bind(_masterSocket, (struct sockaddr *)&_address, sizeof(_address)) < 0)  
		fatalError();
	if (listen(_masterSocket, 1) < 0) 
		fatalError();
	while (1)
	{
		FD_ZERO(&_readfds);		
		FD_SET(_masterSocket, &_readfds);
		for (int i = 0; i < 1024; i++)
			if (_clients[i].fd != -1)
				FD_SET(_clients[i].fd, &_readfds);
		connectClients();
		getClientMsg();
	}
    return (0);  
}
