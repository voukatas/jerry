/*
 * @Author: Stylianos Voukatas 
 * @Date: 2020-06-14 22:37:00 
 * @Last Modified by: Stylianos Voukatas
 * @Last Modified time: 2020-06-16 11:16:50
 */
#define CATCH_CONFIG_MAIN
//#define DATEINDX 25
#define LOG_LVL_TAG_INDX 25
#define THREAD_MSG_LEN 30
#define MSG_INDX(INDX) LOG_LVL_TAG_INDX+THREAD_MSG_LEN+INDX

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "catch.hpp"
#include "../handlers.h"
#include "../HttpRequest.h"
#include "../util.h"
#include "../ThreadPool.h"
#include "../Logger/Logger.h"
#include "../Logger/loglvl.h"


void checkMsg(int index, std::string str,int strlen, int recordNum);
void readLineFromFileException();

TEST_CASE( "GET Method Tests")
{
	LoggerSpace::Logger::instance().setLogLvl(Loglvl::NONE);
	
	int clientSocket{4};
	HttpRequest httpRequest0{clientSocket};
	httpRequest0.setRequest("GET /test HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nSec-Fetch-Mode: no-cors");
	httpRequest0.parseReq();

	HttpRequest httpRequest1{clientSocket};
	httpRequest1.setRequest("GET /favicon.ico HTTP/1.1");
	httpRequest1.parseReq();

	HttpRequest httpRequest2{clientSocket};
	httpRequest2.setRequest("GET / HTTP/1.1");
	httpRequest2.parseReq();

	HttpRequest httpRequest3{clientSocket};
	httpRequest3.setRequest("GET /../index.html HTTP/1.1");
	httpRequest3.parseReq();


//	HttpRequest httpRequest3.setRequest("GET/ HTTP/1.1");//malformed
//	HttpRequest httpRequest4.setRequest("GET /HTTP/1.1");//malformed
//	HttpRequest httpRequest5.setRequest("GET/HTTP/1.1");//malformed
//	HttpRequest httpRequest6.setRequest("GET  / HTTP/1.1");//malformed
//	HttpRequest httpRequest7.setRequest("HTTP/1.1");//malformed
//	HttpRequest httpRequest8.setRequest("GET / HTTP");//malformed

	SECTION("Get request")
	{
		REQUIRE(httpRequest0.getRequest() == "GET /test HTTP/1.1\nHost: localhost:8080\nConnection: keep-alive\nSec-Fetch-Mode: no-cors");
		REQUIRE(httpRequest1.getRequest() == "GET /favicon.ico HTTP/1.1");
		REQUIRE(httpRequest2.getRequest() == "GET / HTTP/1.1");
	}

	SECTION("Get method name")
	{
		REQUIRE(httpRequest0.getMethodName() == "GET");
		REQUIRE(httpRequest1.getMethodName() != "gET");
		REQUIRE(httpRequest2.getMethodName() == "GET");
	}

	SECTION("Get path")
	{
		REQUIRE(httpRequest0.getPath() == "test");
		REQUIRE(httpRequest1.getPath() == "favicon.ico");
		REQUIRE(httpRequest2.getPath() == "");
	}

	SECTION("Get protocol")
	{
		REQUIRE(httpRequest0.getProtocol() == "HTTP/1.1");
		REQUIRE(httpRequest0.getProtocol() != "HTTP/");
	}

	SECTION("Get fields")
	{
		REQUIRE(httpRequest0.getFieldMap()["Host"] == "localhost:8080");
		REQUIRE(httpRequest0.getFieldMap()["Connection"] == "keep-alive");
		REQUIRE(httpRequest0.getFieldMap()["Sec-Fetch-Mode"] == "no-cors");
	}

	SECTION("Validate if regular file")
	{
		REQUIRE(httpRequest0.isReqValid() == -1);
		REQUIRE(httpRequest3.isReqValid() == 0);//true, cause 0 is success

	}


}

TEST_CASE( "Logger Tests")
{	
	SECTION("Setter Getter")
	{
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::ERROR);
		REQUIRE(LoggerSpace::Logger::instance().getLogLvl() == Loglvl::ERROR);
	}
	
	SECTION("Test different loglvls")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::ERROR);		
		LoggerSpace::Logger::instance().log(Loglvl::TRACE,"trace");
		
		readLineFromFileException();//REQUIRE_THROWS
	}

	SECTION("Test LogLvl Trace")
	{
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::TRACE);		
		
		LoggerSpace::Logger::instance().log(Loglvl::TRACE,"jerry_TRACE");		
		checkMsg(LOG_LVL_TAG_INDX,"[TRACE]",7,0);//7 chars since we have [TRACE]
		checkMsg(MSG_INDX(8),"jerry_TRACE",11,0);//LOGLVLTAG+7+1 is the message index//+1 for the space
		
	}

	SECTION("Test LogLvl INFO with higher loglevel set")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::TRACE);		
		
		LoggerSpace::Logger::instance().log(Loglvl::INFO,"jerry_INFO");
		checkMsg(LOG_LVL_TAG_INDX,"[INFO]",6,1);//7 chars since we have [TRACE]
		checkMsg(MSG_INDX(7),"jerry_INFO",10,1);//LOGLVLTAG+6+1 is the message index//+1 for the space
		
	}

	SECTION("Test LogLvl WARN")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::WARN);		
		
		LoggerSpace::Logger::instance().log(Loglvl::WARN,"jerry_WARN");
		checkMsg(LOG_LVL_TAG_INDX,"[WARN]",6,2);//7 chars since we have [WARN]
		checkMsg(MSG_INDX(7),"jerry_WARN",10,2);//LOGLVLTAG+6+1 is the message index//+1 for the space
		
	}

	SECTION("Test LogLvl DEBUG")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::DEBUG);		
		
		LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"jerry_DEBUG");
		checkMsg(LOG_LVL_TAG_INDX,"[DEBUG]",7,3);//7 chars since we have [DEBUG]
		checkMsg(MSG_INDX(8),"jerry_DEBUG",11,3);//LOGLVLTAG+7+1 is the message index//+1 for the space
		
	}

	SECTION("Test LogLvl ERROR")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::ERROR);		
		
		LoggerSpace::Logger::instance().log(Loglvl::ERROR,"jerry_ERROR");
		checkMsg(LOG_LVL_TAG_INDX,"[ERROR]",7,4);//7 chars since we have [ERROR]
		checkMsg(MSG_INDX(8),"jerry_ERROR",11,4);//LOGLVLTAG+7+1 is the message index//+1 for the space
		
	}

	SECTION("Test LogLvl FATAL")
	{
		
		LoggerSpace::Logger::instance().setLogLvl(Loglvl::FATAL);
		
		int result = -1;
		LoggerSpace::Logger::instance().log(Loglvl::FATAL,"jerry_FATAL:"+std::to_string(result));
		checkMsg(LOG_LVL_TAG_INDX,"[FATAL]",7,5);//7 chars since we have [FATAL]
		checkMsg(MSG_INDX(8),"jerry_FATAL:-1",14,5);//LOGLVLTAG+7+1 is the message index//+1 for the space
		
	}

}


void checkMsg(int index, std::string str,int strlen, int recordNum)
{
	std::ifstream ifs("jerry.log");
	std::string line;
	int counter = 0;
	
	while (std::getline(ifs, line))
	{
		if(counter == recordNum)
		{
			//std::getline(ifs, line);
			//std::cout<<line<<std::endl;
			REQUIRE( line.substr(index, strlen) == str);
			break;
		}
		else
		{
			counter++;
		}
		

	}
}

void readLineFromFileException()
{
	std::ifstream ifs("jerry.log");
	std::string line;
	//while (std::getline(ifs, line))
	//{	
		std::getline(ifs, line);
		//std::cout<<line<<std::endl;
		REQUIRE_THROWS( line.substr(25, 30) != "jerry");
	//}
}












