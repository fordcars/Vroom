#pragma once
#include <memory>
#include <filesystem>

class ShaderResource {
public:
    using Ptr = std::shared_ptr<ShaderResource>;
    using CPtr = std::shared_ptr<const ShaderResource>;

    static Ptr create(const std::filesystem::path& vertexPath,
        const std::filesystem::path& fragmentPath) {
        return std::make_shared<ShaderResource>(vertexPath, fragmentPath);
    }

    ShaderResource(const std::filesystem::path& vertexPath,
        const std::filesystem::path& fragmentPath);
};