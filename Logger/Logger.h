/*
 * @Author: Stylianos Voukatas 
 * @Date: 2020-06-13 17:25:39 
 * @Last Modified by: Stylianos Voukatas
 * @Last Modified time: 2020-06-16 11:02:13
 */

#ifndef LOGGER_H
#define LOGGER_H

#include <mutex>
#include <fstream>

#include "loglvl.h"

namespace LoggerSpace
{
  class Logger
  {
  public:
  
    explicit Logger(std::string fileName);
        
    //delete copy/move constructors and assgiments
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    Logger( Logger&&) = delete;
    Logger& operator=(Logger&&) = delete;

    //Singleton
    static Logger& instance();
    // member function to write to logfile
    void log(Loglvl loglvl, std::string msg);
    void setLogLvl(Loglvl logLvl);
    Loglvl getLogLvl();
 
  private:
    std::ofstream ofs;//shared resource
    std::mutex logger_mtx;
    Loglvl configuredLogLvl;

    //private constructor/destructor
    Logger() = default;
    ~Logger() = default;
  };
}

#endif /* LOGGER_H */