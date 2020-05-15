/*
 * HttpParser.h
 *
 *  Created on: May 14, 2020
 *      Author: voukatas
 */
#pragma once

class HttpParser
{
private:
	std::string req;
	std::string method_name;
	std::string path;
	std::string protocol;
public:
	HttpParser(const char *get_req);
	
	std::string getRequest();
	std::string getMethodName();
	std::string getPath();
	std::string getProtocol();
	bool isReqValid();

};
