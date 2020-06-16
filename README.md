# Jerry

A Multi-threaded HTTP Server with Blocking I/O

## Getting Started

First you need to know that the name of the project come from (it seems ironically but it is true) a cat

Second this is just for fun and I did it to familiarize myself with the C/C++ language and briefly understand how networking and threads work with it.

It is build with GCC and Linux, I don't know if it works on other machines (Windows, MSVC etc..) and probably it doesn't 

### Architecture
First I wrote it using C and with each request having its own process ( a lot of forking ) and then I thought, why not threads?! So...

You can choose between two modes, both multi-threaded, by configuring the config.h
One mode is using a thread pool (set THREADPOOL = 1 inside config.h) and the other one just spawns a new thread for each request (set THREADPOOL = 0 inside config.h)

The main() thread initiates the sever socket (creates, binds and setup the listener) and then spawns a thread for each client in case thread pool is disabled or if thread pool is enabled just adds the new request to the pool for handling

The server waits for the GET request, which reads partially( ToDo: complete the parser) and responds with the requested contents of the page, if that page exists, else it sends a failure message to the client.

If no specific page is requested (eg. localhost:8080/) the server loads the index.html file

This is just a design I picked to reach my goals. Other possibly designs could be async I/O with threads, non-blocking multiplexing I/O with epoll/poll/select and so on...

I eventually added a [thread pool](https://github.com/voukatas/ThreadPool) that I wrote for this project

By default the logger is set to loglevel TRACE and the port the server runs to 8080. In case you want a different setup change the below options in the config.h before compiling.
```
#define PORT "8080"  // the port users will be connecting to
#define BACKLOG 20	 // how many pending connections the queue will hold
#define MAXDATASIZE 200 // the maximum number of data tha we can receive from the client
#define THREADPOOL 1 //set this to 1 to use the thread pool else 0 to use the original implementation ( spawn a new thread per request )
//in case you are going to test with Apache Bench it will be better to set ERROR
#define LOGLEVEL Loglvl::TRACE //values to select: FATAL, ERROR, DEBUG, WARN, INFO, TRACE, NONE
``` 

Now, in order to have the project up and running there is not much work to do just compile and run the source

build both server and tests
```
make all
./jerry_the_http_server //to run the server
```

build only the server code 
```
make jerry
```

build only tests 
```
make tests
```

run tests 
```
make runtests
```

clean the project
```
make clean
```

run a static analysis with cppcheck on server code only
```
make staticanalysis 
```


### An other way which might not be applicable anymore:

```
g++ -std=c++17 jerry.cpp handlers.cpp HttpRequest.cpp HttpResponse.cpp -lpthread -o jerry_the_http_server
```

or if you want further debugging info

```
g++ -std=c++17 jerry.cpp handlers.cpp HttpRequest.cpp HttpResponse.cpp -lpthread -o jerry_the_http_server -ggdb3 //Note that -ggdb3 works only/best with gdb
```
and run it

```
./jerry_the_http_server //or you can name it jerry_the_cat for more fun
```

## Deployment

#### You should never deploy this on a live system it was built just for fun, except if you are insane, in that case it is ok

## Built With

C/C++

## Testing
At this point I should probably add more unit tests...I know,I know TDD etc.. 

As a unit-testing framework I am using [Catch](https://github.com/catchorg/Catch2) by Phil Nash which is pretty straightforward and excellent!

To run the tests use

```
make runtests
```

or an other way which might not be applicable anymore:

```
cd test
```
```
g++ -std=c++17 ../handlers.cpp ../HttpRequest.cpp ../HttpResponse.cpp tests.cpp -lpthread -o tests
```

For benchmarking I used [Apache Benchmark](https://httpd.apache.org/docs/2.4/programs/ab.html)

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
valgrind --leak-check=full --show-leak-kinds=all ./jerry_the_http_server
```

Since this is an HTTP Server you might also need to test this in your browser(duh!). In case you haven't change the port, open your browser and write the following ( also it is a good time to compile with the DEBUG flag on (1))
```
http://localhost:8080/
```

actually the above request triggers the index.html file, so you can do this also
```
http://localhost:8080/index.html
```

or browse the source code of the project like this
```
http://localhost:8080/jerry.cpp
```

or even create your own html file while the server is running and request this
```
vim myhtml.html// vim is good
write <H1>Hello World from Jerry!!</> and then
http://localhost:8080/myhtml.html
```

## ToDo
1. At this point maybe a makefile is needed

2. Create a better logging system

3. Redesign and actually implement a proper parser

4. Add probably configurable restrictions

5. Add cause code mapping

and many more!!


## Known Issues
1. Jerry will fail if you try to access directly a folder (core dump) like test or /home/ --> fixed

2. There are plenty more issues that needs fixing, but again this is a hobby/fun/learning project

## Valgrind
In case you use valgrind you might see something like the below log. Do not worry, these are not actual leaks.

Issue 1
288 bytes in 1 blocks are possibly lost in loss record 1 of 2
Most probably you stopped the application with a SIGINT (Ctrl-c) (actually that's the only option) so the resources from thread or threads that was running didn't released properly


Issue 2
72,704 bytes in 1 blocks are still reachable in loss record 2 of 2
This is because of <iostream> lib which handles on its own the memory that has allocated and the valgrind sees it as a leak.

Issue 3 
You might see also allocations from static variables/functions these also are not an issue

In case you see something else then... well... It's your fault! No, I am kidding, most probably is an actual leakage and I would be happy(not so much) to let me know


```
==27974== Memcheck, a memory error detector
==27974== Copyright (C) 2002-2015, and GNU GPL'd, by Julian Seward et al.
==27974== Using Valgrind-3.11.0 and LibVEX; rerun with -h for copyright info
==27974== Command: ./jerry_the_http_server
==27974== 
Server started : server_sock = 3
Waiting for connections...
^C==27974== 
==27974== Process terminating with default action of signal 2 (SIGINT)
==27974==    at 0x4E4298D: pthread_join (pthread_join.c:90)
==27974==    by 0x401175: main (jerry.cpp:88)
==27974== 
==27974== HEAP SUMMARY:
==27974==     in use at exit: 72,992 bytes in 2 blocks
==27974==   total heap usage: 112,578 allocs, 112,576 frees, 4,245,596 bytes allocated
==27974== 
==27974== 288 bytes in 1 blocks are possibly lost in loss record 1 of 2
==27974==    at 0x4C2FB55: calloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==27974==    by 0x40138B4: allocate_dtv (dl-tls.c:322)
==27974==    by 0x40138B4: _dl_allocate_tls (dl-tls.c:539)
==27974==    by 0x4E4226E: allocate_stack (allocatestack.c:588)
==27974==    by 0x4E4226E: pthread_create@@GLIBC_2.2.5 (pthread_create.c:539)
==27974==    by 0x401116: main (jerry.cpp:82)
==27974== 
==27974== 72,704 bytes in 1 blocks are still reachable in loss record 2 of 2
==27974==    at 0x4C2DB8F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==27974==    by 0x50F8F85: ??? (in /usr/lib/x86_64-linux-gnu/libstdc++.so.6.0.28)
==27974==    by 0x40106C9: call_init.part.0 (dl-init.c:72)
==27974==    by 0x40107DA: call_init (dl-init.c:30)
==27974==    by 0x40107DA: _dl_init (dl-init.c:120)
==27974==    by 0x4000C69: ??? (in /lib/x86_64-linux-gnu/ld-2.23.so)
==27974== 
==27974== LEAK SUMMARY:
==27974==    definitely lost: 0 bytes in 0 blocks
==27974==    indirectly lost: 0 bytes in 0 blocks
==27974==      possibly lost: 288 bytes in 1 blocks
==27974==    still reachable: 72,704 bytes in 1 blocks
==27974==         suppressed: 0 bytes in 0 blocks
==27974== 
==27974== For counts of detected and suppressed errors, rerun with: -v
==27974== ERROR SUMMARY: 1 errors from 1 contexts (suppressed: 0 from 0)
Killed

```


## Closure
The whole project was implemented in order to further understand the above mentioned technologies. I decided to share it with everyone in case someone else needs to see how all these might work. Comments and fixes are all welcome :)

I am not sure if I ever going to extend this from this basic form.(but also I might, who knows)

## Acknowledgments

Extremely helpful for an induction to networking is [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)

