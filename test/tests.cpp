#define CATCH_CONFIG_MAIN

#include <iostream>
#include "catch.hpp"
#include "../handlers.h"
#include "../HttpParser.h"


TEST_CASE( "GET Method Tests")
{
	HttpParser parse_req0{"GET /test HTTP/1.1"};
	HttpParser parse_req1{"GET /favicon.ico HTTP/1.1"};
	HttpParser parse_req2{"GET / HTTP/1.1"};
	HttpParser parse_req3{"GET/ HTTP/1.1"};//malformed
	HttpParser parse_req4{"GET /HTTP/1.1"};//malformed
	HttpParser parse_req5{"GET/HTTP/1.1"};//malformed
	HttpParser parse_req6{"GET  / HTTP/1.1"};//malformed
	HttpParser parse_req7{"HTTP/1.1"};//malformed
	HttpParser parse_req8{"GET / HTTP"};//malformed

	SECTION("Get method name")
	{
		REQUIRE(parse_req0.getRequest() == "GET /test HTTP/1.1");
		REQUIRE(parse_req1.getRequest() == "GET /favicon.ico HTTP/1.1");
		REQUIRE(parse_req1.getRequest() != "GET/favicon.ico HTTP/1.1");
		REQUIRE(parse_req2.getRequest() == "GET / HTTP/1.1");
		REQUIRE(parse_req3.getRequest() != "GET / HTTP/1.1");

	}

	SECTION("Get method name")
	{
		REQUIRE(parse_req0.getMethodName() == "GET");
		REQUIRE(parse_req0.getMethodName() != "gET");
		REQUIRE(parse_req3.getMethodName() == "GET");
	}

	SECTION("Get path")
	{
		REQUIRE(parse_req0.getPath() == "test");
		REQUIRE(parse_req1.getPath() == "favicon.ico");
		REQUIRE(parse_req2.getPath() == "");
	}

	SECTION("Get protocol")
	{
		REQUIRE(parse_req0.getProtocol() == "HTTP/1.1");
		REQUIRE(parse_req0.getProtocol() != "HTTP/");
	}

	SECTION("Validate")
	{
//		REQUIRE(parse_req3.isReqValid() == false);
//		REQUIRE(parse_req4.isReqValid() == false);
//		REQUIRE(parse_req5.isReqValid() == false);
//		REQUIRE(parse_req6.isReqValid() == false);
//		REQUIRE(parse_req7.isReqValid() == false);
//		REQUIRE(parse_req8.isReqValid() == false);
		REQUIRE(parse_req0.isReqValid() == true);
		REQUIRE(parse_req1.isReqValid() == true);
		REQUIRE(parse_req2.isReqValid() == true);
	}


}





















