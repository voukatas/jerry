/*
 * handlers.c
 *
 *  Created on: May 13, 2020
 *      Author: voukatas
 */
#include <iostream>
#include <cstdio>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>

#include "handlers.h"
#include "config.h"

// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}


void *tClient(void* client_sock)
{
	int client_sock_local = *((int*)client_sock);
	//free the previously allocated int
	delete (int*)client_sock;


	char buffer[MAXDATASIZE] = { 0 };
	int numbytes;
	std::string msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 21\n\n<H1>Hello World!</H1>";

	const char * c_msg = msg.c_str();


	//wait data from the client
	if ((numbytes = recv(client_sock_local, buffer, MAXDATASIZE - 1, 0)) == -1)
	{
		std::perror("Server: recv");
		close(client_sock_local);
		return NULL;// connection closed, nothing to do so return
	}
	else
	{
		//set the last byte to NUL termination string
		buffer[numbytes] = '\0';

		if(DEBUG)
		{
			std::cerr << "client sent: " << buffer << std::endl;
		}
	}


	//send data to client
	if (send(client_sock_local, c_msg, strlen(c_msg), 0) == -1)//if (send(client_sock_local, msg, strlen(msg), 0) == -1)
	{
		std::perror("send");
	}
	else
	{
		if(DEBUG)
		{
			std::cerr << "--------------DATA SEND--------------" << std::endl;
		}
	}

	//close the connection
	close(client_sock_local);

}

void *handleClient(void* server_sock_val)
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

			if ((pthread_create(&th_conn, NULL, &tClient, p_clientSock)) == 0)
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
				if(p_clientSock != nullptr)
				{
					delete (int*)p_clientSock;
				}
				std::perror("handleClient: pthread_create: fail");
				close(*p_clientSock);
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
