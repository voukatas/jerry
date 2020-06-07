/*
 * ThreadPool.hpp
 *
 *  Created on: May 24, 2020
 *      Author: voukatas
 */

#ifndef THREADPOOL_HPP_
#define THREADPOOL_HPP_

#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <condition_variable>
#include <iostream>

namespace ThreadPoolSpace
{
    class ThreadPool{

	    using Job = std::function<void()>;

    private:
        //keep track of threads for cleanup
        std::vector<std::thread> workers;
        std::queue<Job> jobQ;
        std::mutex jobQMutex;
        std::condition_variable jobQCv;
        bool stopExecution = false;

    public:

        explicit ThreadPool(std::size_t numOfWorkers);
        virtual ~ThreadPool();
        ThreadPool() = delete;

	    void addJob(Job job);

    private:

        void start(std::size_t numOfWorkers);
        void shutdown();

    };

}


#endif /* THREADPOOL_HPP_ */
