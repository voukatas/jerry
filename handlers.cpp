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
#include <errno.h>
#include <cstring>

#include "handlers.h"
#include "HttpRequest.h"
#include "HttpResponse.h"
#include "config.h"
#include "ThreadPool.h"
#include "util.h"
#include "Logger/Logger.h"
#include "Logger/loglvl.h"

static std::string fail_msg = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: 29\n\n<H1>Not a valid Request!</H1>";
static std::string fail_method_msg = "HTTP/1.1 500 Internal Server Error\nContent-Type: text/html\nContent-Length: 29\n\n<H1>Method not supported</H1>";

//close connection
static void closeConn(int clientSocket);
//check returned cause and send failure msg if not success
static int checkCauseAndSentFailure(int cause, HttpResponse res, int clientSocket );

//serve client function
void *handleClient(void* client_sock)
{
	int clientSocket = *((int*)client_sock);
	//free the previously allocated int
	delete (int*)client_sock;

	HttpRequest req{clientSocket};
	HttpResponse res{clientSocket};

	auto cause = req.readData();//0 for success

	if(cause != 0)
	{
		//no need to use closeConn, we do not need to close conn gracefully
		//readData closes the conn immediately
		return NULL;
	}

	req.parseReq();

	cause = req.isReqValid();

	if( checkCauseAndSentFailure(cause,res,clientSocket) != 0 )
	{
		return NULL;// connection closed, nothing to do so return
	}

	std::string path = req.getPath().empty()?"index.html":req.getPath();


	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"handlers: path = "+path);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"handlers: req.getPath().empty() = "+ std::string(req.getPath().empty()?"TRUE":"FALSE"));
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"handlers: req.getPath() = "+req.getPath());

	std::string html_page;

	try
	{
		res.readHtml(path,html_page);
//		html_page = res.readHtml(path);
	}
	catch(const std::exception& e)
	{
		LoggerSpace::Logger::instance().log(Loglvl::ERROR,"handlers: exception = "+std::string(e.what()));		
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
		int accept_fd;


		struct sockaddr_storage their_addr; // connector's address information
		socklen_t sin_size;
		sin_size = sizeof their_addr;

		accept_fd = accept(server_sock_local, (struct sockaddr *) &their_addr, &sin_size);

		if (accept_fd > 0)
		{

			//This is needed in order to avoid having the client socket overwritten from other request
			p_clientSock = new(std::nothrow) int;

			if(p_clientSock != nullptr)
			{
				*p_clientSock = accept_fd;
			}

			char s[INET6_ADDRSTRLEN];
			inet_ntop(their_addr.ss_family,get_in_addr((struct sockaddr *)&their_addr), s, sizeof s);
			LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"Server: got connection from "+std::string(s));	

		}

		if(p_clientSock != nullptr)
		{
			LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"handleClient: p_clientSock "+std::to_string(*p_clientSock ));

			//check in which mode the server runs
			if(mode == Mode::ThreadPool)
			{
				LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"server runs in ThreadPool mode" );

				if(workers!=nullptr)
				{
					(*workers).addJob([=] () { handleClient((void*)p_clientSock); });
				}
				else
				{
					LoggerSpace::Logger::instance().log(Loglvl::FATAL,"Something went terribly wrong, ThreadPool is null" );
					exit(-1);
				}
				
			}
			else
			{
				LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"server runs in ThreadPerReq mode" );

				pthread_t th_conn;

				if ((pthread_create(&th_conn, NULL, &handleClient, p_clientSock)) == 0)
				{
					LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"handleClient: A new thread connection created. Socket:"+std::to_string(*p_clientSock) );

					//There is no I/O in files so we do not care much if app terminates, thread should clear it's own resources
					pthread_detach(th_conn);

				}
				else
				{					
					LoggerSpace::Logger::instance().log(Loglvl::ERROR,"handleClient: pthread_create: fail" + std::string(std::strerror(errno)));
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
				LoggerSpace::Logger::instance().log(Loglvl::ERROR,"handleClient: failed to allocate memory, closing the socket");				
			}

			LoggerSpace::Logger::instance().log(Loglvl::ERROR,"Server: accept " + std::string(std::strerror(errno)));
			//std::perror("Server: accept");
		}

	}
}

//close the connection with the client
static void closeConn(int clientSocket)
{
	shutdown(clientSocket,SHUT_RDWR);
	close(clientSocket);
}

static int checkCauseAndSentFailure(int cause, HttpResponse res, int clientSocket )
{
	if( cause != 0 )
	{
		if(cause == -2)
		{
			res.sendData(fail_method_msg);			
		}
		else
		{
			res.sendData(fail_msg);
		}
		
		
		closeConn(clientSocket);		
	}

	return cause;	
}

void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}
