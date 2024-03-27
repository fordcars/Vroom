#pragma once
#include <memory>
#include <filesystem>
#include <unordered_map>
#include <string>

#include <glad/glad.h>

class ShaderResource {
public:
    using Ptr = std::shared_ptr<ShaderResource>;
    using CPtr = std::shared_ptr<const ShaderResource>;

    static Ptr create(const std::string& name,
                      const std::filesystem::path& vertexPath,
                      const std::filesystem::path& fragmentPath) {
        return std::make_shared<ShaderResource>(name, vertexPath, fragmentPath);
    }

    ShaderResource(const std::string& name,
                   const std::filesystem::path& vertexPath,
                   const std::filesystem::path& fragmentPath);
    ~ShaderResource();
    ShaderResource(const ShaderResource& other) = delete; // Don't copy shaders lol
    ShaderResource(ShaderResource&& other) noexcept;
    ShaderResource& operator=(ShaderResource other) = delete; // Same
    ShaderResource& operator=(ShaderResource&& other) noexcept;
    friend void swap(ShaderResource& first, ShaderResource& second) noexcept;

    GLuint getId() const;
    GLint findUniform(const std::string& uniformName) const;
    GLint findUniformBlock(const std::string& uniformBlockName) const;

private:
    std::string mName; // Useful for logs
    GLuint mId = 0;
    std::unordered_map<std::string, GLuint> mUniformMap; // mUniformMap[uniformName] = uniform location
    std::unordered_map<std::string, GLuint> mUniformBlockMap; // Same, but for uniform block indices

    static GLuint compileShader(const std::filesystem::path& shaderPath,
                                const std::string& shaderSource, GLenum type);
    static GLuint linkShaderProgram(const std::string& shaderProgramName,
                                    GLuint vertexShader,
                                    GLuint fragmentShader);
    static std::string getGLShaderDebugLog(GLuint object,
                                           PFNGLGETSHADERIVPROC glGet_iv,
                                           PFNGLGETSHADERINFOLOGPROC glGet__InfoLog);

    void registerUniforms();
    void registerUniform(const std::string& uniformName);
    void registerUniformBlock(const std::string& uniformBlockName);
};
