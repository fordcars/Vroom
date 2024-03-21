#pragma once

#include <string>
#include <filesystem>

namespace Utils {
    std::string getFileContents(const std::filesystem::path& path);
}
