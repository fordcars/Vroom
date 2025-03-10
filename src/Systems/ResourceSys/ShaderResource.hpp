#pragma once
#include <glad/glad.h>

#include <array>
#include <filesystem>
#include <memory>
#include <string>

#include "Constants.hpp"

class ShaderResource {
public:
    using Ptr = std::shared_ptr<ShaderResource>;
    using CPtr = std::shared_ptr<const ShaderResource>;

    static Ptr create(const std::string& name, const std::filesystem::path& vertexPath,
                      const std::filesystem::path& fragmentPath) {
        return std::make_shared<ShaderResource>(name, vertexPath, fragmentPath);
    }

    ShaderResource(const std::string& name, const std::filesystem::path& vertexPath,
                   const std::filesystem::path& fragmentPath);
    ~ShaderResource();
    ShaderResource(const ShaderResource& other) = delete; // Don't copy shaders lol
    ShaderResource(ShaderResource&& other) noexcept;
    ShaderResource& operator=(ShaderResource other) = delete; // Same
    ShaderResource& operator=(ShaderResource&& other) noexcept;
    friend void swap(ShaderResource& first, ShaderResource& second) noexcept;

    GLuint getId() const { return mId; }
    const std::string& getName() const { return mName; }
    GLint getUniform(std::size_t uniformNameIndex) const;
    GLint getUniformBlock(std::size_t uniformBlockNameIndex) const;

private:
    std::string mName; // Useful for logs
    GLuint mId = 0;
    std::array<GLint, Constants::UniformName::size()> mUniformLocations;
    std::array<GLint, Constants::UniformBlockName::size()> mUniformBlockLocations;

    static GLuint compileShader(const std::filesystem::path& shaderPath,
                                const std::string& shaderSource, GLenum type);
    static GLuint linkShaderProgram(const std::string& shaderProgramName,
                                    GLuint vertexShader, GLuint fragmentShader);
    static std::string getGLShaderDebugLog(GLuint object, PFNGLGETSHADERIVPROC glGet_iv,
                                           PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

    void registerUniforms();
    void registerUniform(const std::string& uniformName);
    void registerUniformBlock(const std::string& uniformBlockName, GLuint bindingPoint);
};
