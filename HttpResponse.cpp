/*
 * HttpResponse.cpp
 *
 *  Created on: May 28, 2020
 *      Author: voukatas
 */

#include <sys/socket.h>
#include <cstdio>
#include <cstring>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <sstream>

#include "HttpResponse.h"
#include "config.h"
#include "Logger/Logger.h"
#include "Logger/loglvl.h"


HttpResponse::HttpResponse(int clientSocket):
clientSocket{clientSocket}{}

int HttpResponse::sendData(std::string html)
{
	const char * c_msg = html.c_str();

	if (send(clientSocket, c_msg, std::strlen(c_msg), 0) == -1)
	{
		LoggerSpace::Logger::instance().log(Loglvl::ERROR,"Server: send: " + std::string(std::strerror(errno)));
		return -1;
	}

	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"DATA SEND:\n"+html);

	return std::strlen(c_msg);
}

void HttpResponse::readHtml(const std::string& path, std::string& html_page)
{
	//Read from the specified file

	//std::string html_page;
	std::ifstream file{path};
	if (file.fail())
	{
		LoggerSpace::Logger::instance().log(Loglvl::ERROR,"failed to open file:"+path);
	    // file could not be opened
		throw std::runtime_error{"file could not be opened"};

	}

	if (file.is_open())
	{
		try
		{
			html_page = read_and_build_html_data(file);
		}
		catch (const std::length_error& e)
		{
			LoggerSpace::Logger::instance().log(Loglvl::ERROR,"Exception occured:"+std::string(e.what()));

			throw std::runtime_error{"length_error"};

		}

	}

	//return html_page;
}

std::string HttpResponse::read_and_build_html_data(std::ifstream& file)
{
	std::string header = "HTTP/1.1 200 OK\nContent-Type: text/html\nContent-Length: ";
	//seekg/tellg are extremely fast to read data
	file.seekg(0, std::ios::end);
	size_t size = file.tellg();

	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"read_and_build_html_data size:"+std::to_string(size));


	std::string buffer(size, ' ');
	file.seekg(0);
	file.read(&buffer[0], size);

	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"default buffer:"+buffer);

	std::stringstream ss;
	ss << header<<size<<"\n\n"<<buffer<<std::endl;

	return ss.str();
}
