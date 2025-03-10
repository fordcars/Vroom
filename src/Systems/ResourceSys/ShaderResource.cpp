#include "ShaderResource.hpp"

#include <utility>

#include "Log.hpp"
#include "Utils/FileUtils.hpp"

ShaderResource::ShaderResource(const std::string& name,
                               const std::filesystem::path& vertexPath,
                               const std::filesystem::path& fragmentPath)
    : mName(name) {
    mUniformLocations.fill(-1);
    mUniformBlockLocations.fill(-1);

    // Get source
    std::string vertexSource = Utils::getFileContents(vertexPath);
    std::string fragmentSource = Utils::getFileContents(fragmentPath);

    // Compile
    GLuint vertexShader = compileShader(vertexPath, vertexSource, GL_VERTEX_SHADER);
    GLuint fragmentShader =
        compileShader(fragmentPath, fragmentSource, GL_FRAGMENT_SHADER);

    // Link
    if(vertexShader != 0 && fragmentShader != 0) {
        mId = linkShaderProgram(mName, vertexShader, fragmentShader);
    }

    registerUniforms(); // For easier access later
}

ShaderResource::~ShaderResource() { glDeleteProgram(mId); }

ShaderResource::ShaderResource(ShaderResource&& other) noexcept { swap(*this, other); }

ShaderResource& ShaderResource::operator=(ShaderResource&& other) noexcept {
    swap(*this, other);
    return *this;
}

void swap(ShaderResource& first, ShaderResource& second) noexcept {
    std::swap(first.mName, second.mName);
    std::swap(first.mId, second.mId);
    std::swap(first.mUniformLocations, second.mUniformLocations);
    std::swap(first.mUniformBlockLocations, second.mUniformBlockLocations);
}

// If uniform is not found, returns -1 (ignored uniform location by OpenGL)
GLint ShaderResource::getUniform(std::size_t uniformNameIndex) const {
    if(uniformNameIndex >= mUniformLocations.size()) {
        Log::error() << "Uniform name index " << uniformNameIndex << " out of bounds.";
        return -1;
    }
    return mUniformLocations[uniformNameIndex];
}

// If uniform block is not found, returns -1 (ignored uniform location by OpenGL)
GLint ShaderResource::getUniformBlock(std::size_t uniformBlockNameIndex) const {
    if(uniformBlockNameIndex >= mUniformBlockLocations.size()) {
        Log::error() << "Uniform block name index " << uniformBlockNameIndex
                     << " out of bounds.";
        return -1;
    }
    return mUniformBlockLocations[uniformBlockNameIndex];
}

// Static
GLuint ShaderResource::compileShader(const std::filesystem::path& shaderPath,
                                     const std::string& shaderSource, GLenum type) {
    GLuint shader = glCreateShader(type);
    GLint shaderValid = 0;

    std::size_t length = shaderSource.length();

    // Weird () syntax to avoid MSVC issue
    if(length > (std::numeric_limits<unsigned int>::max)()) {
        Log::error() << "Shader source " << shaderPath.string() << " too long! ";
        return 0;
    }

    if(length == 0) {
        Log::error() << "Shader source " << shaderPath.string() << " empty! ";
        return 0;
    }

    const char* shaderFiles[] = {shaderSource.c_str()};
    const int shaderLengths[] = {static_cast<int>(length)};

    glShaderSource(shader, 1, shaderFiles, shaderLengths);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &shaderValid);

    if(!shaderValid) {
        std::string shaderLog =
            getGLShaderDebugLog(shader, glGetShaderiv, glGetShaderInfoLog);
        glDeleteShader(shader);

        Log::error() << "Failed to compile shader " << shaderPath.string() << ": ";
        Log::error() << shaderLog;
        return 0;
    }

    return shader;
}

// Static
GLuint ShaderResource::linkShaderProgram(const std::string& shaderProgramName,
                                         GLuint vertexShader, GLuint fragmentShader) {
    GLint programValid;
    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &programValid);

    // Flag shaders for deletion
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    if(!programValid) {
        std::string shaderLog =
            getGLShaderDebugLog(program, glGetProgramiv, glGetProgramInfoLog);
        Log::error() << "Failed to link shader '" << shaderProgramName << "': ";
        Log::error() << shaderLog;
        return 0;
    }

    return program;
}

// Static
std::string ShaderResource::getGLShaderDebugLog(
    GLuint object, PFNGLGETSHADERIVPROC glGet_iv,
    PFNGLGETSHADERINFOLOGPROC glGet__InfoLog) {
    GLint logLength;
    std::string log;

    glGet_iv(object, GL_INFO_LOG_LENGTH, &logLength);
    log.resize(logLength);

    if(logLength) glGet__InfoLog(object, logLength, NULL, &log[0]);

    log.pop_back(); // Remove null terminator (\0) that OpenGL added

    // Add indentation to all lines for pretty output
    size_t pos = log.find_last_of('\n');
    while(pos != std::string::npos) {
        log.replace(pos, 1, "\n    ");
        if(pos == 0) break;
        pos = log.find_last_of('\n', pos - 1);
    }

    return log;
}

// Register all active uniforms in program
void ShaderResource::registerUniforms() {
    const GLsizei bufferSize = 256;
    GLchar uniformNameBuffer[bufferSize]; // Each uniform name will be read to this buffer

    // Uniforms
    // Also iterates through uniforms which are part of a uniform block. Those are
    // silently ignored by registerUniforms().
    GLint count;
    glGetProgramiv(mId, GL_ACTIVE_UNIFORMS, &count);
    for(int i = 0; i < count; i++) {
        GLsizei charCount = 0;
        glGetActiveUniformName(mId, i, bufferSize, &charCount, uniformNameBuffer);
        registerUniform(uniformNameBuffer);
    }

    // Uniform blocks
    GLuint bindingPointCounter = 0;
    glGetProgramiv(mId, GL_ACTIVE_UNIFORM_BLOCKS, &count);
    for(int i = 0; i < count; i++) {
        GLsizei charCount = 0;
        glGetActiveUniformBlockName(mId, i, bufferSize, &charCount, uniformNameBuffer);
        GLuint blockIndex = glGetUniformBlockIndex(mId, uniformNameBuffer);

        // Assign a binding point
        glUniformBlockBinding(mId, blockIndex, bindingPointCounter);

        Log::debug() << "Uniform block '" << uniformNameBuffer
                     << "' bound to binding point " << bindingPointCounter;

        // Register the uniform block
        registerUniformBlock(uniformNameBuffer, bindingPointCounter);
        bindingPointCounter++;
    }
}

void ShaderResource::registerUniform(const std::string& uniformName) {
    auto uniformNameIndex = Constants::UniformName::runtimeGet(uniformName);
    if(uniformNameIndex.has_value()) {
        if(mUniformLocations[uniformNameIndex.value()] != -1) {
            Log::warn() << "Uniform '" << uniformName
                        << "' already registered for shader '" << mName << "'.";
            return;
        }

        GLuint uniformLocation = glGetUniformLocation(mId, uniformName.c_str());
        if(uniformLocation != -1) {
            Log::debug() << "Registering uniform '" << uniformName << "' in shader '"
                         << mName << "'.";
            mUniformLocations[uniformNameIndex.value()] = uniformLocation;
        }

        return;
    }

    // Silently skip unknown uniform names; these are probably part of uniform block
}

void ShaderResource::registerUniformBlock(const std::string& uniformBlockName,
                                          GLuint bindingPoint) {
    auto uniformBlockNameIndex =
        Constants::UniformBlockName::runtimeGet(uniformBlockName);
    if(uniformBlockNameIndex.has_value()) {
        if(mUniformBlockLocations[uniformBlockNameIndex.value()] != -1) {
            Log::warn() << "Uniform block '" << uniformBlockName
                        << "' already registered for shader '" << mName << "'.";
            return;
        }
        Log::debug() << "Registering uniform block '" << uniformBlockName
                     << "' in shader'" << mName << "' at binding point " << bindingPoint
                     << ".";
        mUniformBlockLocations[uniformBlockNameIndex.value()] = bindingPoint;
        return;
    }

    Log::debug() << "Skipping unknown uniform block name '" << uniformBlockName << "'.";
}