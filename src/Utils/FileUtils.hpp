#pragma once

#include <filesystem>
#include <string>

namespace Utils {
std::string getFileContents(const std::filesystem::path& path);
}
