/*
 * handlers.c
 *
 *  Created on: May 13, 2020
 *      Author: voukatas
 */
#include <stdio.h>
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
	free(client_sock);


	char buffer[MAXDATASIZE] = { 0 };
	int numbytes;
	char *msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 21\n\n<H1>Hello World!</H1>";


	//wait from the client data
	if ((numbytes = recv(client_sock_local, buffer, MAXDATASIZE - 1, 0)) == -1)
	{
		perror("Server: recv");
		close(client_sock_local);
		return NULL;// connection closed, nothing to do so return
	}
	else
	{
		//set the last byte to NUL termination string
		buffer[numbytes] = '\0';

		if(DEBUG)
		{
			fprintf(stderr, "client sent: %s\n\n",buffer);
		}
	}


	//send to client data
	if (send(client_sock_local, msg, strlen(msg), 0) == -1)
	{
		perror("send");
	}
	else
	{
		if(DEBUG)
		{
			fprintf(stderr, "-----------DATA SEND------------------\n");
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

		int* p_clientSock = NULL;
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
				fprintf(stderr, "Server: got connection from %s\n\n", s);
			}


		}
		else
		{
			accept_fd = accept(server_sock_local, NULL, NULL);
		}


		if (accept_fd > 0)
		{
			//This is needed in order to avoid having the client socket overwritten from other request
			p_clientSock = (int*)malloc(sizeof (int));
			*p_clientSock = accept_fd;
		}


		if(DEBUG && accept_fd > 0)
		{
			fprintf(stderr, "^^^handleClient: p_clientSock %p\n\n", p_clientSock);
		}

		if ((p_clientSock != NULL) && (*p_clientSock > 0) )
		{
			pthread_t th_conn;

			if ((pthread_create(&th_conn, NULL,(void *) &tClient, (void *)p_clientSock)) == 0)
			{
				if(DEBUG)
				{
					fprintf(stderr, "handleClient: A new thread connection created. Socket: %d\n",	*p_clientSock);
				}

				//This is needed to free the thread resources but pthread_detach is faster,
				//we don't want to wait so go with that
				//pthread_join(clientThread, NULL);
				pthread_detach(th_conn);

			}
			else
			{
				perror("handleClient: pthread_create: fail");
				close(*p_clientSock);
			}

		}
		else
		{
			perror("Server: accept");
			//close(*p_clientSock);
		}

	}
}
