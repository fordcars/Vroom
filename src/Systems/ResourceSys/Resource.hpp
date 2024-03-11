#pragma once
#include <string>

class Resource {
public:
    Resource(const std::string& name) : mName(name) {}

private:
    std::string mName;
};