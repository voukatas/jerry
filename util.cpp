/*
 * util.cpp
 *
 *  Created on: Jun 6, 2020
 *      Author: voukatas
 */


#include <iostream>
#include "util.h"
#include "config.h"

void printServerInfo(int server_sock, std::size_t numOfThreads )
{
	std::cerr << "Server started : server_sock = " << server_sock << std::endl;
	if( THREADPOOL == 1)
	{
		std::cout<<"Server Mode: ThreadPool, Number of Threads:"<<numOfThreads<<"\n"<<std::endl;
	}
	else
	{
		std::cout<<"Server Mode: Thread per Request" <<std::endl;
	}
		
	std::cerr << "Waiting for connections..." << std::endl;
}

ListenerArgs::ListenerArgs(int server_sock, void* workers, Mode mode):
	server_sock{server_sock}, workers{workers}, mode{mode}{};