# Jerry

A basic skeleton for a Multi-threading HTTP Server with Blocking I/O that prints a "Hello World!" message to the clients

## Getting Started

First you need to know that the name of the project come from (it seems ironically but it is true) a cat

Second this is just for fun and I did it to familiarize myself with the C/C++ language and briefly understand how networking and threads work with it.

It is build with GCC and Linux, I don't know if this works on other machines (Windows, MSVC etc..)

### Architecture
First I wrote it with each request having its own process ( a lot of forking ) and then I thought, why not threads?! So...

The main() thread initiates the sever socket (creates, binds and setup the listener) and then creates a separate thread that accepts the clients. 
Then the thread that does the accept, spawns a thread for each client that does the handling for them( actually waits for the request and responds with a hello world message)

This is just a design I pick to reach my goals. Other possibly more efficient designs could be async I/O with threads, blocking I/O thread pools, separate process to track, non-blocking multiplexing I/O with epoll/poll/select and so on...

By default I have set the debugging messages to off and the port to 8080. In case you want a different setup change the below options in the config.h before compiling.
```
#define PORT "8080" 
#define DEBUG 0     //set this to 1 for debug logs, in case you are going to test with Apache Bench it will be better to leave them disabled
``` 

Now, in order to have the project up and running there is not much work to do just compile and run the source

```
gcc jerry.c handlers.c -lpthread -o jerry_the_http_server
```

or if you want further debugging info

```
gcc jerry.c handlers.c -lpthread -o jerry_the_http_server -ggdb3 //Note that -ggdb3 works only/best with gdb
```
and run it

```
./jerry_the_http_server //or you can name it jerry_the_cat for more fun
```

## Deployment

#### You should never deploy this on a live system it was built just for fun, except if you are insane, in that case it is ok

## Built With

Pure C

## Testing
At this point I should probably add some unit tests...I know,I know TDD etc.. but for now I just benchmark it with [Apache Benchmark](https://httpd.apache.org/docs/2.4/programs/ab.html)

An Example command
```
ab -c 1000 -n 5000000 -r localhost:8080/
```
-c for how many concurrent requests you want. In case you need more requests concurrently, you might need to check and change the limits on your system.
```
ulimit -a // see the limits
ulimit -n 5000 // change the limit of open files( for concurrency) to 5000
```

It might also be helpful to change the limits of the core dump so you can debug it further with gdb (or others) if a core dump happens. (Of course not for our code, but in case a friend needs it)
```
ulimit -c unlimited // change the core dump limit
```

Last tool that might come in handy (again not for us, we write excellent code) is valgrind
```
valgrind --leak-check=full ./jerry_the_http_server
```

Since this is an HTTP Server you might also need to test this in your browser(duh!). In case you haven't change the port, open your browser and write the following ( also it is a good time to compile with the DEBUG flag on (1))
```
http://localhost:8080/
```

## Closure
The whole project was implemented in order to further understand the above mentioned technologies. I decided to share it with everyone in case someone else out there needs to see how all these might work. Comments and fixes are all welcome, since they  will help me further understand how things work.

I am not sure if I ever going to extend this from a basic hello world message.(but I am flirting with a GET/HEAD parser)

## Acknowledgments

Extremely helpful for an induction to networking is [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

