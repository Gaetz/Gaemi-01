#include "Log.h"
#include <ctime>
#include <iostream>
#include "Defines.h"

using engine::Log;

Log::Log() {
    file.open(GAME_LOG_FILE, std::fstream::app);
}

Log::~Log() {
    os << std::endl;
    file << os.str();
    std::cout << os.str();
    os.clear();
    file.close();
}

std::ofstream Log::file;

void Log::restart() {
    file.open(GAME_LOG_FILE, std::fstream::trunc);
    file.close();
}

std::ostringstream& Log::get(LogLevel level) {
    if (!file)
        return os;

    // Log
    time_t now;
    char date[19];
#ifdef GPLATFORM_LINUX
    struct tm* timeInfo;
    timeInfo = localtime(&now);
    strftime(date, 19, "%y-%m-%d %H:%M:%S", timeInfo);
#elif GPLATFORM_WINDOWS
    struct tm timeInfo;
    time(&now);
    localtime_s(&timeInfo, &now);
    strftime(date, 19, "%y-%m-%d %H:%M:%S", &timeInfo);
#endif
    // Log
    os << date << " " << getLabel(level) << ": \t";
    return os;
}

std::string Log::getLabel(LogLevel type) {
    std::string label;
    switch (type) {
        case LogLevel::Debug:
            label = "DEBUG";
            break;
        case LogLevel::Info:
            label = "INFO ";
            break;
        case LogLevel::Warning:
            label = "WARN ";
            break;
        case LogLevel::Error:
            label = "ERROR";
            break;
    }
    return label;
}