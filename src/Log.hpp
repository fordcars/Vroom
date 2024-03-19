#pragma once

#include <sstream>
#include <string>
#include <iomanip>
#include <iostream>

#include <SDL2/SDL.h>

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
    LogBuffer(const std::string& name, LogLevel logLevel, LogLevel currentLevel,
        bool printToCerr = false, const std::string& msgSuffix = "")
        : mName(name)
        , mLevel(logLevel)
        , mCurrentLevel(currentLevel)
        , mPrintToCerr(printToCerr)
        , mMsgSuffix(msgSuffix) {}

    ~LogBuffer() {
        if(mLevel > mCurrentLevel) return;

        std::ostringstream tmp;
        tmp << std::setw(8) << std::left << ('[' + mName + "] ")
            << std::setw(0) << str();
        if(!mMsgSuffix.empty()) tmp << mMsgSuffix;
        if(mPrintToCerr) std::cerr << tmp.str() << std::endl;
        else std::cout << tmp.str() << std::endl;
    }

private:
    std::string mName;
    LogLevel mLevel;
    LogLevel mCurrentLevel;
    bool mPrintToCerr;
    std::string mMsgSuffix;
};

// Thread-safe logging class
// Singleton
class Log {
public:
    static void setLevel(LogLevel level) {
        getInstance().mLevel = level;
    }

    static LogBuffer error() { return LogBuffer("Error", LogLevel::ERROR, getInstance().mLevel, true); }
    static LogBuffer warn()  { return LogBuffer("Warn",  LogLevel::WARN,  getInstance().mLevel); }
    static LogBuffer info()  { return LogBuffer("Info",  LogLevel::INFO,  getInstance().mLevel); }
    static LogBuffer debug() { return LogBuffer("Debug", LogLevel::DEBUG, getInstance().mLevel); }

    // Call when you know an SDL function failed
    static LogBuffer sdl_error() {
        std::string sdlError = std::string(" - SDL error: ") + SDL_GetError();
        SDL_ClearError();
        return LogBuffer("Error", LogLevel::ERROR, getInstance().mLevel, true, sdlError);
    }

private:
    LogLevel mLevel = LogLevel::INFO;
    static Log& getInstance();
};
