#define CATCH_CONFIG_MAIN

#include <iostream>
#include "catch.hpp"
#include "../handlers.h"
#include "../HttpRequest.h"
#include "../util.h"
#include "../ThreadPool.h"


TEST_CASE( "GET Method Tests")
{
	int clientSocket{4};
	HttpRequest httpRequest0{clientSocket};
	httpRequest0.setRequest("GET /test HTTP/1.1");
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
		REQUIRE(httpRequest0.getRequest() == "GET /test HTTP/1.1");
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

	SECTION("Validate if regular file")
	{
		REQUIRE(httpRequest0.isReqValid() == -1);
		REQUIRE(httpRequest3.isReqValid() == 0);//true, cause 0 is success

	}

	SECTION("Validate GET request with regex")
	{
		//ToDo
	}


}





















