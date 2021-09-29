#include "Log.h"
#include <ctime>
#include <iostream>
#include "Defines.h"
#include "Asserts.h"
#include "Engine.h"

using engine::Log;

Log::Log() {
    file.open(GAME_LOG_FILE, std::fstream::app);
}

Log::~Log() {
    os << std::endl;
    file << os.str();
    Engine::getState().platform->consoleWrite(os.str(), logLevel);
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
    logLevel = static_cast<u32>(level);

    char date[19];
    auto isoDate = Engine::getState().platform->getDate();
    for (int i = 0; i < 19; ++i) {
        date[i] = isoDate[i];
    }

    // Log
    os << date << " " << getLabel(level) << ": \t";
    return os;
}

std::string Log::getLabel(LogLevel type) {
    std::string label;
    switch (type) {
        case LogLevel::Trace:
            label = "TRACE";
            break;
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
        case LogLevel::Fatal:
            label = "FATAL";
            break;
    }
    return label;
}

void engine::reportAssertionFailure(const std::string& expression, const std::string& message, const char* codeFile, i32 codeLine) {
    LOG(LogLevel::Fatal) << "Assertion failure: " << expression << " , message: " << message << " , in file " << codeFile << " line " << codeLine;
}
