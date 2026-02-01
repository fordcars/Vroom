#include "ObjImage.hpp"

ObjImage::ObjImage(std::string name, GLuint textureId, int width, int height)
    : name(std::move(name)), textureId(textureId), width(width), height(height) {}

ObjImage::ObjImage(ObjImage &&o) noexcept
    : name(std::move(o.name)), textureId(o.textureId), width(o.width), height(o.height) {
    o.textureId = 0;
}

ObjImage::~ObjImage() { glDeleteTextures(1, &textureId); }