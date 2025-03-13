#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>

class ObjImage {
public:
    using Ptr = std::shared_ptr<ObjImage>;
    using CPtr = std::shared_ptr<const ObjImage>;

    std::string name;
    GLuint textureId = 0;
    int width;
    int height;

    static Ptr create(std::string name, GLuint textureId, int width, int height) {
        return std::make_shared<ObjImage>(std::move(name), textureId, width, height);
    }

    ObjImage(std::string name, GLuint textureId, int width, int height);
    ObjImage(const ObjImage& o);
    ObjImage(ObjImage&& o);
    ~ObjImage();
    ObjImage& operator=(const ObjImage&) = delete;
    ObjImage& operator=(ObjImage&&) = delete;
};