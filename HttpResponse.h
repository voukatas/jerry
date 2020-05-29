/*
 * HttpResponse.h
 *
 *  Created on: May 28, 2020
 *      Author: voukatas
 */

#pragma once
class HttpResponse
{
private:
	int clientSocket;


public:
	HttpResponse(int clientSocket);
	virtual ~HttpResponse() = default;
	int sendData(std::string html);
	void readHtml(const std::string path, std::string* html_page);
	static std::string read_and_build_html_data(std::ifstream& file);
	//ToDo Add method to set the status of the response

};

