CPPFLAGS=-std=c++17 -Wall -Wextra -ggdb3

all: jerry tests

tests: test/catch.hpp test/tests.cpp config.h handlers.h HttpRequest.h handlers.o HttpRequest.o HttpResponse.o test.o
	g++ HttpRequest.o HttpResponse.o handlers.o test/tests.o -lpthread -o test/tests

test.o: test/catch.hpp test/tests.cpp
	g++ $(CPPFLAGS) -c test/tests.cpp -o test/tests.o
	

jerry: handlers.o HttpRequest.o HttpResponse.o config.h jerry.o
	g++ HttpRequest.o HttpResponse.o handlers.o jerry.o -lpthread -o jerry_the_http_server

handlers.o: handlers.cpp handlers.h config.h
	g++ $(CPPFLAGS) -c handlers.cpp 

HttpRequest.o: HttpRequest.cpp HttpRequest.h config.h
	g++ $(CPPFLAGS) -c HttpRequest.cpp

HttpResponse.o: HttpResponse.cpp HttpResponse.h config.h
	g++ $(CPPFLAGS) -c HttpResponse.cpp

jerry.o: jerry.cpp config.h
	g++ $(CPPFLAGS) -c jerry.cpp
     
runtests: tests
	cd test;./tests

clean:
	 rm handlers.o HttpRequest.o HttpResponse.o jerry.o test/tests.o jerry_the_http_server test/tests
