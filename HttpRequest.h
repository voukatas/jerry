/*
 * HttpRequest.h
 *
 *  Created on: May 28, 2020
 *      Author: voukatas
 */

#pragma once

#include <sstream> 
#include <map>
class HttpRequest
{
private:
	int clientSocket;
	std::istringstream request;
	std::string method_name;
	std::string path;
	std::string protocol;
	int parseReqFirstLine(std::string& line);
	int parseReqFields(std::string& line);
	std::map<std::string, std::string> fieldMap;

public:
	explicit HttpRequest(int clientSocket);
	virtual ~HttpRequest() = default;
	int readData();
	void parseReq();
	std::string getRequest();
	std::string getMethodName();
	std::string& getPath();
	std::string getProtocol();
	int isReqValid();
	static bool is_path_file(std::string path);
	//for testing purposes, maybe include a mocking framework later
	void setRequest(std::string req);
	std::map<std::string, std::string>& getFieldMap();

};
