/*
 * HttpRequest.cpp
 *
 *  Created on: May 28, 2020
 *      Author: voukatas
 */


#include <cstdio>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <errno.h>
#include <cstring>

#include "HttpRequest.h"
#include "config.h"
#include "Logger/Logger.h"
#include "Logger/loglvl.h"


HttpRequest::HttpRequest(int clientSocket):
clientSocket{clientSocket}{}

int HttpRequest::readData(void)
{
	char buffer[MAXDATASIZE] = { 0 };
	int numbytes;

	//wait data from client
	numbytes = recv(clientSocket, buffer, MAXDATASIZE - 1, 0);
	if ( numbytes <= 0)
	{
		//peer has performed an orderly shutdown
		if( numbytes == 0)
		{
			LoggerSpace::Logger::instance().log(Loglvl::INFO,"Server: recv peer has performed an orderly shutdown: " + std::string(std::strerror(errno)));
		}
		else
		{
			LoggerSpace::Logger::instance().log(Loglvl::INFO,"Server: recv: " + std::string(std::strerror(errno)));
		}
		//close the socket immediately
		close(clientSocket);
		return -1;// connection closed, nothing to do so return
	}

	//set the last byte to NUL termination string
	buffer[numbytes] = '\0';

	request = std::string(buffer);

	return 0;
}

std::string HttpRequest::getRequest()
{
	return request;
}

std::string HttpRequest::getMethodName()
{
	return method_name;
}

std::string& HttpRequest::getPath()
{
	return path;
}

std::string HttpRequest::getProtocol()
{
	return protocol;
}

int HttpRequest::isReqValid()
{
	//ToDo Check if the method is valid
	//ToDo regex to validae req

	//client ask for the default page
	if(path.empty())
	{
		return 0;
	}
	//check if the path is a file and if not send an error response
	if( !HttpRequest::is_path_file(path) )
	{
		LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"path: "+path+" NOT A FILE\n");
		return -1;
	}

	return 0;
}

void HttpRequest::parseReq()
{
	method_name = std::string(request,0,3);
	std::string path_and_protocol = std::string(request,4);
	std::size_t path_len = path_and_protocol.find(" ");
	path = std::string(path_and_protocol,1,path_len-1);
	protocol = std::string(path_and_protocol,path_len+1,8);

	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent:\n"+request);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent method:"+method_name);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent path:"+path);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent protocol:"+protocol);

}

//checks if the given path is a regular file
bool HttpRequest::is_path_file(std::string path_value)
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

void HttpRequest::setRequest(std::string req)
{
	request = req;
}
