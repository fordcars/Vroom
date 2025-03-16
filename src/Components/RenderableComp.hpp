#pragma once

#include <vector>

#include "Systems/ResourceSys/Obj/ObjMesh.hpp"
#include "Systems/ResourceSys/Obj/ObjResource.hpp"
#include "Systems/ResourceSys/ShaderResource.hpp"

struct RenderableComp {
    enum class ShadingType { DeferredShading, ForwardShaded };

    ObjResource::CPtr objectResource;
    ShaderResource::CPtr shader;
    ShadingType shadingType = ShadingType::DeferredShading;
};