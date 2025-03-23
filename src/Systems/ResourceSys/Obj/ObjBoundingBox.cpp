#include "ObjBoundingBox.hpp"

#include <Eigen/Dense>
#include <limits>

#include "ObjResource.hpp"

namespace {
// Get points from vertices with mesh transformations applied
std::vector<glm::vec3> getModelspacePoints(const ObjResource& resource) {
    std::vector<glm::vec3> points;

    for(const auto& mesh : resource.objMeshes) {
        for(const auto& index : mesh->indices) {
            const auto& vertex = resource.vertices[index];
            points.push_back(
                glm::vec3(mesh->transform * glm::vec4(vertex.position, 1.0f)));
        }
    }
    return points;
}
} // namespace

ObjBoundingBox::ObjBoundingBox(const ObjResource& resource) {
    calculateModelspaceAABB(resource);
}

std::pair<glm::vec3, glm::vec3> ObjBoundingBox::getWorldspaceAABB(
    const glm::mat4& modelMatrix) const {
    glm::vec3 outMinCorner = glm::vec3(std::numeric_limits<float>::max());
    glm::vec3 outMaxCorner = glm::vec3(std::numeric_limits<float>::lowest());

    // Define the 8 corners of the OBB
    glm::vec3 corners[8] = {minCorner,
                            {maxCorner.x, minCorner.y, minCorner.z},
                            {maxCorner.x, maxCorner.y, minCorner.z},
                            {minCorner.x, maxCorner.y, minCorner.z},
                            {minCorner.x, minCorner.y, maxCorner.z},
                            {maxCorner.x, minCorner.y, maxCorner.z},
                            {maxCorner.x, maxCorner.y, maxCorner.z},
                            {minCorner.x, maxCorner.y, maxCorner.z}};

    // Transform each corner and compute AABB min/max
    for(const auto& corner : corners) {
        glm::vec3 transformedCorner = glm::vec3(modelMatrix * glm::vec4(corner, 1.0f));
        outMinCorner = glm::min(outMinCorner, transformedCorner);
        outMaxCorner = glm::max(outMaxCorner, transformedCorner);
    }

    return {outMinCorner, outMaxCorner};
}

void ObjBoundingBox::calculateModelspaceAABB(const ObjResource& resource) {
    std::vector<glm::vec3> points = getModelspacePoints(resource);
    Log::debug() << "Calculating bounding box (" << points.size() << " points).";

    if(points.empty()) {
        Log::error() << "No points to calculate bounding box.";
        return;
    }

    // Initialize minCorner and maxCorner
    minCorner = glm::vec3(std::numeric_limits<float>::max());
    maxCorner = glm::vec3(std::numeric_limits<float>::lowest());

    // Iterate through all points to find the AABB
    for(const auto& point : points) {
        minCorner = glm::min(minCorner, point);
        maxCorner = glm::max(maxCorner, point);
    }
}
