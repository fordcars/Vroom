#include "ObjOrientedBoundingBox.hpp"

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

glm::mat3 computeCovarianceMatrix(const std::vector<glm::vec3>& points,
                                  glm::vec3& centroid) {
    centroid = glm::vec3(0.0f);
    for(const auto& p : points) {
        centroid += p;
    }
    centroid /= static_cast<float>(points.size());

    glm::mat3 cov(0.0f);
    for(const auto& p : points) {
        glm::vec3 diff = p - centroid;
        cov += glm::outerProduct(diff, diff);
    }
    return cov / static_cast<float>(points.size());
}

// Compute eigenvalues and eigenvectors using Eigen library
void computeEigenDecomposition(const glm::mat3& matrix, glm::vec3& eigenvalues,
                               glm::mat3& eigenvectors) {
    Eigen::Matrix3f mat;
    for(int i = 0; i < 3; i++)
        for(int j = 0; j < 3; j++) mat(i, j) = matrix[i][j];

    Eigen::SelfAdjointEigenSolver<Eigen::Matrix3f> solver(mat);
    Eigen::Vector3f eigVals = solver.eigenvalues();
    Eigen::Matrix3f eigVecs = solver.eigenvectors();

    eigenvalues = glm::vec3(eigVals[0], eigVals[1], eigVals[2]);
    eigenvectors = glm::mat3(eigVecs(0, 0), eigVecs(1, 0), eigVecs(2, 0), eigVecs(0, 1),
                             eigVecs(1, 1), eigVecs(2, 1), eigVecs(0, 2), eigVecs(1, 2),
                             eigVecs(2, 2));
}
} // namespace

ObjOrientedBoundingBox::ObjOrientedBoundingBox(const ObjResource& resource) {
    calculateCorners(resource);
}

std::pair<glm::vec3, glm::vec3> ObjOrientedBoundingBox::getWorldspaceAABB(
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

void ObjOrientedBoundingBox::calculateCorners(const ObjResource& resource) {
    std::vector<glm::vec3> points = getModelspacePoints(resource);

    glm::vec3 centroid;
    glm::mat3 covariance = computeCovarianceMatrix(points, centroid);

    glm::vec3 eigenvalues;
    glm::mat3 eigenvectors;
    computeEigenDecomposition(covariance, eigenvalues, eigenvectors);

    // Find min/max extents along the new basis
    glm::vec3 minExtents(std::numeric_limits<float>::max());
    glm::vec3 maxExtents(std::numeric_limits<float>::lowest());

    for(const auto& p : points) {
        glm::vec3 localPos = glm::transpose(eigenvectors) * (p - centroid);
        minExtents = glm::min(minExtents, localPos);
        maxExtents = glm::max(maxExtents, localPos);
    }

    // Compute OBB center and extents
    glm::vec3 obbCenter = centroid + eigenvectors * ((minExtents + maxExtents) * 0.5f);

    // Compute the 2 key corners (min and max)
    minCorner = obbCenter + eigenvectors * minExtents;
    maxCorner = obbCenter + eigenvectors * maxExtents;
}
