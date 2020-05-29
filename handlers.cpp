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

#include "handlers.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "config.h"


static std::string fail_msg = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: 29\n\n<H1>Not a valid Request!</H1>";

//close connection
static void closeConn(int clientSocket);
//socket address IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa);

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
		res.readHtml(path,&html_page);
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

//accept connections thread
void *listener(void* server_sock_val)
{

	//needs casting since it is void
	int server_sock_local = *((int*)server_sock_val);


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
			char s[INET6_ADDRSTRLEN];

			accept_fd = accept(server_sock_local, (struct sockaddr *) &their_addr, &sin_size);

			if (accept_fd > 0)
			{
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
			p_clientSock = new int;
			*p_clientSock = accept_fd;
		}


		if(DEBUG && accept_fd > 0)
		{
			std::cerr << "^^^handleClient: p_clientSock " << *p_clientSock << "\n" << std::endl;
		}

		if ((p_clientSock != nullptr) && (*p_clientSock > 0) )
		{
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

				if(p_clientSock != nullptr)
				{
					delete (int*)p_clientSock;
				}
			}

		}
		else
		{
			if(p_clientSock != nullptr)
			{
				delete (int*)p_clientSock;
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
