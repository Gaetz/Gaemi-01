// Inspired from http://www.drdobbs.com/cpp/logging-in-c/201804215
// and https://github.com/zuhd-org/easyloggingpp

#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <sstream>
#include "Defines.h"

#define GAME_LOG_FILE "game.log"

namespace engine {

enum LogLevel {
    Fatal = 0,
    Error = 1,
    Warning = 2,
    Info = 3,
    Debug = 4,
    Trace = 5
};

#ifdef GRELEASE
constexpr int LOG_REPORTING_LEVEL = LogLevel::Error;
#else
constexpr int LOG_REPORTING_LEVEL = LogLevel::Trace;
#endif


// General purpose logging class
// Logs in standard output and in a file, configured
// with the GAME_LOG_FILE macro.
// Usage : LOG(LogLevel) << "Message"
class Log {
public:
    Log();

    virtual ~Log();

    std::ostringstream& get(LogLevel level = LogLevel::Info);

    GAPI static void restart();

private:
    std::ostringstream os;
    static std::ofstream file;

    std::string getLabel(LogLevel type);

    Log(const Log&);

    Log& operator=(const Log&);
};
}

#define LOG(level)                                              \
    if (static_cast<int>(level) > LOG_REPORTING_LEVEL)          \
        ;                                                       \
    else                                                        \
        Log().get(level)
#endif