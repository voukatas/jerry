CPPFLAGS=-std=c++17 -Wall -Wextra -ggdb3

all: jerry tests staticanalysis

tests: handlers.o HttpRequest.o HttpResponse.o ThreadPool.o test.o
	g++ HttpRequest.o HttpResponse.o handlers.o test/tests.o ThreadPool.o -lpthread -o test/tests

test.o: test/catch.hpp test/tests.cpp handlers.h HttpRequest.h config.h
	g++ $(CPPFLAGS) -c test/tests.cpp -o test/tests.o
	

jerry: handlers.o HttpRequest.o HttpResponse.o jerry.o ThreadPool.o util.o
	g++ HttpRequest.o HttpResponse.o handlers.o ThreadPool.o jerry.o util.o -lpthread -o jerry_the_http_server

util.o: util.h util.cpp config.h
	g++ $(CPPFLAGS) -c util.cpp

ThreadPool.o: ThreadPool.h ThreadPool.cpp config.h
	g++ $(CPPFLAGS) -c ThreadPool.cpp

handlers.o: handlers.cpp handlers.h config.h HttpRequest.h HttpResponse.h ThreadPool.h util.h
	g++ $(CPPFLAGS) -c handlers.cpp 

HttpRequest.o: HttpRequest.cpp HttpRequest.h config.h
	g++ $(CPPFLAGS) -c HttpRequest.cpp

HttpResponse.o: HttpResponse.cpp HttpResponse.h config.h
	g++ $(CPPFLAGS) -c HttpResponse.cpp

jerry.o: jerry.cpp config.h ThreadPool.h util.h handlers.h
	g++ $(CPPFLAGS) -c jerry.cpp
     
runtests: tests
	cd test;./tests

staticanalysis: jerry
	cppcheck --enable=all -itest .

clean:
	 rm handlers.o HttpRequest.o HttpResponse.o jerry.o test/tests.o jerry_the_http_server test/tests ThreadPool.o util.o
