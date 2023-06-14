/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasori <sasori@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/25 02:11:15 by aaljaber          #+#    #+#             */
/*   Updated: 2023/06/14 23:07:56 by sasori           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct s_client
{
	int fd;
	int id;
	char msgStorage[4096];
} t_client;

int _masterSocket;
struct sockaddr_in _address;
int _addrlen;
fd_set _readfds;
int _maxSocketfd;
int _totalBitSet;
int _readbyte;
char _msgBuffer[4096];
t_client _clients[1024];
int _id;

void initServer(int port)
{
	_masterSocket = 0;
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	_address.sin_port = htons(port);
	_addrlen = sizeof(_address);
	for (int i = 0; i < 1024; i++)
		_clients[i].fd = -1;
	_id = 0;
}

void sendToClients(int fd)
{
	for (int i = 0; i < 1024; i++)
	{
		if (_clients[i].fd != fd && _clients[i].fd != -1)
			send(_clients[i].fd, _msgBuffer, strlen(_msgBuffer), 0);
	}
	memset(_msgBuffer, 0, 4096);
}

void printfatalError()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit(1);
}

void connectClients()
{
	if (select(_maxSocketfd + 1, &_readfds, NULL, NULL, NULL) < 0)
		printfatalError();
	if (FD_ISSET(_masterSocket, &_readfds))
	{
		int newSocket;
		if ((newSocket = accept(_masterSocket, NULL, NULL)) < 0)
			printfatalError();
		if (newSocket > _maxSocketfd)
			_maxSocketfd = newSocket;
		for (int i = 0; i < 1024; i++)
		{
			if (_clients[i].fd == -1)
			{
				_clients[i].fd = newSocket;
				_clients[i].id = _id++;
				sprintf(_msgBuffer, "server: client %d just arrived\n", _clients[i].id);
				sendToClients(newSocket);
				break;
			}
		}
	}
}

void getClientMsg()
{
	for (int i = 0; i < 1024; i++)
	{
		if (_clients[i].fd != -1 && FD_ISSET(_clients[i].fd, &_readfds))
		{
			if ((_readbyte = recv(_clients[i].fd, _msgBuffer, 1, 0)) <= 0)
			{
				sprintf(_msgBuffer, "server: client %d just left\n", _clients[i].id);
				sendToClients(_clients[i].fd);
				FD_CLR(_clients[i].fd, &_readfds);
				close(_clients[i].fd);
				_clients[i].fd = -1;
			}
			else
			{
				_msgBuffer[_readbyte] = '\0';
				if (_msgBuffer[_readbyte - 1] == '\n')
				{
					strcat(_clients[i].msgStorage, _msgBuffer);
					memset(_msgBuffer, 0, 4096);
					sprintf(_msgBuffer, "client %d: %s", _clients[i].id, _clients[i].msgStorage);
					sendToClients(_clients[i].fd);
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
		exit(1);
	}
	initServer(atoi(argv[1]));
	if (!(_masterSocket = socket(AF_INET, SOCK_STREAM, 0)))
		printfatalError();
	_maxSocketfd = _masterSocket;
	if (bind(_masterSocket, (struct sockaddr *)&_address, sizeof(_address)) < 0)
		printfatalError();
	if (listen(_masterSocket, 1) < 0)
		printfatalError();
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
