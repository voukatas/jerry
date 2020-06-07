/*
 * handlers.c
 *
 *  Created on: May 13, 2020
 *      Author: voukatas
 */
#include <cstdio>
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <new>

#include "handlers.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "config.h"
#include "ThreadPool.h"
#include "util.h"

static std::string fail_msg = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: 29\n\n<H1>Not a valid Request!</H1>";

//close connection
static void closeConn(int clientSocket);

//serve client function
void *handleClient(void* client_sock)
{
	int clientSocket = *((int*)client_sock);
	//free the previously allocated int
	delete (int*)client_sock;

	HttpRequest req{clientSocket};
	HttpResponse res{clientSocket};

	int cause = req.readData();//0 for success

	if(cause != 0)
	{
		//no need to use closeConn, we do not need to close conn gracefully
		//readData closes the conn immediately
		return NULL;
	}

	req.parseReq();

	if(req.isReqValid() != 0)
	{
		res.sendData(fail_msg);
		closeConn(clientSocket);
		return NULL;// connection closed, nothing to do so return
	}

	std::string path = req.getPath().empty()?"index.html":req.getPath();

	if(DEBUG)
	{
		std::cerr << "handlers: path = " << path << "\n" << std::endl;
		std::cerr << "handlers: req.getPath().empty() = " << req.getPath().empty() << "\n" << std::endl;
		std::cerr << "handlers: req.getPath() = " << req.getPath() << "\n" << std::endl;
	}

	std::string html_page;

	try
	{
		res.readHtml(path,html_page);
//		html_page = res.readHtml(path);
	}
	catch(const std::exception& e)
	{
		std::cerr << "handlers: exception = " << e.what() << "\n" << std::endl;
		res.sendData(fail_msg);
		closeConn(clientSocket);
		return NULL;// connection closed, nothing to do so return
	}


	res.sendData(html_page);

	//close the connection
	closeConn(clientSocket);

	return NULL;

}

//accept connections thread where each request run on a new thread
void *listener(void* listenerArgs)
{	
	ListenerArgs* args = reinterpret_cast<ListenerArgs*>(listenerArgs);
	int server_sock_local = args->server_sock;
	Mode mode = args->mode;
	//needs casting since it is void
	//ThreadPool& workers = *((ThreadPool*)args->workers);
	ThreadPoolSpace::ThreadPool* workers = nullptr;
	if(args->workers!=nullptr)
	{
		workers = reinterpret_cast<ThreadPoolSpace::ThreadPool*>(args->workers);
	}
	

	while (1)
	{

		int* p_clientSock = nullptr;
		int accept_fd = -1;


		if(DEBUG)
		{
			//Debugging logs
			struct sockaddr_storage their_addr; // connector's address information
			socklen_t sin_size;
			sin_size = sizeof their_addr;

			accept_fd = accept(server_sock_local, (struct sockaddr *) &their_addr, &sin_size);

			if (accept_fd > 0)
			{
				char s[INET6_ADDRSTRLEN];
				inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
				std::cerr << "Server: got connection from " << s << std::endl;
			}


		}
		else
		{
			accept_fd = accept(server_sock_local, NULL, NULL);
		}


		if (accept_fd > 0)
		{
			//This is needed in order to avoid having the client socket overwritten from other request
			p_clientSock = new(std::nothrow) int;

			if(p_clientSock != nullptr)
			{
				*p_clientSock = accept_fd;
			}

		}


		if(DEBUG && p_clientSock != nullptr)
		{
			std::cerr << "^^^handleClient: p_clientSock " << *p_clientSock << "\n" << std::endl;
		}

		if(p_clientSock != nullptr)
		{
			//check in which mode the server runs
			if(mode == Mode::ThreadPool)
			{
				if(DEBUG)
				{
					std::cerr << "server runs in ThreadPool mode" << std::endl;
				}
				if(workers!=nullptr)
				{
					(*workers).addJob([=] () { handleClient((void*)p_clientSock); });
				}
				else
				{
					if(DEBUG)
					{
						std::cerr << "Something went terribly wrong, ThreadPool is null" << std::endl;
					}
					exit(-1);
				}
				
			}
			else
			{
				if(DEBUG)
				{
					std::cerr << "server runs in THreadPerReq mode" << std::endl;
				}

				pthread_t th_conn;

				if ((pthread_create(&th_conn, NULL, &handleClient, p_clientSock)) == 0)
				{
					if(DEBUG)
					{
						std::cerr << "handleClient: A new thread connection created. Socket:" << *p_clientSock << std::endl;
					}

					//This is needed to free the thread resources but pthread_detach is faster,
					//we don't want to wait so go with that
					//pthread_join(clientThread, NULL);
					pthread_detach(th_conn);

				}
				else
				{
					std::perror("handleClient: pthread_create: fail");
					close(*p_clientSock);
					delete (int*)p_clientSock;

				}
			}


		}
		else
		{
			if(accept_fd > 0)
			{
				close(accept_fd);
				std::cerr << "handleClient: failed to allocate memory, closing the socket" << std::endl;
			}

			std::perror("Server: accept");
		}

	}
}

//not the best practice but currently it does the job
static void closeConn(int clientSocket)
{
	shutdown(clientSocket,SHUT_RDWR);
	close(clientSocket);
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}
