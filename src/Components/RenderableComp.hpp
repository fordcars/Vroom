#pragma once
#include "Systems/ResourceSys/Obj/ObjMesh.hpp"
#include "Systems/ResourceSys/ShaderResource.hpp"

struct RenderableComp {
    ObjMesh::CPtr mesh;
    ShaderResource::CPtr shader;
};