#pragma once

#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>
#include <memory>

enum class LogLevel : int {
    ERROR = 0,
    WARN  = 1,
    INFO  = 2,
    DEBUG = 3
};

// Internal buffer for thread-safety.
// Flushes buffer in destructor.
class LogBuffer : public std::ostringstream {
public:
    LogBuffer(const std::string& name, LogLevel logLevel, LogLevel currentLevel, bool printToCerr = false)
        : mName(name)
        , mLevel(logLevel)
        , mCurrentLevel(currentLevel)
        , mPrintToCerr(printToCerr) {}
    ~LogBuffer() {
        if(mLevel > mCurrentLevel) return;

        std::ostringstream tmp;
        tmp << std::setw(8) << std::left << ('[' + mName + "] ")
            << std::setw(0) << str();
        if(mPrintToCerr) std::cerr << tmp.str() << std::endl;
        else std::cout << tmp.str() << std::endl;
    }

private:
    std::string mName;
    LogLevel mLevel;
    LogLevel mCurrentLevel;
    bool mPrintToCerr;
};

// Thread-safe logging class
// Singleton
class Log {
public:
    static void setLevel(LogLevel level) {
        getInstance().mLevel = level;
    }

    static LogBuffer err()   { return LogBuffer("Error", LogLevel::ERROR, getInstance().mLevel, true); }
    static LogBuffer warn()  { return LogBuffer("Warn",  LogLevel::WARN,  getInstance().mLevel); }
    static LogBuffer info()  { return LogBuffer("Info",  LogLevel::INFO,  getInstance().mLevel); }
    static LogBuffer debug() { return LogBuffer("Debug", LogLevel::DEBUG, getInstance().mLevel); }

private:
    LogLevel mLevel = LogLevel::INFO;

    static Log& getInstance() {
        static std::unique_ptr<Log> mInstance = std::make_unique<Log>();
        return *mInstance;
    }
};
