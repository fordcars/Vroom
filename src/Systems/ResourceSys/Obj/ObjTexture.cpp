#include "ObjTexture.hpp"

ObjTexture::ObjTexture(std::string name, ObjImage::Ptr image, GLuint samplerId)
    : name(std::move(name)), image(std::move(image)), samplerId(samplerId) {}

ObjTexture::ObjTexture(const ObjTexture& o) {
    name = o.name;
    image = o.image;
    samplerId = o.samplerId;
}

ObjTexture::ObjTexture(ObjTexture&& o) {
    name = std::move(o.name);
    image = std::move(o.image);
    samplerId = o.samplerId;
    o.samplerId = 0;
}

ObjTexture::~ObjTexture() { glDeleteSamplers(1, &samplerId); }