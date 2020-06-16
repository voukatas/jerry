/*
 * @Author: Stylianos Voukatas 
 * @Date: 2020-06-13 17:36:38 
 * @Last Modified by: Stylianos Voukatas
 * @Last Modified time: 2020-06-16 11:03:31
 */

#include <ctime>
#include <sstream>
#include <iomanip>
#include <thread>

#include "Logger.h"
#include "loglvl.h"
#include "../config.h"
#include "../util.h"

using namespace LoggerSpace;


Logger::Logger(std::string logfile)
{
    ofs.open(logfile);
 
    if (ofs.fail())
    {
      throw std::iostream::failure("Logger: Failed to open file:" + logfile);
    }

    setLogLvl(LOGLEVEL);

}

// Guaranteed to be destroyed.
// Instantiated on first use.
// Thread safe in C++11
//https://stackoverflow.com/questions/1661529/is-meyers-implementation-of-the-singleton-pattern-thread-safe
Logger& Logger::instance()
{
    static Logger instance{R"(jerry.log)"};
    return instance;
}

// member function to write to logfile
void Logger::log(Loglvl loglvl, std::string msg)
{    
    if( loglvl <= getLogLvl() )
    {        
        //std::time_t now = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
        auto now = std::chrono::system_clock::now();
        auto now_t = std::chrono::system_clock::to_time_t(now);
        auto nowMs = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

        std::stringstream timestamp;
        timestamp << std::put_time(std::localtime(&now_t), "%Y-%m-%d %X" ) << '.'<< std::setfill('0')<< std::setw(3) << nowMs.count();

        std::stringstream threadID;
	    threadID << "[Thread Id = " << std::this_thread::get_id()<<"] ";

        std::string logLvlStr = ToString(loglvl);

        //create local scope for lock and avoid race conditions
        {
            std::unique_lock<std::mutex> lock{logger_mtx};
            
            ofs << timestamp.str() << ": " <<"["<<logLvlStr<<"] "<< threadID.str()<<msg << std::endl;
        }

    }    
         
}

void Logger::setLogLvl(Loglvl logLvl)
{
    configuredLogLvl = logLvl;

}

Loglvl Logger::getLogLvl()
{
    return configuredLogLvl;
}