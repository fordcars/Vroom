#include "FileUtils.hpp"
#include <fstream>
#include <sstream>
#include "Log.hpp"

namespace Utils {
    std::string getFileContents(const std::filesystem::path& path) {
        namespace fs = std::filesystem;

        if (fs::exists(path)) {
            std::ifstream file(path);

            if (file.is_open()) {
                std::stringstream outStream;
                outStream << file.rdbuf();
                return outStream.str();
            } else {
                Log::error() << "Failed to open file: '" << path << "'.";
            }
        } else {
            Log::error() << "FIle does not exist: '" << path << "'.";
        }

        return "";
    }
}