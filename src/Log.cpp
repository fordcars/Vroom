#include "Log.hpp"
#include <memory>

// Static
Log& Log::getInstance() {
    static std::unique_ptr<Log> instance = std::make_unique<Log>();
    return *instance;
}
