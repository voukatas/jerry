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
#include <vector>

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

	//request = std::string(buffer);
	request.str(std::string(buffer));

	return 0;
}

std::string HttpRequest::getRequest()
{
	return request.str();
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
	//Only GET method is currently supported
	if(method_name != "GET")
	{
		return -2;
	}

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
	//ToDo: check for failures
	std::string line;
	std::getline(request, line);

	HttpRequest::parseReqFirstLine(line);

	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent:\n"+request.str());
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client line:\n" + line);
	
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent method:"+method_name);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent path:"+path);
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"client sent protocol:"+protocol);



	while(std::getline(request, line))
	{
		HttpRequest::parseReqFields(line);
	}	

}

void HttpRequest::parseReqFirstLine(std::string &line)
{
	std::vector<std::string> splitedLine;
	
	std::string delimiter = " ";

	size_t pos = 0;
	std::string token;
	while ((pos = line.find(delimiter)) != std::string::npos)
	{
		token = line.substr(0, pos);
		splitedLine.push_back(token);
		line.erase(0, pos + delimiter.length());
	}
	splitedLine.push_back(line);

	method_name = splitedLine.at(0);
	path = splitedLine.at(1).erase(0,1);//remove the /
	protocol = splitedLine.at(2);

}

void HttpRequest::parseReqFields(std::string& line)
{
	std::stringstream ss(line);
    std::string s;

	if(line == "\r" || line == "")
	{
		return;
	}
	
	std::string delimiter = ":";
	std::string field = line.substr(0, line.find(delimiter));
	std::string value = line.substr(line.find(delimiter)+2);

	fieldMap[field] = value;
	
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"parseReqFields: field="+field+" value="+fieldMap[field]);
	
}

std::map<std::string, std::string>& HttpRequest::getFieldMap()
{
	return fieldMap;
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
	request.str(req);
}
