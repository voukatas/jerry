/*
 * HttpParser.cpp
 *
 *  Created on: May 14, 2020
 *      Author: voukatas
 */

#include<iostream>
#include "HttpParser.h"


HttpParser::HttpParser(const char *get_req):
		req{std::string(get_req)}
{
	method_name = std::string(req,0,3);
	std::string path_and_protocol = std::string(req,4);
	std::size_t path_len = path_and_protocol.find(" ");
	path = std::string(path_and_protocol,1,path_len-1);
	protocol = std::string(path_and_protocol,path_len+1);
}

std::string HttpParser::getRequest()
{
	return req;
}

std::string HttpParser::getMethodName()
{
	return method_name;
}

std::string HttpParser::getPath()
{
	return path;
}

std::string HttpParser::getProtocol()
{
	return protocol;
}

bool HttpParser::isReqValid()
{
	//regex on the request
	//also check the extension .html
	return true;
	//ToDo
}
