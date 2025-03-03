#pragma once

#include <filesystem>

class ObjResource;
class ObjLoader {
public:
    virtual ~ObjLoader() = default;
    virtual bool load(ObjResource& resource) = 0;
};
