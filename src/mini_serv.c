/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   mini_serv.c                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasori <sasori@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/25 02:11:15 by aaljaber          #+#    #+#             */
/*   Updated: 2023/06/15 04:32:52 by sasori           ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#define MAXCHAR 1000000

typedef struct s_client
{
	int fd;
	int id;
	char msgStorage[MAXCHAR];
} t_client;

int _masterSocket;
struct sockaddr_in _address, _cliAdrr;
int _addrlen, _cliLen;
fd_set _readfds;
int _maxSocketfd;
int _totalBitSet;
int _readbyte;
char _msgBuffer[MAXCHAR];
t_client _clients[1024];
int _id;
char _msg[1000000];


void initServer(int port)
{
	_masterSocket = 0;
	_address.sin_family = AF_INET;
	_address.sin_addr.s_addr = htonl(2130706433);
	_address.sin_port = htons(port);
	_addrlen = sizeof(_address);
	_cliLen = sizeof(_cliAdrr);
	for (int i = 0; i < 1024; i++)
		_clients[i].fd = -1;
	_id = 0;
}

void sendToClients(int fd)
{
	for (int i = 0; i < 1024; i++)
	{
		if (_clients[i].fd != fd && _clients[i].fd != -1)
			send(_clients[i].fd, _msg, strlen(_msg), 0);
	}
	memset(_msg, 0, MAXCHAR);
}

void printfatalError()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit(1);
}

int connectClients()
{
	if (select(_maxSocketfd + 1, &_readfds, NULL, NULL, NULL) < 0)
		return (1);
	if (FD_ISSET(_masterSocket, &_readfds))
	{
		int newSocket;
		if ((newSocket = accept(_masterSocket, (struct sockaddr *)&_cliAdrr, (socklen_t *)&_cliLen)) < 0)
			return (1);
		if (newSocket > _maxSocketfd)
			_maxSocketfd = newSocket;
		for (int i = 0; i < 1024; i++)
		{
			if (_clients[i].fd == -1)
			{
				_clients[i].fd = newSocket;
				_clients[i].id = _id++;
				sprintf(_msg, "server: client %d just arrived\n", _clients[i].id);
				sendToClients(newSocket);
				break;
			}
		}
	}
	return (0);
}

void getClientMsg()
{
	for (int i = 0; i < 1024; i++)
	{
		if (_clients[i].fd != -1 && FD_ISSET(_clients[i].fd, &_readfds))
		{
			if ((_readbyte = recv(_clients[i].fd, _msgBuffer, 1, 0)) <= 0)
			{
				sprintf(_msg, "server: client %d just left\n", _clients[i].id);
				sendToClients(_clients[i].fd);
				FD_CLR(_clients[i].fd, &_readfds);
				close(_clients[i].fd);
				_clients[i].fd = -1;
			}
			else
			{
				_msgBuffer[_readbyte] = '\0';
				if (strstr(_msgBuffer, "\n"))
				{
					unsigned long j = 0;
					if (_clients[i].msgStorage[0] != '\0')
						j = strlen(_clients[i].msgStorage);
					for (unsigned long k = 0; k < strlen(_msgBuffer); k++)
					{
						_clients[i].msgStorage[j++] = _msgBuffer[k];
						if (_msgBuffer[k] == '\n')
						{
							_clients[i].msgStorage[j] = '\0';
							sprintf(_msg, "client %d: %s", _clients[i].id, _clients[i].msgStorage);
							sendToClients(_clients[i].fd);
							j = 0;
							memset(_clients[i].msgStorage, 0, 1000000);
						}
					}
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
	if ((_masterSocket = socket(AF_INET, SOCK_STREAM, 0)) == -1)
		printfatalError();
	_maxSocketfd = _masterSocket;
	if (bind(_masterSocket, (const struct sockaddr *)&_address, sizeof(_address)) != 0)
		printfatalError();
	if (listen(_masterSocket, 10) != 0)
		printfatalError();
	while (1)
	{
		FD_ZERO(&_readfds);
		FD_SET(_masterSocket, &_readfds);
		for (int i = 0; i < 1024; i++)
			if (_clients[i].fd != -1)
				FD_SET(_clients[i].fd, &_readfds);
		if (connectClients())
			continue;
		getClientMsg();
	}
	return (0);
}
