#include "ObjResource.hpp"

ObjResource::ObjResource(std::unique_ptr<ObjLoader> loader) {
    loader->load(*this);
    boundingBox = ObjBoundingBox::create(*this);
}