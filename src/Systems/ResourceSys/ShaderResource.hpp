#pragma once
#include <memory>
#include <filesystem>

class ShaderResource {
public:
    using Ptr = std::shared_ptr<ShaderResource>;
    using CPtr = std::shared_ptr<const ShaderResource>;

    static Ptr create(const std::filesystem::path& path) {
        return std::make_shared<ShaderResource>(path);
    }

    ShaderResource(const std::filesystem::path& path);
};