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
#include <cstring>

#include "handlers.h"
#include "config.h"
#include "util.h"
#include "Logger/Logger.h"
#include "Logger/loglvl.h"

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

	//initialize the logger
	try
	{
		LoggerSpace::Logger::instance().log(LOGLEVEL,"Logging started");		
	}
	catch(const std::iostream::failure& e)
	{
		std::cerr << "Logger init failed, abort. " << e.what() << '\n';
		exit(-1);
	}
	

	if ((rv = getaddrinfo(NULL, PORT, &addr_info, &ptr_serv_info)) != 0)
	{
		LoggerSpace::Logger::instance().log(Loglvl::FATAL,"main():getaddrinfo: " + std::string(gai_strerror(rv)));		
		exit(-1);
	}

	// loop through all the results and bind to the first we can
	for (p = ptr_serv_info; p != NULL; p = p->ai_next)
	{
		if ((server_sock = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1)
		{
			LoggerSpace::Logger::instance().log(Loglvl::ERROR,"main():server_sock " + std::string(std::strerror(errno)));
			continue;
		}

		//might also need SO_NOSIGPIPE for broken pipes
		if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &restart, sizeof(int)) == -1)
		{
			LoggerSpace::Logger::instance().log(Loglvl::FATAL,"main():setsockopt " + std::string(std::strerror(errno)));
			exit(-1);
		}

		if (bind(server_sock, p->ai_addr, p->ai_addrlen) == -1)
		{
			close(server_sock);
			LoggerSpace::Logger::instance().log(Loglvl::ERROR,"main():bind " + std::string(std::strerror(errno)));
			continue;
		}

		break;
	}

	//free the allocated struct
	freeaddrinfo(ptr_serv_info);

	if (p == NULL)
	{
		LoggerSpace::Logger::instance().log(Loglvl::FATAL,"main():failed to bind ");		
		exit(-1);
	}

	if (listen(server_sock, BACKLOG) == -1)
	{
		LoggerSpace::Logger::instance().log(Loglvl::FATAL,"main():listen " + std::string(std::strerror(errno)));
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
			LoggerSpace::Logger::instance().log(Loglvl::ERROR,"main():pthread_create: fail " + std::string(std::strerror(errno)));			
		}
	}
	
	close(server_sock);

	return 0;
}