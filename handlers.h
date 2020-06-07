/*
 * handlers.h
 *
 *  Created on: May 13, 2020
 *      Author: voukatas
 */
#pragma once

#include "ThreadPool.h"
#include "util.h"


void *get_in_addr(struct sockaddr *sa);//socket address IPv4 or IPv6
void *handleClient(void* client_sock);
void *listener(void* args);


