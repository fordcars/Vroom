#pragma once

#include <vector>
#include "Systems/ResourceSys/Obj/ObjMesh.hpp"
#include "Systems/ResourceSys/Obj/ObjResource.hpp"
#include "Systems/ResourceSys/ShaderResource.hpp"

struct RenderableComp {
    ObjResource::CPtr objectResource;
    std::vector<ObjMesh::CPtr> meshes;
    ShaderResource::CPtr shader;

    void setMeshes(const std::vector<ObjMesh::Ptr>& nonConstMeshes) {
        meshes =
            decltype(meshes)(nonConstMeshes.begin(), nonConstMeshes.end());
    }
};