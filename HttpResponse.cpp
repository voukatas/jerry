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


HttpResponse::HttpResponse(int clientSocket):
clientSocket{clientSocket}{}

int HttpResponse::sendData(std::string html)
{
	const char * c_msg = html.c_str();

	if (send(clientSocket, c_msg, std::strlen(c_msg), 0) == -1)
	{
		std::perror("send");
		return -1;
	}

	if(DEBUG)
	{
		std::cerr << "--------------DATA SEND--------------" << std::endl;
		std::cerr <<"!!!!!!!!!"<< html << std::endl;
	}

	return std::strlen(c_msg);
	//added delay to keep the socket open
	//sleep(1);
}

void HttpResponse::readHtml(const std::string& path, std::string& html_page)
{
	//Read from the specified file

	//std::string html_page;
	std::ifstream file{path};
	if (file.fail())
	{
		if(DEBUG)
		{
			std::cerr << "failed to open file:" << path << "\n" << std::endl;
		}
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
			if(DEBUG)
			{
				std::cerr << "Exception occured:" <<e.what()<< "\n" << std::endl;
			}

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
