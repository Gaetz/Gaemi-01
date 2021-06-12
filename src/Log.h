// Inspired from http://www.drdobbs.com/cpp/logging-in-c/201804215
// and https://github.com/zuhd-org/easyloggingpp

#ifndef LOG_H
#define LOG_H

#include <fstream>
#include <sstream>

#define GAME_LOG_FILE "game.log"

enum class LogLevel {
    Error = 0,
    Warning = 1,
    Info = 2,
    Debug = 3
};

struct LogConfig {
    int reportingLevel = static_cast<int>(LogLevel::Info);
    bool restart = false;
};

extern LogConfig LOG_CONFIG;

// General purpose logging class
// Logs in standard output and in a file, configured
// with the GAME_LOG_FILE macro.
// Usage : LOG(MessageLevel) << "Message"
class Log {
public:
    Log();

    virtual ~Log();

    std::ostringstream& get(LogLevel level = LogLevel::Info);

    static void restart();

private:
    std::ostringstream os;
    static std::ofstream file;

    std::string getLabel(LogLevel type);

    Log(const Log&);

    Log& operator=(const Log&);
};


#define LOG(level)                                              \
    if (static_cast<int>(level) > LOG_CONFIG.reportingLevel)    \
        ;                                                       \
    else                                                        \
        Log().get(level)

#endif