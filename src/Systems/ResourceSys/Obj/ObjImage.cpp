#include "ObjImage.hpp"

ObjImage::ObjImage(std::string name, GLuint textureId, int width, int height)
    : name(std::move(name)), textureId(textureId), width(width), height(height) {}

ObjImage::ObjImage(const ObjImage& o) {
    name = o.name;
    textureId = o.textureId;
}

ObjImage::ObjImage(ObjImage&& o) {
    name = std::move(o.name);
    textureId = o.textureId;
    o.textureId = 0;
}

ObjImage::~ObjImage() { glDeleteTextures(1, &textureId); }