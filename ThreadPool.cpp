/*
 * ThreadPool.hpp
 *
 *  Created on: May 24, 2020
 *      Author: voukatas
 */

#include "config.h"
#include "ThreadPool.h"
#include "Logger/Logger.h"
#include "Logger/loglvl.h"

using namespace ThreadPoolSpace;


ThreadPool::ThreadPool(std::size_t numOfWorkers){
    start(numOfWorkers);
}

ThreadPool::~ThreadPool(){
    shutdown();
}

void ThreadPool::addJob(Job job){
    //create local scope for lock and avoid race conditions
    {
        std::unique_lock<std::mutex> lock{jobQMutex};
        //emplace creates a std::function<void()> and stores it in our queue
        jobQ.emplace(job);
    }

    //jobQ has a job so notify a thread
    jobQCv.notify_one();

}

void ThreadPool::start(std::size_t numOfWorkers){

    for(std::size_t i = 0; i < numOfWorkers; i++){
        //create and hold worker threads
        workers.emplace_back([=](){

            while(true){

                Job job;

                //critical section below
                //use a lock to protect the jobQ and a cv to avoid unnecessary cpu utilization
                {
                    std::unique_lock<std::mutex> lock{jobQMutex};
                    jobQCv.wait(lock, [=] { return !jobQ.empty() || stopExecution; });

                    if( jobQ.empty() && stopExecution ){
                        break;
                    }
                    
                    job = std::move(jobQ.front());
                    jobQ.pop();
                }
                //better execute the job out of scope in order to hold the lock less
                job();
                
                LoggerSpace::Logger::instance().log(Loglvl::TRACE,"Job completed");

            }
        });
    }
}

void ThreadPool::shutdown(){

    LoggerSpace::Logger::instance().log(Loglvl::TRACE,"ThreadPool is shutting down");

    {
        std::unique_lock<std::mutex> lock(jobQMutex);
        stopExecution = true;
    }

    jobQCv.notify_all();

    for(std::thread& worker: workers){
        worker.join();
    }
}

