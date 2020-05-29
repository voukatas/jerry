/*
 * jerry.c
 *
 *  Created on: May 11, 2020
 *      Author: voukatas
 */

#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#include "handlers.h"
#include "config.h"


int main()
{

	int server_sock;  						// listen on server_sock
	struct addrinfo addr_info, *ptr_serv_info, *p;
	int restart = 1;							// option to restart the server immediately
	int rv;

	memset(&addr_info, 0, sizeof addr_info);
	addr_info.ai_family = AF_UNSPEC;
	addr_info.ai_socktype = SOCK_STREAM;
	addr_info.ai_flags = AI_PASSIVE; 			// use my IP

	if ((rv = getaddrinfo(NULL, PORT, &addr_info, &ptr_serv_info)) != 0)
	{
		std::cerr << "getaddrinfo: " << gai_strerror(rv) << std::endl;
		return -1;
	}

	// loop through all the results and bind to the first we can
	for (p = ptr_serv_info; p != NULL; p = p->ai_next)
	{
		if ((server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			std::perror("Server: server_sock");
			continue;
		}

		if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &restart, sizeof(int)) == -1)
		{
			std::perror("setsockopt");
			exit(-1);
		}

		if (bind(server_sock, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(server_sock);
			std::perror("Server: bind");
			continue;
		}

		break;
	}

	//free the allocated struct
	freeaddrinfo(ptr_serv_info);

	if (p == NULL)
	{
		std::cerr << "Server: failed to bind" << std::endl;
		exit(-1);
	}

	if (listen(server_sock, BACKLOG) == -1)
	{
		std::perror("listen");
		exit(-1);
	}

	//create the thread that will accept connections
	pthread_t th_accept;
	if ((pthread_create(&th_accept, NULL, &listener, &server_sock)) == 0)
	{
		std::cerr << "Server started : server_sock = " << server_sock << std::endl;
		std::cerr << "Waiting for connections..." << std::endl;

		//wait for thread
		pthread_join(th_accept, NULL);
	}
	else
	{
		std::perror("main():pthread_create: fail");
	}


	close(server_sock);

	return 0;
}
