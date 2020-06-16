/*
 * util.cpp
 *
 *  Created on: Jun 6, 2020
 *      Author: voukatas
 */


#include <iostream>
#include "util.h"
#include "config.h"
#include "Logger/Logger.h"

void printServerInfo(int server_sock, std::size_t numOfThreads )
{
	LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"Server started : server_sock = "+std::to_string(server_sock));
	
	if( THREADPOOL == 1)
	{
		LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"Server Mode: ThreadPool, Number of Threads:"+std::to_string(numOfThreads));
	}
	else
	{
		LoggerSpace::Logger::instance().log(Loglvl::DEBUG,"Server Mode: Thread per Request");
	}
		
	std::cerr << "Waiting for connections..." << std::endl;
}
//bundle arguments
ListenerArgs::ListenerArgs(int server_sock, void* workers, Mode mode):
	server_sock{server_sock}, workers{workers}, mode{mode}{};

//converts a Loglvl enum to string
std::string ToString(Loglvl loglvl)
{
    switch (loglvl)
    {
        case Loglvl::FATAL:   return "FATAL";
        case Loglvl::ERROR:   return "ERROR";
        case Loglvl::DEBUG:	  return "DEBUG";
		case Loglvl::WARN:	  return "WARN";
		case Loglvl::INFO:	  return "INFO";
		case Loglvl::TRACE:	  return "TRACE";
        default:      		  return "UNKOWN";
    }
}