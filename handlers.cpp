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
#include <fstream>
#include <streambuf>
#include <sstream>
#include <sys/stat.h>
#include <exception>

#include "handlers.h"
#include "HttpParser.h"
#include "config.h"


static void sendData(int client_sock_local, std::string msg );
static std::string read_and_build_html_data(std::ifstream& file) throw(std::length_error);
static bool is_path_file(std::string path);


static std::string fail_msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 29\n\n<H1>Not a valid Request!</H1>";
static std::string header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";

//socket address IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)
{
	if (sa->sa_family == AF_INET)
	{
		return &(((struct sockaddr_in*) sa)->sin_addr);
	}

	return &(((struct sockaddr_in6*) sa)->sin6_addr);
}

//serve client function
void *tClient(void* client_sock)
{
	int client_sock_local = *((int*)client_sock);
	//free the previously allocated int
	delete (int*)client_sock;


	char buffer[MAXDATASIZE] = { 0 };
	int numbytes;
	//std::string msg = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: 21\n\n<H1>Hello World!</H1>";

	std::string html_page;

	//wait data from the client
	numbytes = recv(client_sock_local, buffer, MAXDATASIZE - 1, 0);
	if ( numbytes <= 0)
	{
		//peer has performed an orderly shutdown
		if( numbytes == 0)
		{
			std::perror("Server: recv peer has performed an orderly shutdown ");
		}
		else
		{
			std::perror("Server: recv");
		}
		close(client_sock_local);
		return NULL;// connection closed, nothing to do so return
	}

	//set the last byte to NUL termination string
	buffer[numbytes] = '\0';

	HttpParser parse_req{buffer};

	if(DEBUG)
	{
		std::cerr << "isReqValid:" << parse_req.isReqValid() << "\n" << std::endl;
		std::cerr << "client sent: " << parse_req.getRequest() << "\n" << std::endl;
		std::cerr << "client sent method:" << parse_req.getMethodName() << std::endl;
		std::cerr << "client sent path:" << parse_req.getPath() << std::endl;
		std::cerr << "client sent path isempty:" << parse_req.getPath().empty() << std::endl;
		std::cerr << "client sent protocol: " << parse_req.getProtocol()<<"\n\n" << std::endl;
	}

	//ToDo isReqValid
	//In case of a http request directly on folder (eg. test or / )  will create a core dump because seekg/tellg
	//will be invalid so the size will be invalid. Either change seekg/tellg or guarded properly, maybe to restrict rights
	//Currently to guard from the core dump a restriction for the "test" folder is used but needs fixing!!!
	if( !parse_req.isReqValid() )
	{
		//ToDo Send a proper Response

		sendData( client_sock_local, fail_msg );
		close(client_sock_local);
		return NULL;// connection closed, nothing to do so return
	}

	//check if the path is a file and if not send an error response
	if( !is_path_file(parse_req.getPath()) )
	{
		//ToDo Send a proper Response

		if(DEBUG)
		{
			std::cerr << "path:" << parse_req.getPath() << " NOT A FILE\n" << std::endl;
		}

		sendData( client_sock_local, fail_msg );
		close(client_sock_local);
		return NULL;// connection closed, nothing to do so return
	}

	if(!parse_req.getPath().empty())
	{
		//Read from the specified file

		std::ifstream file{parse_req.getPath()};
		if (file.fail())
		{
			if(DEBUG)
			{
				std::cerr << "failed to open file:" << parse_req.getPath() << "\n" << std::endl;
			}
		    // file could not be opened
			sendData( client_sock_local, fail_msg );
			close(client_sock_local);
			return NULL;// connection closed, nothing to do so return

		}

		if (file.is_open())
		{
			try
			{
				html_page = read_and_build_html_data(file);
			}
			catch (const std::length_error& e)
			{
				if(DEBUG)
				{
					std::cerr << "Exception occured:" <<e.what()<< "\n" << std::endl;
				}
				sendData( client_sock_local, fail_msg );
				close(client_sock_local);
				return NULL;// connection closed, nothing to do so return

			}

		}

	}


	//Client Requested the default path
	if(parse_req.getPath().empty())
	{
		//Read from index.html
		if(DEBUG)
		{
			std::cerr << "Client Requested the default path:" << "\n" << std::endl;
		}

		std::ifstream index{"index.html"};

		if (index.fail())
		{
			if(DEBUG)
			{
				std::cerr << "failed to open file:" << "index.html" << "\n" << std::endl;
			}
		    // file could not be opened
			sendData( client_sock_local, fail_msg );
			close(client_sock_local);
			return NULL;// connection closed, nothing to do so return

		}

		if (index.is_open())
		{
			try
			{
				html_page = read_and_build_html_data(index);
			}
			catch (const std::length_error& e)
			{
				if(DEBUG)
				{
					std::cerr << "Exception occured(index):" <<e.what()<< "\n" << std::endl;
				}
				sendData( client_sock_local, fail_msg );
				close(client_sock_local);
				return NULL;// connection closed, nothing to do so return

			}

		}


	}

	//send data to client
	sendData( client_sock_local, html_page );

	//close the connection
	close(client_sock_local);

}

//the function that the thread that does the accept executes
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

std::string read_and_build_html_data(std::ifstream& file) throw(std::length_error)
{
	//seekg/tellg are extremely fast to read data
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();

	if(DEBUG)
	{
		std::cerr << "------read_and_build_html_data size:" << size<< std::endl;
	}


	std::string buffer(size, ' ');
	file.seekg(0);
	file.read(&buffer[0], size);

	if(DEBUG)
	{
		std::cerr << "------default buffer:" << buffer << std::endl;
	}

	std::stringstream ss;
	ss << header<<size<<"\n\n"<<buffer<<std::endl;

	return ss.str();

}

void sendData(int client_sock_local, std::string msg )
{
	const char * c_msg = msg.c_str();

	if (send(client_sock_local, c_msg, strlen(c_msg), 0) == -1)
		{
			std::perror("send");
		}
		else
		{
			if(DEBUG)
			{
				std::cerr << "--------------DATA SEND--------------" << std::endl;
				std::cerr <<"!!!!!!!!!"<< msg << std::endl;
			}
		}
	//added delay to keep the socket open
	sleep(1);
}

//checks if the given path is a regular file
bool is_path_file(std::string path_value)
{
	const char * path = path_value.c_str();
	struct stat s;
	if( stat(path,&s) == 0 )
	{
		if( s.st_mode & S_IFREG )//S_IFDIR//for directory
		{
			return true;
		}
	}
	return false;

}
