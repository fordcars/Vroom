#pragma once

#include <glad/glad.h>

#include <glm/glm.hpp>
#include <memory>
#include <string>

#include "ObjImage.hpp"

class ObjTexture {
public:
    using Ptr = std::shared_ptr<ObjTexture>;
    using CPtr = std::shared_ptr<const ObjTexture>;

    std::string name;
    ObjImage::Ptr image = nullptr;
    GLuint samplerId = 0;

    static Ptr create(std::string name, ObjImage::Ptr image, GLuint samplerId) {
        return std::make_shared<ObjTexture>(std::move(name), std::move(image), samplerId);
    }

    ObjTexture(std::string name, ObjImage::Ptr image, GLuint samplerId);
    ObjTexture(const ObjTexture& o);
    ObjTexture(ObjTexture&& o);
    ~ObjTexture();
    ObjTexture& operator=(const ObjTexture&) = delete;
    ObjTexture& operator=(ObjTexture&&) = delete;
};