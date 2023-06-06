/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.c                                             :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: sasori <sasori@student.42.fr>              +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2022/12/25 02:11:15 by aaljaber          #+#    #+#             */
/*   Updated: 2023/06/07 01:14:47 by sasori           ###   ########.fr       */
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

void initServer(int port)
{
	_masterSocket = 0;
	_opt = 1;
	_address.sin_family = AF_INET;  
    _address.sin_addr.s_addr = INADDR_ANY;  
    _address.sin_port = htons(port);
    _addrlen = sizeof(_address);
	_head = NULL;
}

t_client* createNode(int value) {
    t_client* newNode = (t_client *) malloc(sizeof(t_client));
    newNode->id = 0;
	newNode->fd = value;
    newNode->next = NULL;
    return newNode;
}

void sendToAll (int fd)
{
	t_client* current = _head;
	while (current != NULL) {
		if (current->fd != fd)
			send(current->fd, _msgBuffer, strlen(_msgBuffer), 0);
		current = current->next;
	}
	memset(_msgBuffer, 0, 4096);
}

void addNodeToList(t_client *newNode) {
    if (_head == NULL)
        _head = newNode;
    else {
        t_client* current = _head;
        while (current->next != NULL)
            current = current->next;
		int length = sprintf(_msgBuffer, "server: client %d just arrived\n", current->id + 1);
		_msgBuffer[length] = '\0';
		sendToAll(-1);
        current->next = newNode;
		newNode->id = current->id + 1;
    }
}

void deleteNode(int fd) {
    t_client* current = _head;
    t_client* previous = NULL;

    while (current != NULL && current->fd != fd) {
        previous = current;
        current = current->next;
    }

    if (current != NULL) {
        if (previous == NULL)
            _head = current->next;
        else
            previous->next = current->next;
        
		int length = sprintf(_msgBuffer, "server: client %d just left\n", current->id);
		_msgBuffer[length] = '\0';
        free(current);
		sendToAll(-1);
    }
}

void fatalError()
{
	write(2, "Fatal error\n", strlen("Fatal error\n"));
	exit (1);
}

int	connectClients()
{
	// ? indicate which of the specified fds is ready to reading
	if (select(_maxSocketfd + 1, &_readfds, NULL, NULL, NULL) < 0)
		fatalError();

	// ? if the fd is still in the set
	if (FD_ISSET(_masterSocket, &_readfds))
	{
		int newSocket;
		// ? extract the 1st connection request on the queue of the pending connection for the listening socket
		// ? create a new connected socket and return its fd
		if ((newSocket = accept(_masterSocket, (struct sockaddr *)&_address, (socklen_t*)&_addrlen)) < 0)
			fatalError();

		// printf("💬 New connection: socket fd is %d\n", newSocket);
		
		if (newSocket > _maxSocketfd)
			_maxSocketfd = newSocket;

		// ? add the connected socket to the client list
		addNodeToList(createNode(newSocket));
	}
	return (1);
}

void	getClientMsg()
{
    for (t_client* current = _head; current != NULL; )
	{
		if (FD_ISSET(current->fd, &_readfds))  
		{
			// ? read the message recieved
			if ((_readbyte = read(current->fd, _msgBuffer, 1024)) == 0)  
			{
				// ? if read returned 0, means user is disconnected, so here close the fd and erase client's info
				
				// ? to get the info of server fd, the port and the address
				// printf("🛑 Disconnection: socket fd is %d\n", current->fd);				
				
				close(current->fd);
				deleteNode(current->fd);
				if (_head == NULL)
					break ;
			}
			else
			{
				// ? when the client receive partial data it doesn't end with new line
				// ? the _msgStorage used to save each data and join them till it get completed
				_msgBuffer[_readbyte] = '\0';
				if (strchr(_msgBuffer, '\n'))
				{
					strcat(current->msgStorage, _msgBuffer);
					memset(_msgBuffer, 0, 4096);
					// printf("📥 Received: %s\n", current->msgStorage);
					int length = sprintf(_msgBuffer, "client %d: %s", current->id, current->msgStorage);
					_msgBuffer[length] = '\0';
					sendToAll(current->fd);
					memset(current->msgStorage, 0, 4096);
				}
				else
					strcat(current->msgStorage, _msgBuffer);
			}
		}
		current = current->next;
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
		// ? clear the fds from the set, used to initialize the fds set
		FD_ZERO(&_readfds);		
		// ? adding server socket to the set
		FD_SET(_masterSocket, &_readfds);
		// ? adding client's socket fds to the set
		for (t_client* current = _head; current != NULL; )
		{
			FD_SET(current->fd, &_readfds);
			current = current->next;
		}
		if (!connectClients())
			return (1);
		getClientMsg();
	}
    return (0);  
}
