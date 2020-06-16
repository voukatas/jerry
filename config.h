/*
 * config.h
 *
 *  Created on: May 13, 2020
 *      Author: voukatas
 */
#pragma once

#include "Logger/loglvl.h"

#define PORT "8080"  // the port users will be connecting to
#define BACKLOG 20	 // how many pending connections the queue will hold
#define MAXDATASIZE 200 // the maximum number of data tha we can receive from the client
#define THREADPOOL 1 //set this to 1 to use the thread pool else 0 to use the original implementation ( spawn a new thread per request )
#define LOGLEVEL Loglvl::ERROR //values to select: FATAL, ERROR, DEBUG, WARN, INFO, TRACE, NONE