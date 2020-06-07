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
#include <signal.h>

#include "handlers.h"
#include "config.h"
#include "util.h"

//thread pool git code https://github.com/voukatas/ThreadPool
#include "ThreadPool.h"

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

		//might also need SO_NOSIGPIPE for broken pipes
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

	//prevent process termination from broken pipes
	signal(SIGPIPE, SIG_IGN);

	if( THREADPOOL == 1)
	{
		std::size_t numOfThreads =  std::thread::hardware_concurrency();

		if(numOfThreads < 1)
		{
			numOfThreads = 1;
		}

		ThreadPoolSpace::ThreadPool workers{numOfThreads};

		printServerInfo(server_sock,numOfThreads);

		//create the arguments list
		ListenerArgs args{server_sock,&workers,Mode::ThreadPool};		

		//The function that executes an endless loop and accepts connections
		listener(&args);
	}
	else
	{
		//create the arguments list
		ListenerArgs args{server_sock,nullptr,Mode::ThreadPerReq};

		//create the thread that will accept connections
		pthread_t th_accept;
		if ((pthread_create(&th_accept, NULL, &listener, &args)) == 0)
		{
			printServerInfo(server_sock,0);

			//wait for thread
			pthread_join(th_accept, NULL);
		}
		else
		{
			std::perror("main():pthread_create: fail");
		}
	}
	
	close(server_sock);

	return 0;
}