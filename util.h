/*
 * util.h
 *
 *  Created on: Jun 6, 2020
 *      Author: voukatas
 */

#ifndef UTIL_H
#define UTIL_H

#include <cstddef>

#include "mode.h"

void printServerInfo(int server_sock, std::size_t numOfThreads );

class ListenerArgs
{   
    public:

    int server_sock;
	void* workers;//type ThreadPool
	Mode mode;    

	ListenerArgs(int server_sock, void* workers, Mode mode);
	
};


#endif /* UTIL_H */
